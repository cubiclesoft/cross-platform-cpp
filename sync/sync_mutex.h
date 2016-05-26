// Cross-platform, optionally named (cross-process), self-counting (recursive) mutex.
// (C) 2016 CubicleSoft.  All Rights Reserved.

#ifndef CUBICLESOFT_SYNC_MUTEX
#define CUBICLESOFT_SYNC_MUTEX

#include "sync_util.h"

namespace CubicleSoft
{
	namespace Sync
	{
		class Mutex
		{
		public:
			Mutex();
			~Mutex();

			bool Create(const char *Name = NULL);

			// Wait is in milliseconds.  Granularity is in 5ms intervals on some platforms.
			bool Lock(std::uint32_t Wait = INFINITE);

			bool Unlock(bool All = false);

			// Automatically unlock a mutex when leaving the current scope.
			class AutoUnlock
			{
			public:
				AutoUnlock(Mutex *LockPtr);
				~AutoUnlock();

			private:
				// Deny copy constructor and assignment operator.  Use a (smart) pointer instead.
				AutoUnlock(const AutoUnlock &);
				AutoUnlock &operator=(const AutoUnlock &);

				Mutex *MxLock;
			};

		private:
			// Deny copy constructor and assignment operator.  Use a (smart) pointer instead.
			Mutex(const Mutex &);
			Mutex &operator=(const Mutex &);

#if defined(_WIN32) || defined(WIN32) || defined(_WIN64) || defined(WIN64)
			CRITICAL_SECTION MxWinCritSection;

			HANDLE MxWinMutex;
#else
			pthread_mutex_t MxPthreadCritSection;

			bool MxNamed;
			char *MxMem;
			Util::UnixSemaphoreWrapper MxPthreadMutex;
#endif

			volatile ThreadIDType MxOwnerID;
			volatile std::uint32_t MxCount;
		};
	}
}

#endif
