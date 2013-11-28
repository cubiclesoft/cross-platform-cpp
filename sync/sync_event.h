// Cross-platform, optionally named (cross-process), event object (e.g. for producer/consumer queues).
// (C) 2013 CubicleSoft.  All Rights Reserved.

#ifndef CUBICLESOFT_SYNC_EVENT
#define CUBICLESOFT_SYNC_EVENT

#include "sync_util.h"

namespace CubicleSoft
{
	namespace Sync
	{
		class Event
		{
		public:
			Event();
			~Event();

			bool Create(const char *Name = NULL, bool Manual = false);

			// Wait time is in milliseconds.  Granularity is in 5ms intervals on some platforms.
			bool Wait(std::uint32_t Wait = INFINITE);

			// Lets a thread through that is waiting.  Lets multiple threads through that are waiting if the event is 'manual'.
			bool Fire();

			// Only use when the event is 'manual'.
			bool Reset();

		private:
			// Deny copy constructor and assignment operator.  Use a (smart) pointer instead.
			Event(const Event &);
			Event &operator=(const Event &);

#if defined(_WIN32) || defined(WIN32) || defined(_WIN64) || defined(WIN64)
			HANDLE MxWinWaitEvent;
#else
			sem_t *MxSemWaitMutex, *MxSemWaitEvent, *MxSemWaitCount, *MxSemWaitStatus;
			bool MxAllocated;
			bool MxManual;
#endif
		};
	}
}

#endif
