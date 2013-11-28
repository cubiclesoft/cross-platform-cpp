// Cross-platform, optionally named (cross-process), semaphore.
// (C) 2013 CubicleSoft.  All Rights Reserved.

#define _CRT_SECURE_NO_WARNINGS

#include <cstdio>
#include <cstring>

#include "sync_semaphore.h"

namespace CubicleSoft
{
	namespace Sync
	{
#if defined(_WIN32) || defined(WIN32) || defined(_WIN64) || defined(WIN64)
		// Windows.
		Semaphore::Semaphore() : MxWinSemaphore(NULL)
		{
		}

		Semaphore::~Semaphore()
		{
			if (MxWinSemaphore != NULL)  ::CloseHandle(MxWinSemaphore);
		}

		bool Semaphore::Create(const char *Name, int InitialVal)
		{
			if (MxWinSemaphore != NULL)  ::CloseHandle(MxWinSemaphore);

			MxWinSemaphore = NULL;

			SECURITY_ATTRIBUTES SecAttr;

			SecAttr.nLength = sizeof(SecAttr);
			SecAttr.lpSecurityDescriptor = NULL;
			SecAttr.bInheritHandle = TRUE;

			MxWinSemaphore = ::CreateSemaphoreA(&SecAttr, (LONG)InitialVal, (LONG)InitialVal, Name);

			if (MxWinSemaphore == NULL)  return false;

			return true;
		}

		bool Semaphore::Lock(std::uint32_t Wait)
		{
			if (MxWinSemaphore == NULL)  return false;

			DWORD Result = ::WaitForSingleObject(MxWinSemaphore, Wait);
			if (Result != WAIT_OBJECT_0)  return false;

			return true;
		}

		bool Semaphore::Unlock(int *PrevCount)
		{
			if (MxWinSemaphore == NULL)  return false;

			LONG Count;
			if (!::ReleaseSemaphore(MxWinSemaphore, 1, &Count))  return false;
			if (PrevCount != NULL)  *PrevCount = (int)Count;

			return true;
		}
#else
		// POSIX pthreads.
		Semaphore::Semaphore() : MxSemSemaphore(SEM_FAILED)
		{
		}

		Semaphore::~Semaphore()
		{
			if (MxSemSemaphore != SEM_FAILED)
			{
				if (MxAllocated)  delete MxSemSemaphore;
				else  sem_close(MxSemSemaphore);
			}
		}

		bool Semaphore::Create(const char *Name, int InitialVal)
		{
			if (MxSemSemaphore != SEM_FAILED)
			{
				if (MxAllocated)  delete MxSemSemaphore;
				else  sem_close(MxSemSemaphore);
			}

			MxSemSemaphore = SEM_FAILED;

			if (Name != NULL)
			{
				char *Name2 = new char[strlen(Name) + 20];

				MxAllocated = false;

				sprintf(Name2, "/Sync_Semaphore_%s_0", Name);
				MxSemSemaphore = sem_open(Name2, O_CREAT, 0666, (unsigned int)InitialVal);

				delete[] Name2;
			}
			else
			{
				MxAllocated = true;

				MxSemSemaphore = new sem_t;
				memset(MxSemSemaphore, 0, sizeof(sem_t));
				sem_init(MxSemSemaphore, 0, (unsigned int)InitialVal);
			}

			if (MxSemSemaphore == SEM_FAILED)  return false;

			return true;
		}

		bool Semaphore::Lock(std::uint32_t Wait)
		{
			if (MxSemSemaphore == SEM_FAILED)  return false;

			// Wait for the semaphore.
			return Util::WaitForSemaphore(MxSemSemaphore, Wait);
		}

		bool Semaphore::Unlock(int *PrevCount)
		{
			if (MxSemSemaphore == SEM_FAILED)  return false;

			// Get the current value first.
			int Val;
			sem_getvalue(MxSemSemaphore, &Val);
			if (PrevCount != NULL)  *PrevCount = Val;

			// Release the semaphore.
			sem_post(MxSemSemaphore);

			return true;
		}
#endif


		Semaphore::AutoUnlock::AutoUnlock(Semaphore *LockPtr)
		{
			MxLock = LockPtr;
		}

		Semaphore::AutoUnlock::~AutoUnlock()
		{
			MxLock->Unlock();
		}
	}
}
