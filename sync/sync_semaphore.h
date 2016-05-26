// Cross-platform, optionally named (cross-process), semaphore.
// (C) 2016 CubicleSoft.  All Rights Reserved.

#ifndef CUBICLESOFT_SYNC_SEMAPHORE
#define CUBICLESOFT_SYNC_SEMAPHORE

#include "sync_util.h"

namespace CubicleSoft
{
	namespace Sync
	{
		class Semaphore
		{
		public:
			Semaphore();
			~Semaphore();

			bool Create(const char *Name = NULL, int InitialVal = 1);

			// Wait is in milliseconds.  Granularity is in 5ms intervals on some platforms.
			bool Lock(std::uint32_t Wait = INFINITE);

			// Should only be called after a successful Lock().  Undefined behavior otherwise.
			bool Unlock(int *PrevCount = NULL);

			// Automatically unlock a semaphore when leaving the current scope.
			class AutoUnlock
			{
			public:
				AutoUnlock(Semaphore *LockPtr);
				~AutoUnlock();

			private:
				// Deny copy constructor and assignment operator.  Use a (smart) pointer instead.
				AutoUnlock(const AutoUnlock &);
				AutoUnlock &operator=(const AutoUnlock &);

				Semaphore *MxLock;
			};

		private:
			// Deny copy constructor and assignment operator.  Use a (smart) pointer instead.
			Semaphore(const Semaphore &);
			Semaphore &operator=(const Semaphore &);

#if defined(_WIN32) || defined(WIN32) || defined(_WIN64) || defined(WIN64)
			HANDLE MxWinSemaphore;
#else
			bool MxNamed;
			char *MxMem;
			Util::UnixSemaphoreWrapper MxPthreadSemaphore;
#endif
		};
	}
}

#endif
