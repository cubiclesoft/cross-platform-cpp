// Cross-platform thread local storage memory allocation class.  Built for short-lived, small, in-thread memory allocations.
// (C) 2013 CubicleSoft.  All Rights Reserved.

#ifndef CUBICLESOFT_SYNC_TLS
#define CUBICLESOFT_SYNC_TLS

#include "sync_util.h"
#include "../templates/detachable_queue.h"
#include "../templates/static_vector.h"

#if defined(_WIN32) || defined(WIN32) || defined(_WIN64) || defined(WIN64)
	#include <windows.h>
#else
	#include <pthread.h>
#endif

namespace CubicleSoft
{
	namespace Sync
	{
		// Only the main thread in the main executable should instantiate this object.
		// Pass to other threads and shared objects/DLLs or use globally via shared memory.
		class TLS
		{
		public:
			TLS();
			~TLS();

			class AutoFree
			{
			public:
				AutoFree(TLS *TLSPtr, void *Data);
				~AutoFree();

			private:
				// Deny copy constructor and assignment operator.  Use a (smart) pointer instead.
				AutoFree(const AutoFree &);
				AutoFree &operator=(const AutoFree &);

				TLS *MxTLS;
				void *MxData;
			};

			// Initializes the local thread cache to cache allocations (Default is 15, 2 ^ 15 = up to 32K allocations).
			// It is highly recommended to surround all code between ThreadInit() and ThreadEnd() with braces, especially when using AutoFree.
			// Only call this function once per thread per TLS instance.
			bool ThreadInit(size_t MaxCacheBits = 15);

			// Standard malloc()-like call.  Do not mix with real malloc/realloc/free!  Do not send to other threads.
			void *malloc(size_t Size);

			// Standard realloc()-like call.
			void *realloc(void *Data, size_t NewSize, bool Cache = true);

			// Duplicates memory using the real malloc() call, shallow copies Data, and free()'s Data for reuse.
			// For real new, just use a copy constructor and then free the memory.  That way a deep copy can more naturally happen.
			void *dup_malloc(void *Data, bool Cache = true);

			// Standard free()-like call.
			void free(void *Data, bool Cache = true);

			// Extract stats.
			bool GetBucketInfo(size_t Num, size_t &Nodes, size_t &Size);

			// Frees up all resources associated with the local thread cache.
			bool ThreadEnd();

		private:
			// Deny copy constructor and assignment operator.  Use a (smart) pointer instead.
			TLS(const TLS &);
			TLS &operator=(const TLS &);

#if defined(_WIN32) || defined(WIN32) || defined(_WIN64) || defined(WIN64)
			DWORD MxTlsIndex;
#else
			pthread_key_t MxKey;
#endif

			bool SetMainPtr(StaticVector<Queue<char>> *MainPtr);
			StaticVector<Queue<char>> *GetMainPtr();

			static size_t NormalizeBitPosition(size_t &Size);
		};
	}
}

#endif
