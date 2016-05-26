// Cross-platform, named (cross-process), shared memory with two auto events.
// (C) 2016 CubicleSoft.  All Rights Reserved.

#define _CRT_SECURE_NO_WARNINGS

#include "sync_sharedmem.h"

#if defined(_WIN32) || defined(WIN32) || defined(_WIN64) || defined(WIN64)
	#include <cstdio>
#else
#endif

namespace CubicleSoft
{
	namespace Sync
	{
#if defined(_WIN32) || defined(WIN32) || defined(_WIN64) || defined(WIN64)
		// Windows.
		SharedMem::SharedMem() : MxSize(0), MxMem(NULL), MxWinMutex(NULL), MxWinEvent1(NULL), MxWinEvent2(NULL)
		{
		}

		SharedMem::~SharedMem()
		{
			if (MxWinMutex != NULL)
			{
				::ReleaseMutex(MxWinMutex);
				::CloseHandle(MxWinMutex);
			}

			if (MxWinEvent1 != NULL)  ::CloseHandle(MxWinEvent1);
			if (MxWinEvent2 != NULL)  ::CloseHandle(MxWinEvent2);
			if (MxMem != NULL)  ::UnmapViewOfFile(MxMem);
		}

		int SharedMem::Create(const char *Name, size_t Size, bool PrefireEvent1, bool PrefireEvent2)
		{
			if (MxWinMutex != NULL)
			{
				::ReleaseMutex(MxWinMutex);
				::CloseHandle(MxWinMutex);
			}

			if (MxWinEvent1 != NULL)  ::CloseHandle(MxWinEvent1);
			if (MxWinEvent2 != NULL)  ::CloseHandle(MxWinEvent2);
			if (MxMem != NULL)  ::UnmapViewOfFile(MxMem);

			MxWinMutex = NULL;
			MxWinEvent1 = NULL;
			MxWinEvent2 = NULL;
			MxMem = NULL;

			SECURITY_ATTRIBUTES SecAttr;

			SecAttr.nLength = sizeof(SecAttr);
			SecAttr.lpSecurityDescriptor = NULL;
			SecAttr.bInheritHandle = TRUE;

			char *Name2 = new char[strlen(Name) + 30];

			// Create the mutex and event objects.
			sprintf(Name2, "%s-%u-Sync_SharedMem-0", Name, (unsigned int)Size);
			MxWinMutex = ::CreateMutexA(&SecAttr, FALSE, Name2);
			sprintf(Name2, "%s-%u-Sync_SharedMem-1", Name, (unsigned int)Size);
			MxWinEvent1 = ::CreateEventA(&SecAttr, FALSE, FALSE, Name2);
			if (MxWinEvent1 != NULL && PrefireEvent1 && ::GetLastError() != ERROR_ALREADY_EXISTS)  FireEvent1();
			sprintf(Name2, "%s-%u-Sync_SharedMem-2", Name, (unsigned int)Size);
			MxWinEvent2 = ::CreateEventA(&SecAttr, FALSE, FALSE, Name2);
			if (MxWinEvent2 != NULL && PrefireEvent2 && ::GetLastError() != ERROR_ALREADY_EXISTS)  FireEvent2();

			if (MxWinMutex == NULL || MxWinEvent1 == NULL || MxWinEvent2 == NULL || ::WaitForSingleObject(MxWinMutex, INFINITE) != WAIT_OBJECT_0)
			{
				delete[] Name2;

				return -1;
			}

			// Create the file mapping object backed by the system page file.
			int Result = -1;
			sprintf(Name2, "%s-%u-Sync_SharedMem-3", Name, (unsigned int)Size);
			HANDLE TempFile = ::CreateFileMappingA(INVALID_HANDLE_VALUE, &SecAttr, PAGE_READWRITE, 0, (DWORD)Size, Name2);
			if (TempFile == NULL)
			{
				TempFile = ::OpenFileMappingA(FILE_MAP_ALL_ACCESS, TRUE, Name2);
				if (TempFile != NULL)  Result = 1;
			}
			else if (::GetLastError() == ERROR_ALREADY_EXISTS)
			{
				Result = 1;
			}
			else
			{
				Result = 0;
			}

			delete[] Name2;

			if (TempFile != NULL)
			{
				MxMem = (char *)::MapViewOfFile(TempFile, FILE_MAP_ALL_ACCESS, 0, 0, (DWORD)Size);
				if (MxMem == NULL)  Result = -1;
				else  MxSize = Size;

				::CloseHandle(TempFile);
			}

			if (Result != 0)
			{
				::ReleaseMutex(MxWinMutex);
				::CloseHandle(MxWinMutex);
				MxWinMutex = NULL;
			}

			return Result;
		}

