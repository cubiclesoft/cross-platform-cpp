// Cross-platform, optionally named (cross-process), reader/writer lock.
// (C) 2014 CubicleSoft.  All Rights Reserved.

#define _CRT_SECURE_NO_WARNINGS

#include <cstdio>
#include <cstring>

#include "sync_readwritelock.h"

namespace CubicleSoft
{
	namespace Sync
	{
#if defined(_WIN32) || defined(WIN32) || defined(_WIN64) || defined(WIN64)
		// Windows.
		ReadWriteLock::ReadWriteLock() : MxWinRSemMutex(NULL), MxWinRSemaphore(NULL), MxWinRWaitEvent(NULL), MxWinWWaitMutex(NULL)
		{
		}

		ReadWriteLock::~ReadWriteLock()
		{
			if (MxWinWWaitMutex != NULL)  ::CloseHandle(MxWinWWaitMutex);
			if (MxWinRWaitEvent != NULL)  ::CloseHandle(MxWinRWaitEvent);
			if (MxWinRSemaphore != NULL)  ::CloseHandle(MxWinRSemaphore);
			if (MxWinRSemMutex != NULL)  ::CloseHandle(MxWinRSemMutex);
		}

		bool ReadWriteLock::Create(const char *Name)
		{
			if (MxWinWWaitMutex != NULL)  ::CloseHandle(MxWinWWaitMutex);
			if (MxWinRWaitEvent != NULL)  ::CloseHandle(MxWinRWaitEvent);
			if (MxWinRSemaphore != NULL)  ::CloseHandle(MxWinRSemaphore);
			if (MxWinRSemMutex != NULL)  ::CloseHandle(MxWinRSemMutex);

			MxWinWWaitMutex = NULL;
			MxWinRWaitEvent = NULL;
			MxWinRSemaphore = NULL;
			MxWinRSemMutex = NULL;

			char *Name2;
			SECURITY_ATTRIBUTES SecAttr;

			SecAttr.nLength = sizeof(SecAttr);
			SecAttr.lpSecurityDescriptor = NULL;
			SecAttr.bInheritHandle = TRUE;

			if (Name == NULL)  Name2 = NULL;
			else  Name2 = new char[strlen(Name) + 20];

			// Create the mutexes, semaphore, and event objects.
			if (Name2 != NULL)  sprintf(Name2, "Sync_ReadWrite|%s|0", Name);
			MxWinRSemMutex = ::CreateSemaphoreA(&SecAttr, 1, 1, Name2);
			if (Name2 != NULL)  sprintf(Name2, "Sync_ReadWrite|%s|1", Name);
			MxWinRSemaphore = ::CreateSemaphoreA(&SecAttr, LONG_MAX, LONG_MAX, Name2);
			if (Name2 != NULL)  sprintf(Name2, "Sync_ReadWrite|%s|2", Name);
			MxWinRWaitEvent = ::CreateEventA(&SecAttr, TRUE, TRUE, Name2);
			if (Name2 != NULL)  sprintf(Name2, "Sync_ReadWrite|%s|3", Name);
			MxWinWWaitMutex = ::CreateSemaphoreA(&SecAttr, 1, 1, Name2);

			if (Name2 != NULL)  delete[] Name2;

			if (MxWinRSemMutex == NULL || MxWinRSemaphore == NULL || MxWinRWaitEvent == NULL || MxWinWWaitMutex == NULL)  return false;

			return true;
		}

