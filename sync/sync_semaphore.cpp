// Cross-platform, optionally named (cross-process), semaphore.
// (C) 2016 CubicleSoft.  All Rights Reserved.

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
		Semaphore::Semaphore() : MxNamed(false), MxMem(NULL)
		{
		}

		Semaphore::~Semaphore()
		{
			if (MxMem != NULL)
			{
				if (MxNamed)  Util::UnmapUnixNamedMem(MxMem, Util::GetUnixSemaphoreSize());
				else
				{
					Util::FreeUnixSemaphore(MxPthreadSemaphore);

					delete[] MxMem;
				}
			}
		}

		bool Semaphore::Create(const char *Name, int InitialVal)
		{
			if (MxMem != NULL)
			{
				if (MxNamed)  Util::UnmapUnixNamedMem(MxMem, Util::GetUnixSemaphoreSize());
				else
				{
					Util::FreeUnixSemaphore(MxPthreadSemaphore);

					delete[] MxMem;
				}
			}

			MxMem = NULL;

			size_t Pos, TempSize = Util::GetUnixSemaphoreSize();
			MxNamed = (Name != NULL);
			int Result = Util::InitUnixNamedMem(MxMem, Pos, "/Sync_Semaphore", Name, TempSize);

			if (Result < 0)  return false;

			Util::GetUnixSemaphore(MxPthreadSemaphore, MxMem + Pos);

			// Handle the first time this semaphore has been opened.
			if (Result == 0)
			{
				Util::InitUnixSemaphore(MxPthreadSemaphore, MxNamed, InitialVal, InitialVal);

				if (MxNamed)  Util::UnixNamedMemReady(MxMem);
			}

			return true;
		}

		bool Semaphore::Lock(std::uint32_t Wait)
		{
			if (MxMem == NULL)  return false;

			// Wait for the semaphore.
			return Util::WaitForUnixSemaphore(MxPthreadSemaphore, Wait);
		}

		bool Semaphore::Unlock(int *PrevCount)
		{
			if (MxMem == NULL)  return false;

			std::uint32_t Count;
			Util::ReleaseUnixSemaphore(MxPthreadSemaphore, &Count);
			if (PrevCount != NULL)  *PrevCount = (int)Count;

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
