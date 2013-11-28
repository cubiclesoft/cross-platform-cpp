// Cross-platform, cross-process synchronization utility functions.
// (C) 2013 CubicleSoft.  All Rights Reserved.

#include "sync_util.h"

namespace CubicleSoft
{
	namespace Sync
	{
#if defined(_WIN32) || defined(WIN32) || defined(_WIN64) || defined(WIN64)
		// Windows.
		ThreadIDType Util::GetCurrentThreadID()
		{
			return ::GetCurrentThreadId();
		}

		std::uint64_t Util::GetUnixMicrosecondTime()
		{
			FILETIME TempTime;
			ULARGE_INTEGER TempTime2;
			std::uint64_t Result;

			::GetSystemTimeAsFileTime(&TempTime);
			TempTime2.HighPart = TempTime.dwHighDateTime;
			TempTime2.LowPart = TempTime.dwLowDateTime;
			Result = TempTime2.QuadPart;

			Result = (Result / 10) - (std::uint64_t)11644473600000000ULL;

			return Result;
		}
#else
		// POSIX pthreads.
		ThreadIDType Util::GetCurrentThreadID()
		{
			return pthread_self();
		}

		std::uint64_t Util::GetUnixMicrosecondTime()
		{
			struct timeval TempTime;

			if (gettimeofday(&TempTime, NULL))  return 0;

			return (std::uint64_t)((std::uint64_t)TempTime.tv_sec * (std::uint64_t)1000000 + (std::uint64_t)TempTime.tv_usec);
		}

		// Simplifies timer-based mutex/semaphore acquisition.
		bool Util::WaitForSemaphore(sem_t *SemPtr, std::uint32_t Wait)
		{
			if (SemPtr == SEM_FAILED)  return false;

			if (Wait == INFINITE)
			{
				while (sem_wait(SemPtr) < 0)
				{
					if (errno != EINTR)  return false;
				}
			}
			else if (Wait == 0)
			{
				while (sem_trywait(SemPtr) < 0)
				{
					if (errno != EINTR)  return false;
				}
			}
			else
			{
				struct timespec TempTime;

				if (clock_gettime(CLOCK_REALTIME, &TempTime) == -1)  return false;
				TempTime.tv_sec += Wait / 1000;
				TempTime.tv_nsec += (Wait % 1000) * 1000000;
				TempTime.tv_sec += TempTime.tv_nsec / 1000000000;
				TempTime.tv_nsec = TempTime.tv_nsec % 1000000000;

				while (sem_timedwait(SemPtr, &TempTime) < 0)
				{
					if (errno != EINTR)  return false;
				}
			}

			return true;
		}
#endif
	}
}