		bool ReadWriteLock::ReadLock(uint32_t Wait)
		{
			uint64_t StartTime, CurrTime;

			if (MxWinRSemaphore == NULL || MxWinRSemMutex == NULL || MxWinRWaitEvent == NULL || MxWinWWaitMutex == NULL)  return false;

			// Get current time in milliseconds.
			StartTime = (Wait == INFINITE ? 0 : Util::GetUnixMicrosecondTime() / 1000000);

			// Acquire the write lock mutex.  Guarantees that readers can't starve the writer.
			DWORD Result = ::WaitForSingleObject(MxWinWWaitMutex, Wait);
			if (Result != WAIT_OBJECT_0)  return false;

			// Acquire the semaphore mutex.
			CurrTime = (Wait == INFINITE ? 0 : Util::GetUnixMicrosecondTime() / 1000000);
			if (Wait < CurrTime - StartTime)
			{
				::ReleaseSemaphore(MxWinWWaitMutex, 1, NULL);

				return false;
			}
			Result = ::WaitForSingleObject(MxWinRSemMutex, Wait - (DWORD)(CurrTime - StartTime));
			if (Result != WAIT_OBJECT_0)
			{
				::ReleaseSemaphore(MxWinWWaitMutex, 1, NULL);

				return false;
			}

			// Acquire the semaphore.
			CurrTime = (Wait == INFINITE ? 0 : Util::GetUnixMicrosecondTime() / 1000000);
			if (Wait < CurrTime - StartTime)
			{
				::ReleaseSemaphore(MxWinRSemMutex, 1, NULL);
				::ReleaseSemaphore(MxWinWWaitMutex, 1, NULL);

				return false;
			}
			Result = ::WaitForSingleObject(MxWinRSemaphore, Wait - (DWORD)(CurrTime - StartTime));
			if (Result != WAIT_OBJECT_0)
			{
				::ReleaseSemaphore(MxWinRSemMutex, 1, NULL);
				::ReleaseSemaphore(MxWinWWaitMutex, 1, NULL);

				return false;
			}

			// Update the event state.
			if (!::ResetEvent(MxWinRWaitEvent))
			{
				::ReleaseSemaphore(MxWinRSemaphore, 1, NULL);
				::ReleaseSemaphore(MxWinRSemMutex, 1, NULL);
				::ReleaseSemaphore(MxWinWWaitMutex, 1, NULL);

				return false;
			}

			// Release the mutexes.
			::ReleaseSemaphore(MxWinRSemMutex, 1, NULL);
			::ReleaseSemaphore(MxWinWWaitMutex, 1, NULL);

			return true;
		}

		bool ReadWriteLock::WriteLock(uint32_t Wait)
		{
			uint64_t StartTime, CurrTime;

			if (MxWinRWaitEvent == NULL || MxWinWWaitMutex == NULL)  return false;

			// Get current time in milliseconds.
			StartTime = (Wait == INFINITE ? 0 : Util::GetUnixMicrosecondTime() / 1000000);

			// Acquire the write lock mutex.
			DWORD Result = ::WaitForSingleObject(MxWinWWaitMutex, Wait);
			if (Result != WAIT_OBJECT_0)  return false;

			// Wait for readers to reach zero.
			CurrTime = (Wait == INFINITE ? 0 : Util::GetUnixMicrosecondTime() / 1000000);
			Result = ::WaitForSingleObject(MxWinRWaitEvent, Wait - (DWORD)(CurrTime - StartTime));
			if (Result != WAIT_OBJECT_0)
			{
				::ReleaseSemaphore(MxWinWWaitMutex, 1, NULL);

				return false;
			}

			return true;
		}

		bool ReadWriteLock::ReadUnlock()
		{
			if (MxWinRSemMutex == NULL || MxWinRSemaphore == NULL || MxWinRWaitEvent == NULL)  return false;

			// Acquire the semaphore mutex.
			DWORD Result = ::WaitForSingleObject(MxWinRSemMutex, INFINITE);
			if (Result != WAIT_OBJECT_0)  return false;

			// Release the semaphore.
			LONG Val;
			if (!::ReleaseSemaphore(MxWinRSemaphore, 1, &Val))
			{
				::ReleaseSemaphore(MxWinRSemMutex, 1, NULL);

				return false;
			}

			// Update the event state.
			if (Val == LONG_MAX - 1)
			{
				if (!::SetEvent(MxWinRWaitEvent))
				{
					::ReleaseSemaphore(MxWinRSemMutex, 1, NULL);

					return false;
				}
			}

			// Release the semaphore mutex.
			::ReleaseSemaphore(MxWinRSemMutex, 1, NULL);

			return true;
		}

		bool ReadWriteLock::WriteUnlock()
		{
			if (MxWinWWaitMutex == NULL)  return false;

			// Release the write lock.
			::ReleaseSemaphore(MxWinWWaitMutex, 1, NULL);

			return true;
		}
#else
		// POSIX pthreads.
		ReadWriteLock::ReadWriteLock() : MxSemRSemMutex(SEM_FAILED), MxSemRSemaphore(SEM_FAILED), MxSemRWaitEvent(SEM_FAILED), MxSemWWaitMutex(SEM_FAILED), MxAllocated(false)
		{
		}

