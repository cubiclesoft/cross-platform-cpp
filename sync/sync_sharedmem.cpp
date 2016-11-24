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
		SharedMem::SharedMem() : MxFirst(false), MxSize(0), MxMem(NULL), MxWinMutex(NULL)
		{
		}

		SharedMem::~SharedMem()
		{
			if (MxWinMutex != NULL)
			{
				::ReleaseMutex(MxWinMutex);
				::CloseHandle(MxWinMutex);
			}

			if (MxMem != NULL)  ::UnmapViewOfFile(MxMem);
		}

		bool SharedMem::Create(const char *Name, size_t Size)
		{
			if (MxWinMutex != NULL)
			{
				::ReleaseMutex(MxWinMutex);
				::CloseHandle(MxWinMutex);
			}

			if (MxMem != NULL)  ::UnmapViewOfFile(MxMem);

			MxWinMutex = NULL;
			MxMem = NULL;
			MxFirst = false;

			SECURITY_ATTRIBUTES SecAttr;

			SecAttr.nLength = sizeof(SecAttr);
			SecAttr.lpSecurityDescriptor = NULL;
			SecAttr.bInheritHandle = TRUE;

			char *Name2 = new char[strlen(Name) + 30];

			// Create the mutex and event objects.
			sprintf(Name2, "%s-%u-Sync_SharedMem-0", Name, (unsigned int)Size);
			MxWinMutex = ::CreateMutexA(&SecAttr, FALSE, Name2);

			if (MxWinMutex == NULL || ::WaitForSingleObject(MxWinMutex, INFINITE) != WAIT_OBJECT_0)
			{
				delete[] Name2;

				return false;
			}

			// Create the file mapping object backed by the system page file.
			int Result = -1;
			sprintf(Name2, "%s-%u-Sync_SharedMem-1", Name, (unsigned int)Size);
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

			if (Result == 0)  MxFirst = true;
			else
			{
				::ReleaseMutex(MxWinMutex);
				::CloseHandle(MxWinMutex);
				MxWinMutex = NULL;
			}

			return (Result > -1);
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

#else
		// POSIX pthreads.
		SharedMem::SharedMem() : MxFirst(false), MxSize(0), MxMem(NULL), MxMemInternal(NULL)
		{
		}

		SharedMem::~SharedMem()
		{
			if (MxMemInternal != NULL)  Util::UnmapUnixNamedMem(MxMemInternal, MxSize);
		}

		bool SharedMem::Create(const char *Name, size_t Size)
		{
			if (MxMemInternal != NULL)  Util::UnmapUnixNamedMem(MxMemInternal, MxSize);

			MxMemInternal = NULL;
			MxMem = NULL;

			size_t Pos, TempSize = Size;
			int Result = Util::InitUnixNamedMem(MxMemInternal, Pos, "/Sync_SharedMem", Name, TempSize);

			if (Result < 0)  return false;

			// Load the pointers.
			MxMem = MxMemInternal + Pos;
			MxSize = Size;

			// Handle the first time this event has been opened.
			if (Result == 0)  MxFirst = true;

			return (Result > -1);
		}

		void SharedMem::Ready()
		{
			if (MxMemInternal != NULL)  Util::UnixNamedMemReady(MxMemInternal);
		}
#endif
	}
}
