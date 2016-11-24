// Cross-platform, optionally named (cross-process), event object (e.g. for producer/consumer queues).
// (C) 2016 CubicleSoft.  All Rights Reserved.

#include "sync_event.h"

namespace CubicleSoft
{
	namespace Sync
	{
#if defined(_WIN32) || defined(WIN32) || defined(_WIN64) || defined(WIN64)
		// Windows.
		Event::Event() : MxWinWaitEvent(NULL)
		{
		}

		Event::~Event()
		{
			if (MxWinWaitEvent != NULL)  ::CloseHandle(MxWinWaitEvent);
		}

		bool Event::Create(const char *Name, bool Manual, bool Prefire)
		{
			if (MxWinWaitEvent != NULL)  ::CloseHandle(MxWinWaitEvent);

			MxWinWaitEvent = NULL;

			SECURITY_ATTRIBUTES SecAttr;

			SecAttr.nLength = sizeof(SecAttr);
			SecAttr.lpSecurityDescriptor = NULL;
			SecAttr.bInheritHandle = TRUE;

			MxWinWaitEvent = ::CreateEventA(&SecAttr, (BOOL)Manual, (Prefire ? TRUE : FALSE), Name);

			if (MxWinWaitEvent == NULL)  return false;

			return true;
		}

		bool Event::Wait(std::uint32_t Wait)
		{
			if (MxWinWaitEvent == NULL)  return false;

			DWORD Result = ::WaitForSingleObject(MxWinWaitEvent, Wait);
			if (Result != WAIT_OBJECT_0)  return false;

			return true;
		}

		bool Event::Fire()
		{
			if (MxWinWaitEvent == NULL)  return false;

			if (!::SetEvent(MxWinWaitEvent))  return false;

			return true;
		}

		bool Event::Reset()
		{
			if (MxWinWaitEvent == NULL)  return false;

			if (!::ResetEvent(MxWinWaitEvent))  return false;

			return true;
		}
#else
		// POSIX pthreads.
		Event::Event() : MxNamed(false), MxMem(NULL)
		{
		}

		Event::~Event()
		{
			if (MxMem != NULL)
			{
				if (MxNamed)  Util::UnmapUnixNamedMem(MxMem, Util::GetUnixEventSize());
				else
				{
					Util::FreeUnixEvent(MxPthreadEvent);

					delete[] MxMem;
				}
			}
		}

		bool Event::Create(const char *Name, bool Manual, bool Prefire)
		{
			if (MxMem != NULL)
			{
				if (MxNamed)  Util::UnmapUnixNamedMem(MxMem, Util::GetUnixEventSize());
				else
				{
					Util::FreeUnixEvent(MxPthreadEvent);

					delete[] MxMem;
				}
			}

			MxMem = NULL;

			size_t Pos, TempSize = Util::GetUnixEventSize();
			MxNamed = (Name != NULL);
			int Result = Util::InitUnixNamedMem(MxMem, Pos, "/Sync_Event", Name, TempSize);

			if (Result < 0)  return false;

			Util::GetUnixEvent(MxPthreadEvent, MxMem + Pos);

			// Handle the first time this event has been opened.
			if (Result == 0)
			{
				Util::InitUnixEvent(MxPthreadEvent, MxNamed, Manual, Prefire);

				if (MxNamed)  Util::UnixNamedMemReady(MxMem);
			}

			return true;
		}

		bool Event::Wait(std::uint32_t WaitAmt)
		{
			if (MxMem == NULL)  return false;

			// Wait for the event.
			return Util::WaitForUnixEvent(MxPthreadEvent, WaitAmt);
		}

		bool Event::Fire()
		{
			if (MxMem == NULL)  return false;

			return Util::FireUnixEvent(MxPthreadEvent);
		}

		bool Event::Reset()
		{
			if (MxMem == NULL)  return false;

			return Util::ResetUnixEvent(MxPthreadEvent);
		}
#endif
	}
}
