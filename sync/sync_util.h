// Cross-platform, cross-process synchronization utility functions.
// (C) 2013 CubicleSoft.  All Rights Reserved.

#ifndef CUBICLESOFT_SYNC_UTIL
#define CUBICLESOFT_SYNC_UTIL

#include <cstdint>

#if defined(_WIN32) || defined(WIN32) || defined(_WIN64) || defined(WIN64)
	#include <windows.h>
#else
	#include <sys/types.h>
	#include <sys/stat.h>
	#include <sys/time.h>
	#include <fcntl.h>
	#include <unistd.h>
	#include <errno.h>
	#include <pthread.h>
	#include <semaphore.h>
	#include <limits.h>
#endif

#ifndef INFINITE
	#define INFINITE   0xFFFFFFFF
#endif

// Cross-platform utility class/functions.
namespace CubicleSoft
{
	namespace Sync
	{
#if defined(_WIN32) || defined(WIN32) || defined(_WIN64) || defined(WIN64)
		typedef DWORD ThreadIDType;
#else
		typedef pthread_t ThreadIDType;
#endif

		class Util
		{
		public:
			static ThreadIDType GetCurrentThreadID();
			static std::uint64_t GetUnixMicrosecondTime();

#if defined(_WIN32) || defined(WIN32) || defined(_WIN64) || defined(WIN64)
#else
			static bool WaitForSemaphore(sem_t *SemPtr, std::uint32_t Wait);
#endif
		};
	}
}

#endif
