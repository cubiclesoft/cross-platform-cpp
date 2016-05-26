// Cross-platform, cross-process synchronization utility functions.
// (C) 2016 CubicleSoft.  All Rights Reserved.

#ifndef CUBICLESOFT_SYNC_UTIL
#define CUBICLESOFT_SYNC_UTIL

#include <cstdint>

#if defined(_WIN32) || defined(WIN32) || defined(_WIN64) || defined(WIN64)
	#include <windows.h>
#else
	#include <pthread.h>
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
			// *NIX OSes require a lot of extra logic for named object support.
			// In general, do not directly call these functions.  Use the specific classes instead.

			static size_t GetUnixSystemAlignmentSize();
			static size_t AlignUnixSize(size_t Size);
			static int InitUnixNamedMem(char *&ResultMem, size_t &StartPos, const char *Prefix, const char *Name, size_t Size);
			static void UnixNamedMemReady(char *MemPtr);
			static void UnmapUnixNamedMem(char *MemPtr, size_t Size);

			// Some platforms are broken even for unnamed semaphores (e.g. Mac OSX).
			// Implements semaphores directly, bypassing POSIX semaphores.
			// Semaphores can be used in place of mutexes (i.e. Start = 1, Max = 1).
			class UnixSemaphoreWrapper
			{
			public:
				pthread_mutex_t *MxMutex;
				volatile std::uint32_t *MxCount;
				volatile std::uint32_t *MxMax;
				pthread_cond_t *MxCond;
			};

			static size_t GetUnixSemaphoreSize();
			static void GetUnixSemaphore(UnixSemaphoreWrapper &Result, char *Mem);
			static void InitUnixSemaphore(UnixSemaphoreWrapper &UnixSemaphore, bool Shared, std::uint32_t Start, std::uint32_t Max);
			static bool WaitForUnixSemaphore(UnixSemaphoreWrapper &UnixSemaphore, std::uint32_t Wait);
			static bool ReleaseUnixSemaphore(UnixSemaphoreWrapper &UnixSemaphore, std::uint32_t *PrevVal);
			static void FreeUnixSemaphore(UnixSemaphoreWrapper &UnixSemaphore);

			// Implements a more efficient (and portable) event object interface than trying to use semaphores.
			class UnixEventWrapper
			{
			public:
				pthread_mutex_t *MxMutex;
				volatile char *MxManual;
				volatile char *MxSignaled;
				volatile std::uint32_t *MxWaiting;
				pthread_cond_t *MxCond;
			};

			static size_t GetUnixEventSize();
			static void GetUnixEvent(UnixEventWrapper &Result, char *Mem);
			static void InitUnixEvent(UnixEventWrapper &UnixEvent, bool Shared, bool Manual);
			static bool WaitForUnixEvent(UnixEventWrapper &UnixEvent, std::uint32_t Wait);
			static bool FireUnixEvent(UnixEventWrapper &UnixEvent);
			static bool ResetUnixEvent(UnixEventWrapper &UnixEvent);
			static void FreeUnixEvent(UnixEventWrapper &UnixEvent);
#endif
		};
	}
}

#endif
