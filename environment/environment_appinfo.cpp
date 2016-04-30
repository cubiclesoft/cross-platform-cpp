// Cross-platform application information.  For Unicode application strings, see 'UTF8::AppInfo'.
// (C) 2016 CubicleSoft.  All Rights Reserved.

#include "environment_appinfo.h"

#if defined(_WIN32) || defined(WIN32) || defined(_WIN64) || defined(WIN64)
#else
	#include <sys/time.h>
#endif

namespace CubicleSoft
{
	namespace Environment
	{
#if defined(_WIN32) || defined(WIN32) || defined(_WIN64) || defined(WIN64)
		// Windows.
		ProcessIDType AppInfo::GetCurrentProcessID()
		{
			return ::GetCurrentProcessId();
		}

		ThreadIDType AppInfo::GetCurrentThreadID()
		{
			return ::GetCurrentThreadId();
		}

		std::uint64_t AppInfo::GetUnixMicrosecondTime()
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
		ProcessIDType AppInfo::GetCurrentProcessID()
		{
			return getpid();
		}

		// POSIX pthreads.
		ThreadIDType AppInfo::GetCurrentThreadID()
		{
			return pthread_self();
		}

		std::uint64_t AppInfo::GetUnixMicrosecondTime()
		{
			struct timeval TempTime;

			if (gettimeofday(&TempTime, NULL))  return 0;

			return (std::uint64_t)((std::uint64_t)TempTime.tv_sec * (std::uint64_t)1000000 + (std::uint64_t)TempTime.tv_usec);
		}
#endif
	}
}
