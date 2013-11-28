// Cross-platform, optionally named (cross-process), event object (e.g. for producer/consumer queues).
// (C) 2013 CubicleSoft.  All Rights Reserved.

#define _CRT_SECURE_NO_WARNINGS

#include <cstdio>
#include <cstring>

#include "sync_event.h"

namespace CubicleSoft
{
	namespace Sync
	{
#if defined(_WIN32) || defined(WIN32) || defined(_WIN64) || defined(WIN64)
		// Windows.
		Event::Event() : MxWinWaitEvent(NULL)
		{
		}

		Event::~Event()
		{
			if (MxWinWaitEvent != NULL)  ::CloseHandle(MxWinWaitEvent);
		}

		bool Event::Create(const char *Name, bool Manual)
		{
			if (MxWinWaitEvent != NULL)  ::CloseHandle(MxWinWaitEvent);

			MxWinWaitEvent = NULL;

			SECURITY_ATTRIBUTES SecAttr;

			SecAttr.nLength = sizeof(SecAttr);
			SecAttr.lpSecurityDescriptor = NULL;
			SecAttr.bInheritHandle = TRUE;

			MxWinWaitEvent = ::CreateEventA(&SecAttr, (BOOL)Manual, FALSE, Name);

			if (MxWinWaitEvent == NULL)  return false;

			return true;
		}

		bool Event::Wait(uint32_t Wait)
		{
			if (MxWinWaitEvent == NULL)  return false;

			DWORD Result = ::WaitForSingleObject(MxWinWaitEvent, Wait);
			if (Result != WAIT_OBJECT_0)  return false;

			return true;
		}

		bool Event::Fire()
		{
			if (MxWinWaitEvent == NULL)  return false;

			if (!::SetEvent(MxWinWaitEvent))  return false;

			return true;
		}

		bool Event::Reset()
		{
			if (MxWinWaitEvent == NULL)  return false;

			if (!::ResetEvent(MxWinWaitEvent))  return false;

			return true;
		}
#else
		// POSIX pthreads.
		Event::Event() : MxSemWaitMutex(SEM_FAILED), MxSemWaitEvent(SEM_FAILED), MxSemWaitCount(SEM_FAILED), MxSemWaitStatus(SEM_FAILED), MxAllocated(false), MxManual(false)
		{
		}

		Event::~Event()
		{
			if (MxAllocated)
			{
				if (MxSemWaitStatus != SEM_FAILED)  delete MxSemWaitStatus;
				if (MxSemWaitCount != SEM_FAILED)  delete MxSemWaitCount;
				if (MxSemWaitEvent != SEM_FAILED)  delete MxSemWaitEvent;
				if (MxSemWaitMutex != SEM_FAILED)  delete MxSemWaitMutex;
			}
			else
			{
				if (MxSemWaitStatus != SEM_FAILED)  sem_close(MxSemWaitStatus);
				if (MxSemWaitCount != SEM_FAILED)  sem_close(MxSemWaitCount);
				if (MxSemWaitEvent != SEM_FAILED)  sem_close(MxSemWaitEvent);
				if (MxSemWaitMutex != SEM_FAILED)  sem_close(MxSemWaitMutex);
			}
		}

		bool Event::Create(const char *Name, bool Manual)
		{
			if (MxAllocated)
			{
				if (MxSemWaitStatus != SEM_FAILED)  delete MxSemWaitStatus;
				if (MxSemWaitCount != SEM_FAILED)  delete MxSemWaitCount;
				if (MxSemWaitEvent != SEM_FAILED)  delete MxSemWaitEvent;
				if (MxSemWaitMutex != SEM_FAILED)  delete MxSemWaitMutex;
			}
			else
			{
				if (MxSemWaitStatus != SEM_FAILED)  sem_close(MxSemWaitStatus);
				if (MxSemWaitCount != SEM_FAILED)  sem_close(MxSemWaitCount);
				if (MxSemWaitEvent != SEM_FAILED)  sem_close(MxSemWaitEvent);
				if (MxSemWaitMutex != SEM_FAILED)  sem_close(MxSemWaitMutex);
			}

			MxSemWaitStatus = SEM_FAILED;
			MxSemWaitCount = SEM_FAILED;
			MxSemWaitEvent = SEM_FAILED;
			MxSemWaitMutex = SEM_FAILED;
			MxManual = Manual;

			if (Name != NULL)
			{
				char *Name2 = new char[strlen(Name) + 20];

				MxAllocated = false;

				sprintf(Name2, "/Sync_Event_%s_0", Name);
				MxSemWaitMutex = sem_open(Name2, O_CREAT, 0666, 1);
				sprintf(Name2, "/Sync_Event_%s_1", Name);
				MxSemWaitEvent = sem_open(Name2, O_CREAT, 0666, 0);

				if (MxManual)
				{
					sprintf(Name2, "/Sync_Event_%s_2", Name);
					MxSemWaitCount = sem_open(Name2, O_CREAT, 0666, 0);

					sprintf(Name2, "/Sync_Event_%s_3", Name);
					MxSemWaitStatus = sem_open(Name2, O_CREAT, 0666, 0);
				}

				delete[] Name2;
			}
			else
			{
				MxAllocated = true;

				MxSemWaitMutex = new sem_t;
				memset(MxSemWaitMutex, 0, sizeof(sem_t));
				sem_init(MxSemWaitMutex, 0, 1);

				MxSemWaitEvent = new sem_t;
				memset(MxSemWaitEvent, 0, sizeof(sem_t));
				sem_init(MxSemWaitEvent, 0, 0);

				if (MxManual)
				{
					MxSemWaitCount = new sem_t;
					memset(MxSemWaitCount, 0, sizeof(sem_t));
					sem_init(MxSemWaitCount, 0, 0);

					MxSemWaitStatus = new sem_t;
					memset(MxSemWaitStatus, 0, sizeof(sem_t));
					sem_init(MxSemWaitStatus, 0, 0);
				}
			}

			if (MxSemWaitMutex == SEM_FAILED || MxSemWaitEvent == SEM_FAILED || (MxManual && (MxSemWaitCount == SEM_FAILED || MxSemWaitStatus == SEM_FAILED)))  return false;

			return true;
		}