		ReadWriteLock::~ReadWriteLock()
		{
			if (MxAllocated)
			{
				if (MxSemWWaitMutex != SEM_FAILED)  delete MxSemWWaitMutex;
				if (MxSemRWaitEvent != SEM_FAILED)  delete MxSemRWaitEvent;
				if (MxSemRSemaphore != SEM_FAILED)  delete MxSemRSemaphore;
				if (MxSemRSemMutex != SEM_FAILED)  delete MxSemRSemMutex;
			}
			else
			{
				if (MxSemWWaitMutex != SEM_FAILED)  sem_close(MxSemWWaitMutex);
				if (MxSemRWaitEvent != SEM_FAILED)  sem_close(MxSemRWaitEvent);
				if (MxSemRSemaphore != SEM_FAILED)  sem_close(MxSemRSemaphore);
				if (MxSemRSemMutex != SEM_FAILED)  sem_close(MxSemRSemMutex);
			}
		}

		bool ReadWriteLock::Create(const char *Name)
		{
			if (MxAllocated)
			{
				if (MxSemWWaitMutex != SEM_FAILED)  delete MxSemWWaitMutex;
				if (MxSemRWaitEvent != SEM_FAILED)  delete MxSemRWaitEvent;
				if (MxSemRSemaphore != SEM_FAILED)  delete MxSemRSemaphore;
				if (MxSemRSemMutex != SEM_FAILED)  delete MxSemRSemMutex;
			}
			else
			{
				if (MxSemWWaitMutex != SEM_FAILED)  sem_close(MxSemWWaitMutex);
				if (MxSemRWaitEvent != SEM_FAILED)  sem_close(MxSemRWaitEvent);
				if (MxSemRSemaphore != SEM_FAILED)  sem_close(MxSemRSemaphore);
				if (MxSemRSemMutex != SEM_FAILED)  sem_close(MxSemRSemMutex);
			}

			MxSemWWaitMutex = SEM_FAILED;
			MxSemRWaitEvent = SEM_FAILED;
			MxSemRSemaphore = SEM_FAILED;
			MxSemRSemMutex = SEM_FAILED;

			if (Name != NULL)
			{
				char *Name2 = new char[strlen(Name) + 20];

				MxAllocated = false;

				sprintf(Name2, "/Sync_ReadWrite_%s_0", Name);
				MxSemRSemMutex = sem_open(Name2, O_CREAT, 0666, 1);
				sprintf(Name2, "/Sync_ReadWrite_%s_1", Name);
				MxSemRSemaphore = sem_open(Name2, O_CREAT, 0666, SEM_VALUE_MAX);
				sprintf(Name2, "/Sync_ReadWrite_%s_2", Name);
				MxSemRWaitEvent = sem_open(Name2, O_CREAT, 0666, 1);
				sprintf(Name2, "/Sync_ReadWrite_%s_3", Name);
				MxSemWWaitMutex = sem_open(Name2, O_CREAT, 0666, 1);

				delete[] Name2;
			}
			else
			{
				MxAllocated = true;

				MxSemRSemMutex = new sem_t;
				memset(MxSemRSemMutex, 0, sizeof(sem_t));
				sem_init(MxSemRSemMutex, 0, 1);

				MxSemRSemaphore = new sem_t;
				memset(MxSemRSemaphore, 0, sizeof(sem_t));
				sem_init(MxSemRSemaphore, 0, SEM_VALUE_MAX);

				MxSemRWaitEvent = new sem_t;
				memset(MxSemRWaitEvent, 0, sizeof(sem_t));
				sem_init(MxSemRWaitEvent, 0, 1);

				MxSemWWaitMutex = new sem_t;
				memset(MxSemWWaitMutex, 0, sizeof(sem_t));
				sem_init(MxSemWWaitMutex, 0, 1);
			}

			if (MxSemRSemMutex == SEM_FAILED || MxSemRSemaphore == SEM_FAILED || MxSemRWaitEvent == SEM_FAILED || MxSemWWaitMutex == SEM_FAILED)  return false;

			return true;
		}

