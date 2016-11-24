// Cross-platform, optionally named (cross-process), reader/writer lock.
// (C) 2016 CubicleSoft.  All Rights Reserved.

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
			if (Name2 != NULL)  sprintf(Name2, "%s-Sync_ReadWrite-0", Name);
			MxWinRSemMutex = ::CreateSemaphoreA(&SecAttr, 1, 1, Name2);
			if (Name2 != NULL)  sprintf(Name2, "%s-Sync_ReadWrite-1", Name);
			MxWinRSemaphore = ::CreateSemaphoreA(&SecAttr, LONG_MAX, LONG_MAX, Name2);
			if (Name2 != NULL)  sprintf(Name2, "%s-Sync_ReadWrite-2", Name);
			MxWinRWaitEvent = ::CreateEventA(&SecAttr, TRUE, TRUE, Name2);
			if (Name2 != NULL)  sprintf(Name2, "%s-Sync_ReadWrite-3", Name);
			MxWinWWaitMutex = ::CreateSemaphoreA(&SecAttr, 1, 1, Name2);

			if (Name2 != NULL)  delete[] Name2;

			if (MxWinRSemMutex == NULL || MxWinRSemaphore == NULL || MxWinRWaitEvent == NULL || MxWinWWaitMutex == NULL)  return false;

			return true;
		}

		bool ReadWriteLock::ReadLock(std::uint32_t Wait)
		{
			std::uint64_t StartTime, CurrTime;

			if (MxWinRSemaphore == NULL || MxWinRSemMutex == NULL || MxWinRWaitEvent == NULL || MxWinWWaitMutex == NULL)  return false;

			// Get current time in milliseconds.
			StartTime = (Wait == INFINITE ? 0 : Util::GetUnixMicrosecondTime() / 1000000);

			// Acquire the write lock mutex.  Guarantees that readers can't starve the writer.
			DWORD Result = ::WaitForSingleObject(MxWinWWaitMutex, Wait);
			if (Result != WAIT_OBJECT_0)  return false;

			// Acquire the semaphore mutex.
			CurrTime = (Wait == INFINITE ? 0 : Util::GetUnixMicrosecondTime() / 1000000);
			if (Wait < CurrTime - StartTime || ::WaitForSingleObject(MxWinRSemMutex, Wait - (DWORD)(CurrTime - StartTime)) != WAIT_OBJECT_0)
			{
				::ReleaseSemaphore(MxWinWWaitMutex, 1, NULL);

				return false;
			}

			// Acquire the semaphore.
			CurrTime = (Wait == INFINITE ? 0 : Util::GetUnixMicrosecondTime() / 1000000);
			if (Wait < CurrTime - StartTime || ::WaitForSingleObject(MxWinRSemaphore, Wait - (DWORD)(CurrTime - StartTime)) != WAIT_OBJECT_0)
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

		bool ReadWriteLock::WriteLock(std::uint32_t Wait)
		{
			std::uint64_t StartTime, CurrTime;

			if (MxWinRWaitEvent == NULL || MxWinWWaitMutex == NULL)  return false;

			// Get current time in milliseconds.
			StartTime = (Wait == INFINITE ? 0 : Util::GetUnixMicrosecondTime() / 1000000);

			// Acquire the write lock mutex.
			DWORD Result = ::WaitForSingleObject(MxWinWWaitMutex, Wait);
			if (Result != WAIT_OBJECT_0)  return false;

			// Wait for readers to reach zero.
			CurrTime = (Wait == INFINITE ? 0 : Util::GetUnixMicrosecondTime() / 1000000);
			if (Wait < CurrTime - StartTime || ::WaitForSingleObject(MxWinRWaitEvent, Wait - (DWORD)(CurrTime - StartTime)) != WAIT_OBJECT_0)
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
		ReadWriteLock::ReadWriteLock() : MxNamed(false), MxMem(NULL), MxRCount(NULL)
		{
		}

		ReadWriteLock::~ReadWriteLock()
		{
			if (MxMem != NULL)
			{
				if (MxNamed)  Util::UnmapUnixNamedMem(MxMem, Util::GetUnixSemaphoreSize() + Util::AlignUnixSize(sizeof(std::uint32_t)) + Util::GetUnixEventSize() + Util::GetUnixSemaphoreSize());
				else
				{
					Util::FreeUnixSemaphore(MxPthreadRCountMutex);
					Util::FreeUnixEvent(MxPthreadRWaitEvent);
					Util::FreeUnixSemaphore(MxPthreadWWaitMutex);

					delete[] MxMem;
				}
			}
		}

		bool ReadWriteLock::Create(const char *Name)
		{
			if (MxMem != NULL)
			{
				if (MxNamed)  Util::UnmapUnixNamedMem(MxMem, Util::GetUnixSemaphoreSize() + Util::AlignUnixSize(sizeof(std::uint32_t)) + Util::GetUnixEventSize() + Util::GetUnixSemaphoreSize());
				else
				{
					Util::FreeUnixSemaphore(MxPthreadRCountMutex);
					Util::FreeUnixEvent(MxPthreadRWaitEvent);
					Util::FreeUnixSemaphore(MxPthreadWWaitMutex);

					delete[] MxMem;
				}
			}

			MxMem = NULL;
			MxRCount = NULL;

			size_t Pos, TempSize = Util::GetUnixSemaphoreSize() + Util::AlignUnixSize(sizeof(std::uint32_t)) + Util::GetUnixEventSize() + Util::GetUnixSemaphoreSize();
			MxNamed = (Name != NULL);
			int Result = Util::InitUnixNamedMem(MxMem, Pos, "/Sync_ReadWrite", Name, TempSize);

			if (Result < 0)  return false;

			// Load the pointers.
			char *MemPtr = MxMem + Pos;
			Util::GetUnixSemaphore(MxPthreadRCountMutex, MemPtr);
			MemPtr += Util::GetUnixSemaphoreSize();

			MxRCount = reinterpret_cast<volatile std::uint32_t *>(MemPtr);
			MemPtr += Util::AlignUnixSize(sizeof(std::uint32_t));

			Util::GetUnixEvent(MxPthreadRWaitEvent, MemPtr);
			MemPtr += Util::GetUnixEventSize();

			Util::GetUnixSemaphore(MxPthreadWWaitMutex, MemPtr);

			// Handle the first time this reader/writer lock has been opened.
			if (Result == 0)
			{
				Util::InitUnixSemaphore(MxPthreadRCountMutex, MxNamed, 1, 1);
				MxRCount[0] = 0;
				Util::InitUnixEvent(MxPthreadRWaitEvent, MxNamed, true, true);
				Util::InitUnixSemaphore(MxPthreadWWaitMutex, MxNamed, 1, 1);

				if (MxNamed)  Util::UnixNamedMemReady(MxMem);
			}

			return true;
		}

		bool ReadWriteLock::ReadLock(std::uint32_t Wait)
		{
			std::uint64_t StartTime, CurrTime;

			if (MxMem == NULL)  return false;

			// Get current time in milliseconds.
			StartTime = (Wait == INFINITE ? 0 : Util::GetUnixMicrosecondTime() / 1000000);

			// Acquire the write lock mutex.  Guarantees that readers can't starve the writer.
			if (!Util::WaitForUnixSemaphore(MxPthreadWWaitMutex, Wait))  return false;

			// Acquire the counter mutex.
			CurrTime = (Wait == INFINITE ? 0 : Util::GetUnixMicrosecondTime() / 1000000);
			if (Wait < CurrTime - StartTime || !Util::WaitForUnixSemaphore(MxPthreadRCountMutex, Wait - (CurrTime - StartTime)))
			{
				Util::ReleaseUnixSemaphore(MxPthreadWWaitMutex, NULL);

				return false;
			}

			// Update the event state.
			if (!Util::ResetUnixEvent(MxPthreadRWaitEvent))
			{
				Util::ReleaseUnixSemaphore(MxPthreadRCountMutex, NULL);
				Util::ReleaseUnixSemaphore(MxPthreadWWaitMutex, NULL);

				return false;
			}

			// Increment the number of readers.
			MxRCount[0]++;

			// Release the mutexes.
			Util::ReleaseUnixSemaphore(MxPthreadRCountMutex, NULL);
			Util::ReleaseUnixSemaphore(MxPthreadWWaitMutex, NULL);

			return true;
		}

		bool ReadWriteLock::WriteLock(std::uint32_t Wait)
		{
			std::uint64_t StartTime, CurrTime;

			if (MxMem == NULL)  return false;

			// Get current time in milliseconds.
			StartTime = (Wait == INFINITE ? 0 : Util::GetUnixMicrosecondTime() / 1000000);

			// Acquire the write lock mutex.
			if (!Util::WaitForUnixSemaphore(MxPthreadWWaitMutex, Wait))  return false;

			// Wait for readers to reach zero.
			CurrTime = (Wait == INFINITE ? 0 : Util::GetUnixMicrosecondTime() / 1000000);
			if (Wait < CurrTime - StartTime || !Util::WaitForUnixEvent(MxPthreadRWaitEvent, Wait - (CurrTime - StartTime)))
			{
				Util::ReleaseUnixSemaphore(MxPthreadWWaitMutex, NULL);

				return false;
			}

			return true;
		}

		bool ReadWriteLock::ReadUnlock()
		{
			if (MxMem == NULL)  return false;

			// Acquire the counter mutex.
			if (!Util::WaitForUnixSemaphore(MxPthreadRCountMutex, INFINITE))  return false;

			// Decrease the number of readers.
			if (MxRCount[0])  MxRCount[0]--;
			else
			{
				Util::ReleaseUnixSemaphore(MxPthreadRCountMutex, NULL);

				return false;
			}

			// Update the event state.
			if (!MxRCount[0] && !Util::FireUnixEvent(MxPthreadRWaitEvent))
			{
				Util::ReleaseUnixSemaphore(MxPthreadRCountMutex, NULL);

				return false;
			}

			// Release the counter mutex.
			Util::ReleaseUnixSemaphore(MxPthreadRCountMutex, NULL);

			return true;
		}

		bool ReadWriteLock::WriteUnlock()
		{
			if (MxMem == NULL)  return false;

			// Release the write lock.
			Util::ReleaseUnixSemaphore(MxPthreadWWaitMutex, NULL);

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
