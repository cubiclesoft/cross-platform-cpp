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
		SharedMem::SharedMem() : MxFirst(false), MxSize(0), MxMem(NULL), MxFile(NULL)
		{
		}

		SharedMem::~SharedMem()
		{
			if (MxMem != NULL)  ::UnmapViewOfFile(MxMem);
			if (MxFile != NULL)  ::CloseHandle(MxFile);
		}

		bool SharedMem::Create(const char *Name, size_t Size)
		{
			if (Name == NULL)  return false;

			if (MxMem != NULL)  ::UnmapViewOfFile(MxMem);
			if (MxFile != NULL)  ::CloseHandle(MxFile);

			MxMem = NULL;
			MxFile = NULL;
			MxFirst = false;

			SECURITY_ATTRIBUTES SecAttr;

			SecAttr.nLength = sizeof(SecAttr);
			SecAttr.lpSecurityDescriptor = NULL;
			SecAttr.bInheritHandle = TRUE;

			char *Name2 = new char[strlen(Name) + 30];

			// Create the file mapping object backed by the system page file.
			sprintf(Name2, "%s-%u-Sync_SharedMem", Name, (unsigned int)Size);
			MxFile = ::CreateFileMappingA(INVALID_HANDLE_VALUE, &SecAttr, PAGE_READWRITE, 0, (DWORD)Size, Name2);
			if (MxFile == NULL)
			{
				MxFile = ::OpenFileMappingA(FILE_MAP_ALL_ACCESS, TRUE, Name2);
				if (MxFile == NULL)  return false;
			}
			else if (::GetLastError() != ERROR_ALREADY_EXISTS)
			{
				MxFirst = true;
			}

			delete[] Name2;

			MxMem = (char *)::MapViewOfFile(MxFile, FILE_MAP_ALL_ACCESS, 0, 0, (DWORD)Size);
			if (MxMem == NULL)  return false;

			MxSize = Size;

			return true;
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
			if (Name == NULL)  return false;

			if (MxMemInternal != NULL)  Util::UnmapUnixNamedMem(MxMemInternal, MxSize);

			MxMemInternal = NULL;
			MxMem = NULL;

			size_t Pos, TempSize = Size;
			int Result = Util::InitUnixNamedMem(MxMemInternal, Pos, "/Sync_SharedMem", Name, TempSize);

			if (Result < 0)  return false;

			// Load the pointers.
			MxMem = MxMemInternal + Pos;
			MxSize = Size;

			// Handle the first time this named memory has been opened.
			if (Result == 0)
			{
				Util::UnixNamedMemReady(MxMemInternal);

				MxFirst = true;
			}

			return true;
		}
#endif
	}
}