		void SharedMem::Ready()
		{
			if (MxWinMutex != NULL)
			{
				::ReleaseMutex(MxWinMutex);
				::CloseHandle(MxWinMutex);
				MxWinMutex = NULL;
			}
		}

		bool SharedMem::WaitEvent1(std::uint32_t Wait)
		{
			if (MxWinEvent1 == NULL)  return false;

			DWORD Result = ::WaitForSingleObject(MxWinEvent1, Wait);
			if (Result != WAIT_OBJECT_0)  return false;

			return true;
		}

		bool SharedMem::FireEvent1()
		{
			if (MxWinEvent1 == NULL)  return false;

			if (!::SetEvent(MxWinEvent1))  return false;

			return true;
		}

		bool SharedMem::WaitEvent2(std::uint32_t Wait)
		{
			if (MxWinEvent2 == NULL)  return false;

			DWORD Result = ::WaitForSingleObject(MxWinEvent2, Wait);
			if (Result != WAIT_OBJECT_0)  return false;

			return true;
		}

		bool SharedMem::FireEvent2()
		{
			if (MxWinEvent2 == NULL)  return false;

			if (!::SetEvent(MxWinEvent2))  return false;

			return true;
		}

#else
		// POSIX pthreads.
		SharedMem::SharedMem() : MxSize(0), MxMem(NULL), MxMemInternal(NULL)
		{
		}

		SharedMem::~SharedMem()
		{
			if (MxMemInternal != NULL)  Util::UnmapUnixNamedMem(MxMemInternal, Util::GetUnixEventSize() * 2 + MxSize);
		}

		int SharedMem::Create(const char *Name, size_t Size, bool PrefireEvent1, bool PrefireEvent2)
		{
			if (MxMemInternal != NULL)  Util::UnmapUnixNamedMem(MxMemInternal, Util::GetUnixEventSize() * 2 + MxSize);

			MxMemInternal = NULL;
			MxMem = NULL;

			size_t Pos, TempSize = Util::GetUnixEventSize() * 2 + Size;
			int Result = Util::InitUnixNamedMem(MxMemInternal, Pos, "/Sync_SharedMem", Name, TempSize);

			if (Result < 0)  return false;

			// Load the pointers.
			char *MemPtr = MxMemInternal + Pos;
			Util::GetUnixEvent(MxPthreadEvent1, MemPtr);
			MemPtr += Util::GetUnixEventSize();

			Util::GetUnixEvent(MxPthreadEvent2, MemPtr);
			MemPtr += Util::GetUnixEventSize();

			MxMem = MemPtr;
			MxSize = Size;

			// Handle the first time this event has been opened.
			if (Result == 0)
			{
				Util::InitUnixEvent(MxPthreadEvent1, true, false);
				Util::InitUnixEvent(MxPthreadEvent2, true, false);

				if (PrefireEvent1)  FireEvent1();
				if (PrefireEvent2)  FireEvent2();
			}

			return Result;
		}

		void SharedMem::Ready()
		{
			if (MxMemInternal != NULL)  Util::UnixNamedMemReady(MxMemInternal);
		}

		bool SharedMem::WaitEvent1(std::uint32_t Wait)
		{
			if (MxMemInternal == NULL)  return false;

			// Wait for the event.
			return Util::WaitForUnixEvent(MxPthreadEvent1, Wait);
		}

		bool SharedMem::FireEvent1()
		{
			if (MxMemInternal == NULL)  return false;

			return Util::FireUnixEvent(MxPthreadEvent1);
		}

		bool SharedMem::WaitEvent2(std::uint32_t Wait)
		{
			if (MxMemInternal == NULL)  return false;

			// Wait for the event.
			return Util::WaitForUnixEvent(MxPthreadEvent2, Wait);
		}

		bool SharedMem::FireEvent2()
		{
			if (MxMemInternal == NULL)  return false;

			return Util::FireUnixEvent(MxPthreadEvent2);
		}
#endif
	}
}
