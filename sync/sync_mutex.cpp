// Cross-platform, optionally named (cross-process), self-counting (recursive) mutex.
// (C) 2013 CubicleSoft.  All Rights Reserved.

#define _CRT_SECURE_NO_WARNINGS

#include <cstdio>
#include <cstring>

#include "sync_mutex.h"

namespace CubicleSoft
{
	namespace Sync
	{
#if defined(_WIN32) || defined(WIN32) || defined(_WIN64) || defined(WIN64)
		// Windows.
		Mutex::Mutex() : MxWinMutex(NULL), MxOwnerID(0), MxCount(0)
		{
			::InitializeCriticalSection(&MxWinCritSection);
		}

		Mutex::~Mutex()
		{
			Unlock(true);

			if (MxWinMutex != NULL)  ::CloseHandle(MxWinMutex);
			::DeleteCriticalSection(&MxWinCritSection);
		}

		bool Mutex::Create(const char *Name)
		{
			::EnterCriticalSection(&MxWinCritSection);

			if (MxWinMutex != NULL)
			{
				if (MxOwnerID > 0)
				{
					if (MxOwnerID == Util::GetCurrentThreadID())
					{
						::ReleaseMutex(MxWinMutex);
						::CloseHandle(MxWinMutex);
						MxWinMutex = NULL;
						MxCount = 0;
						MxOwnerID = 0;
					}
					else
					{
						::LeaveCriticalSection(&MxWinCritSection);

						return false;
					}
				}
			}

			SECURITY_ATTRIBUTES SecAttr;

			SecAttr.nLength = sizeof(SecAttr);
			SecAttr.lpSecurityDescriptor = NULL;
			SecAttr.bInheritHandle = TRUE;

			MxWinMutex = ::CreateMutexA(&SecAttr, FALSE, Name);
			if (MxWinMutex == NULL)
			{
				::LeaveCriticalSection(&MxWinCritSection);

				return false;
			}

			::LeaveCriticalSection(&MxWinCritSection);

			return true;
		}

		bool Mutex::Lock(uint32_t Wait)
		{
			::EnterCriticalSection(&MxWinCritSection);

			// Make sure the mutex exists.
			if (MxWinMutex == NULL)
			{
				::LeaveCriticalSection(&MxWinCritSection);

				return false;
			}

			// Check to see if this is already owned by the calling thread.
			if (MxOwnerID == Util::GetCurrentThreadID())
			{
				MxCount++;
				::LeaveCriticalSection(&MxWinCritSection);

				return true;
			}

			::LeaveCriticalSection(&MxWinCritSection);

			// Acquire the mutex.
			DWORD Result = ::WaitForSingleObject(MxWinMutex, (DWORD)Wait);
			if (Result != WAIT_OBJECT_0)  return false;

			::EnterCriticalSection(&MxWinCritSection);
			MxOwnerID = Util::GetCurrentThreadID();
			MxCount = 1;
			::LeaveCriticalSection(&MxWinCritSection);

			return true;
		}

		bool Mutex::Unlock(bool All)
		{
			::EnterCriticalSection(&MxWinCritSection);

			// Make sure the mutex exists and make sure it is owned by the calling thread.
			if (MxWinMutex == NULL || MxOwnerID != Util::GetCurrentThreadID())
			{
				::LeaveCriticalSection(&MxWinCritSection);

				return false;
			}

			if (All)  MxCount = 1;
			MxCount--;
			if (!MxCount)
			{
				MxOwnerID = 0;

				// Release the mutex.
				::ReleaseMutex(MxWinMutex);
			}

			::LeaveCriticalSection(&MxWinCritSection);

			return true;
		}
#else
		// POSIX pthreads.
		Mutex::Mutex() : MxSemMutex(SEM_FAILED), MxAllocated(false), MxOwnerID(0), MxCount(0)
		{
			pthread_mutex_init(&MxPthreadCritSection, NULL);
		}

		Mutex::~Mutex()
		{
			Unlock(true);

			if (MxSemMutex != SEM_FAILED)
			{
				if (MxAllocated)  delete MxSemMutex;
				else  sem_close(MxSemMutex);
			}

			pthread_mutex_destroy(&MxPthreadCritSection);
		}

		bool Mutex::Create(const char *Name)
		{
			if (pthread_mutex_lock(&MxPthreadCritSection) != 0)  return false;

			if (MxOwnerID > 0)
			{
				if (MxOwnerID == Util::GetCurrentThreadID())
				{
					sem_post(MxSemMutex);
					if (MxAllocated)  delete MxSemMutex;
					else  sem_close(MxSemMutex);

					MxSemMutex = SEM_FAILED;
					MxAllocated = false;
					MxCount = 0;
					MxOwnerID = 0;
				}
				else
				{
					pthread_mutex_unlock(&MxPthreadCritSection);

					return false;
				}
			}

			if (Name != NULL)
			{
				char *Name2 = new char[strlen(Name) + 20];

				MxAllocated = false;

				sprintf(Name2, "/Sync_Mutex_%s_0", Name);
				MxSemMutex = sem_open(Name2, O_CREAT, 0666, 1);

				delete[] Name2;
			}
			else
			{
				MxAllocated = true;

				MxSemMutex = new sem_t;
				memset(MxSemMutex, 0, sizeof(sem_t));
				sem_init(MxSemMutex, 0, 1);
			}

			pthread_mutex_unlock(&MxPthreadCritSection);

			if (MxSemMutex == SEM_FAILED)  return false;

			return true;
		}

		bool Mutex::Lock(uint32_t Wait)
		{
			if (pthread_mutex_lock(&MxPthreadCritSection) != 0)  return false;

			// Check to see if this mutex is already owned by the calling thread.
			if (MxOwnerID == Util::GetCurrentThreadID())
			{
				MxCount++;
				pthread_mutex_unlock(&MxPthreadCritSection);

				return true;
			}

			pthread_mutex_unlock(&MxPthreadCritSection);

			if (!Util::WaitForSemaphore(MxSemMutex, Wait))  return false;

			pthread_mutex_lock(&MxPthreadCritSection);
			MxOwnerID = Util::GetCurrentThreadID();
			MxCount = 1;
			pthread_mutex_unlock(&MxPthreadCritSection);

			return true;
		}

		bool Mutex::Unlock(bool All)
		{
			if (pthread_mutex_lock(&MxPthreadCritSection) != 0)  return false;

			// Make sure the mutex is owned by the calling thread.
			if (MxOwnerID != Util::GetCurrentThreadID())
			{
				pthread_mutex_unlock(&MxPthreadCritSection);

				return false;
			}

			if (All)  MxCount = 1;
			MxCount--;
			if (!MxCount)
			{
				MxOwnerID = 0;

				// Release the mutex.
				sem_post(MxSemMutex);
			}

			pthread_mutex_unlock(&MxPthreadCritSection);

			return true;
		}
#endif


		Mutex::AutoUnlock::AutoUnlock(Mutex *LockPtr)
		{
			MxLock = LockPtr;
		}

		Mutex::AutoUnlock::~AutoUnlock()
		{
			MxLock->Unlock();
		}
	}
}
