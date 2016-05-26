// Cross-platform, named (cross-process), shared memory with two auto events.
// (C) 2016 CubicleSoft.  All Rights Reserved.

#ifndef CUBICLESOFT_SYNC_SHAREDMEM
#define CUBICLESOFT_SYNC_SHAREDMEM

#include "sync_util.h"

namespace CubicleSoft
{
	namespace Sync
	{
		class SharedMem
		{
		public:
			SharedMem();
			~SharedMem();

			// Returns -1 for failure, 0 for first time initialization, or 1 otherwise.
			// For platform consistency, Name + Size is a unique key.  size_t is, of course, limited to 4GB RAM on most platforms.
			// On some platforms (e.g. Windows), the event objects may vanish if all handles are freed.
			// Therefore, PrefireEvent# specifies whether or not to fire (signal) the event if it is created during Create().
			int Create(const char *Name, size_t Size, bool PrefireEvent1, bool PrefireEvent2);

			// When 0 is returned from Create(), call MemReady() after initializing the memory.
			// The shared memory is locked until this function is called.
			void Ready();

			// Wait time is in milliseconds.  Granularity is in 5ms intervals on some platforms.
			bool WaitEvent1(std::uint32_t Wait = INFINITE);

			// Lets a thread through that is waiting.
			bool FireEvent1();

			// Wait time is in milliseconds.  Granularity is in 5ms intervals on some platforms.
			bool WaitEvent2(std::uint32_t Wait = INFINITE);

			// Lets a thread through that is waiting.
			bool FireEvent2();

			inline size_t GetSize()  { return MxSize; }
			inline char *RawData()  { return MxMem; }

		private:
			// Deny copy constructor and assignment operator.  Use a (smart) pointer instead.
			SharedMem(const SharedMem &);
			SharedMem &operator=(const SharedMem &);

			size_t MxSize;
			char *MxMem;

#if defined(_WIN32) || defined(WIN32) || defined(_WIN64) || defined(WIN64)
			HANDLE MxWinMutex, MxWinEvent1, MxWinEvent2;
#else
			char *MxMemInternal;
			Util::UnixEventWrapper MxPthreadEvent1, MxPthreadEvent2;
#endif
		};
	}
}

#endif