		bool Event::Wait(uint32_t WaitAmt)
		{
			uint64_t StartTime, CurrTime;

			if (MxSemWaitMutex == SEM_FAILED || MxSemWaitEvent == SEM_FAILED || (MxManual && (MxSemWaitCount == SEM_FAILED || MxSemWaitStatus == SEM_FAILED)))  return false;

			// Get current time in milliseconds.
			StartTime = (WaitAmt == INFINITE ? 0 : Util::GetUnixMicrosecondTime() / 1000000);

			if (!MxManual)  CurrTime = StartTime;
			else
			{
				// Lock the mutex.
				if (!Util::WaitForSemaphore(MxSemWaitMutex, WaitAmt))  return false;

				// Get the status.  If it is 1, then the event has been fired.
				int Val;
				sem_getvalue(MxSemWaitStatus, &Val);
				if (Val == 1)
				{
					sem_post(MxSemWaitMutex);

					return true;
				}

				// Increment the wait count.
				CurrTime = (WaitAmt == INFINITE ? 0 : Util::GetUnixMicrosecondTime() / 1000000);
				if (WaitAmt < CurrTime - StartTime)
				{
					sem_post(MxSemWaitMutex);

					return false;
				}
				sem_post(MxSemWaitCount);

				// Release the mutex.
				sem_post(MxSemWaitMutex);
			}

			// Wait for the semaphore.
			bool Result = Util::WaitForSemaphore(MxSemWaitEvent, WaitAmt - (CurrTime - StartTime));

			if (MxManual)
			{
				// Lock the mutex.
				Util::WaitForSemaphore(MxSemWaitMutex, INFINITE);

				// Decrease the wait count.
				Util::WaitForSemaphore(MxSemWaitCount, INFINITE);

				// Release the mutex.
				sem_post(MxSemWaitMutex);
			}

			return Result;
		}

		bool Event::Fire()
		{
			if (MxSemWaitMutex == SEM_FAILED || MxSemWaitEvent == SEM_FAILED)  return false;

			if (MxManual)
			{
				// Lock the mutex.
				if (!Util::WaitForSemaphore(MxSemWaitMutex, INFINITE))  return false;

				// Update the status.  No wait.
				int Val;
				sem_getvalue(MxSemWaitStatus, &Val);
				if (Val == 0)  sem_post(MxSemWaitStatus);

				// Release the mutex.
				sem_post(MxSemWaitMutex);

				// Release all waiting threads.  Might do too many sem_post() calls.
				int x;
				sem_getvalue(MxSemWaitCount, &Val);
				for (x = 0; x < Val; x++)  sem_post(MxSemWaitEvent);
			}
			else
			{
				// Release one thread.
				int Val;
				sem_getvalue(MxSemWaitEvent, &Val);
				if (Val == 0)  sem_post(MxSemWaitEvent);
			}

			return true;
		}

		bool Event::Reset()
		{
			if (!MxManual)  return false;

			// Lock the mutex.
			if (!Util::WaitForSemaphore(MxSemWaitMutex, INFINITE))  return false;

			// Restrict the semaphore.  Fixes the too many sem_post() calls in Fire().
			while (Util::WaitForSemaphore(MxSemWaitEvent, 0))  {}

			// Update the status.  Start waiting.
			int Val;
			sem_getvalue(MxSemWaitStatus, &Val);
			if (Val == 1)  Util::WaitForSemaphore(MxSemWaitStatus, INFINITE);

			// Release the mutex.
			sem_post(MxSemWaitMutex);

			return true;
		}
#endif
	}
}