		bool ReadWriteLock::ReadLock(uint32_t Wait)
		{
			uint64_t StartTime, CurrTime;

			if (MxSemRSemMutex == SEM_FAILED || MxSemRSemaphore == SEM_FAILED || MxSemRWaitEvent == SEM_FAILED || MxSemWWaitMutex == SEM_FAILED)  return false;

			// Get current time in milliseconds.
			StartTime = (Wait == INFINITE ? 0 : Util::GetUnixMicrosecondTime() / 1000000);

			// Acquire the write lock mutex.  Guarantees that readers can't starve the writer.
			if (!Util::WaitForSemaphore(MxSemWWaitMutex, Wait))  return false;

			// Acquire the semaphore mutex.
			CurrTime = (Wait == INFINITE ? 0 : Util::GetUnixMicrosecondTime() / 1000000);
			if (Wait < CurrTime - StartTime)
			{
				sem_post(MxSemWWaitMutex);

				return false;
			}
			if (!Util::WaitForSemaphore(MxSemRSemMutex, Wait - (CurrTime - StartTime)))
			{
				sem_post(MxSemWWaitMutex);

				return false;
			}

			// Acquire the semaphore.
			CurrTime = (Wait == INFINITE ? 0 : Util::GetUnixMicrosecondTime() / 1000000);
			if (Wait < CurrTime - StartTime)
			{
				sem_post(MxSemRSemMutex);
				sem_post(MxSemWWaitMutex);

				return false;
			}
			if (!Util::WaitForSemaphore(MxSemRSemaphore, Wait - (CurrTime - StartTime)))
			{
				sem_post(MxSemRSemMutex);
				sem_post(MxSemWWaitMutex);

				return false;
			}

			// Update the event state.
			int Val;
			sem_getvalue(MxSemRSemaphore, &Val);
			if (Val == SEM_VALUE_MAX - 1)
			{
				if (!Util::WaitForSemaphore(MxSemRWaitEvent, INFINITE))
				{
					sem_post(MxSemRSemaphore);
					sem_post(MxSemRSemMutex);
					sem_post(MxSemWWaitMutex);

					return false;
				}
			}

			// Release the mutexes.
			sem_post(MxSemRSemMutex);
			sem_post(MxSemWWaitMutex);

			return true;
		}

		bool ReadWriteLock::WriteLock(uint32_t Wait)
		{
			uint64_t StartTime, CurrTime;

			if (MxSemRWaitEvent == NULL || MxSemWWaitMutex == NULL)  return false;

			// Get current time in milliseconds.
			StartTime = (Wait == INFINITE ? 0 : Util::GetUnixMicrosecondTime() / 1000000);

			// Acquire the write lock mutex.
			if (!Util::WaitForSemaphore(MxSemWWaitMutex, Wait))  return false;

			// Wait for readers to reach zero.
			CurrTime = (Wait == INFINITE ? 0 : Util::GetUnixMicrosecondTime() / 1000000);
			if (!Util::WaitForSemaphore(MxSemRWaitEvent, Wait - (CurrTime - StartTime)))
			{
				sem_post(MxSemWWaitMutex);

				return false;
			}

			// Release the semaphore to avoid a later deadlock.
			sem_post(MxSemRWaitEvent);

			return true;
		}

		bool ReadWriteLock::ReadUnlock()
		{
			if (MxSemRSemMutex == NULL || MxSemRSemaphore == NULL || MxSemRWaitEvent == NULL)  return false;

			// Acquire the semaphore mutex.
			if (!Util::WaitForSemaphore(MxSemRSemMutex, INFINITE))  return false;

			// Release the semaphore.
			int Result = sem_post(MxSemRSemaphore);
			if (Result != 0)
			{
				sem_post(MxSemRSemMutex);

				return false;
			}

			// Update the event state.
			int Val;
			sem_getvalue(MxSemRSemaphore, &Val);
			if (Val == SEM_VALUE_MAX)
			{
				if (sem_post(MxSemRWaitEvent) != 0)
				{
					sem_post(MxSemRSemMutex);

					return false;
				}
			}

			// Release the semaphore mutex.
			sem_post(MxSemRSemMutex);

			return (Result == 0);
		}

		bool ReadWriteLock::WriteUnlock()
		{
			if (MxSemWWaitMutex == NULL)  return false;

			// Release the write lock.
			sem_post(MxSemWWaitMutex);

			return true;
		}
#endif


		ReadWriteLock::AutoReadUnlock::AutoReadUnlock(ReadWriteLock *LockPtr)
		{
			MxLock = LockPtr;
		}

		ReadWriteLock::AutoReadUnlock::~AutoReadUnlock()
		{
			MxLock->ReadUnlock();
		}


		ReadWriteLock::AutoWriteUnlock::AutoWriteUnlock(ReadWriteLock *LockPtr)
		{
			MxLock = LockPtr;
		}

		ReadWriteLock::AutoWriteUnlock::~AutoWriteUnlock()
		{
			MxLock->WriteUnlock();
		}
	}
}
