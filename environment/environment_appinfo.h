// Cross-platform application information.  For Unicode application strings, see 'UTF8::AppInfo'.
// (C) 2016 CubicleSoft.  All Rights Reserved.

#ifndef CUBICLESOFT_ENVIRONMENT_APPINFO
#define CUBICLESOFT_ENVIRONMENT_APPINFO

#include <cstdint>

#if defined(_WIN32) || defined(WIN32) || defined(_WIN64) || defined(WIN64)
	#include <windows.h>
#else
	#include <sys/types.h>
	#include <unistd.h>
	#include <pthread.h>
#endif

namespace CubicleSoft
{
	namespace Environment
	{
#if defined(_WIN32) || defined(WIN32) || defined(_WIN64) || defined(WIN64)
		typedef DWORD ProcessIDType;
		typedef DWORD ThreadIDType;
#else
		typedef pid_t ProcessIDType;
		typedef pthread_t ThreadIDType;
#endif

		class AppInfo
		{
		public:
			static ProcessIDType GetCurrentProcessID();
			static ThreadIDType GetCurrentThreadID();
			static std::uint64_t GetUnixMicrosecondTime();
		};
	}
}

#endif
