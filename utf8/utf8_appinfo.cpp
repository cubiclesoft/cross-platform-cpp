// Cross-platform Unicode application information.
// (C) 2016 CubicleSoft.  All Rights Reserved.

#include "utf8_appinfo.h"

#include <cstring>

#if defined(_WIN32) || defined(WIN32) || defined(_WIN64) || defined(WIN64)
	#include <ShlObj.h>
#else
	#include <limits.h>
	#include <stdlib.h>
	#include <pwd.h>

	#ifdef __APPLE__
		#include <libproc.h>
	#endif
#endif

namespace CubicleSoft
{
	namespace UTF8
	{
#if defined(_WIN32) || defined(WIN32) || defined(_WIN64) || defined(WIN64)
		bool AppInfo::GetExecutableFilename(char *Buffer, size_t &BufferSize, const char *Argv0)
		{
			size_t z = BufferSize;
			size_t z2 = z;
			BufferSize = 0;
			if (z2)  Buffer[0] = '\0';

			// Retrieve the full EXE path and filename for this process.
			WCHAR Filename[8192];
			size_t y = (size_t)::GetModuleFileNameW(NULL, Filename, sizeof(Filename) / sizeof(WCHAR));
			if (y >= sizeof(Filename) / sizeof(WCHAR) - 1)  return false;
			Filename[y] = L'\0';

			Util::ConvertToUTF8(Filename, y + 1, sizeof(WCHAR), (std::uint8_t *)Buffer, z);
			if (z == z2)  return false;
			BufferSize = z;

			return true;
		}

		bool AppInfo::GetSystemAppStorageDir(char *Buffer, size_t &BufferSize, const char *AppDir)
		{
			size_t z = BufferSize;
			size_t z2 = z;
			BufferSize = 0;
			if (z2)  Buffer[0] = '\0';

			WCHAR Dirname[8192];
			if (::SHGetFolderPathW(NULL, CSIDL_COMMON_APPDATA | CSIDL_FLAG_CREATE, NULL, 0, Dirname) != S_OK)  return false;

			return GetWindowsUTF8StorageDir(Dirname, Buffer, BufferSize, AppDir, z, z2);
		}

		bool AppInfo::GetCurrentUserAppStorageDir(char *Buffer, size_t &BufferSize, const char *AppDir)
		{
			size_t z = BufferSize;
			size_t z2 = z;
			BufferSize = 0;
			if (z2)  Buffer[0] = '\0';

			WCHAR Dirname[8192];
			if (::SHGetFolderPathW(NULL, CSIDL_LOCAL_APPDATA | CSIDL_FLAG_CREATE, NULL, 0, Dirname) != S_OK)  return false;

			return GetWindowsUTF8StorageDir(Dirname, Buffer, BufferSize, AppDir, z, z2);
		}

		bool AppInfo::GetTempStorageDir(char *Buffer, size_t &BufferSize)
		{
			size_t z2 = BufferSize;
			BufferSize = 0;
			if (z2)  Buffer[0] = '\0';

			WCHAR Dirname[8192];
			size_t z = (size_t)::GetTempPathW(sizeof(Dirname) / sizeof(WCHAR), Dirname);
			if (!z)  return false;

			return GetWindowsUTF8StorageDir(Dirname, Buffer, BufferSize, NULL, z, z2);
		}

		// Internal function to standardize the conversion to UTF-8 on Windows.
		bool AppInfo::GetWindowsUTF8StorageDir(WCHAR *Dirname, char *Buffer, size_t &BufferSize, const char *AppDir, size_t z, size_t z2)
		{
			Util::ConvertToUTF8(Dirname, wcslen(Dirname), sizeof(WCHAR), (std::uint8_t *)Buffer, z);

			if (z < z2 && Buffer[z - 1] != '\\')  Buffer[z++] = '\\';
			if (AppDir != NULL)
			{
				while (z < z2 && *AppDir)  Buffer[z++] = *AppDir++;

				if (z < z2 && Buffer[z - 1] != '\\')  Buffer[z++] = '\\';
			}
			if (z < z2)  Buffer[z++] = '\0';
			if (z == z2)  return false;
			BufferSize = z;

			return true;
		}

#else

		bool AppInfo::GetExecutableFilename(char *Buffer, size_t &BufferSize, const char *Argv0)
		{
			size_t z2 = BufferSize;
			BufferSize = 0;
			if (z2)  Buffer[0] = '\0';

			// Linux.
			ssize_t z = readlink("/proc/self/exe", Buffer, z2);

			// FreeBSD.
			if (z == -1)  z = readlink("/proc/curproc/file", Buffer, z2);

			// NetBSD.
			if (z == -1)  z = readlink("/proc/curproc/exe", Buffer, z2);

			// Solaris.
			if (z == -1)  z = readlink("/proc/self/path/a.out", Buffer, z2);

			// Mac OSX.
	#ifdef __APPLE__
			if (z == -1)  z = proc_pidpath(getpid(), Buffer, z2);
	#endif

			char *Filename;
			if (z > -1)
			{
				if ((size_t)z < z2)  Buffer[z++] = '\0';

				Filename = realpath(Buffer, NULL);
			}
			else
			{
				// Other *NIX-style OS.
				if (Argv0 == NULL)  return false;

				Filename = realpath(Argv0, NULL);
			}

			if (Filename == NULL)  return false;

			if (strlen(Filename) >= z2)
			{
				free(Filename);

				return false;
			}

			strcpy(Buffer, Filename);
			BufferSize = strlen(Buffer) + 1;

			free(Filename);

			return true;
		}

		bool AppInfo::GetSystemAppStorageDir(char *Buffer, size_t &BufferSize, const char *AppDir)
		{
			size_t z2 = BufferSize;
			BufferSize = 0;
			if (z2)  Buffer[0] = '\0';

			if (z2 < 10)  return false;

			strcpy(Buffer, "/var/lib/");
			size_t z = strlen(Buffer);

			return GetNIXStorageDir(Buffer, BufferSize, NULL, AppDir, z, z2);
		}

		bool AppInfo::GetCurrentUserAppStorageDir(char *Buffer, size_t &BufferSize, const char *AppDir)
		{
			size_t z2 = BufferSize;
			BufferSize = 0;
			if (z2)  Buffer[0] = '\0';

			const char *HomeDir = getenv("HOME");
			if (HomeDir == NULL)  HomeDir = getpwuid(getuid())->pw_dir;
			if (HomeDir == NULL)  return false;

			if (z2 < strlen(HomeDir))  return false;

			strcpy(Buffer, HomeDir);
			size_t z = strlen(HomeDir);

			return GetNIXStorageDir(Buffer, BufferSize, ".", AppDir, z, z2);
		}

		bool AppInfo::GetTempStorageDir(char *Buffer, size_t &BufferSize)
		{
			size_t z2 = BufferSize;
			BufferSize = 0;
			if (z2)  Buffer[0] = '\0';

			if (z2 < 5)  return false;

			strcpy(Buffer, "/tmp/");
			size_t z = 5;

			return GetNIXStorageDir(Buffer, BufferSize, NULL, NULL, z, z2);
		}

		// Internal function to standardize a directory on *NIX.
		bool AppInfo::GetNIXStorageDir(char *Buffer, size_t &BufferSize, const char *AppDirPrefix, const char *AppDir, size_t z, size_t z2)
		{
			if (z < z2 && Buffer[z - 1] != '/')  Buffer[z++] = '/';
			if (AppDir != NULL)
			{
				if (AppDirPrefix != NULL)
				{
					while (z < z2 && *AppDirPrefix)  Buffer[z++] = *AppDirPrefix++;
				}

				while (z < z2 && *AppDir)  Buffer[z++] = *AppDir++;

				if (z < z2 && Buffer[z - 1] != '/')  Buffer[z++] = '/';
			}
			if (z < z2)  Buffer[z++] = '\0';
			if (z == z2)  return false;
			BufferSize = z;

			return true;
		}

#endif

		bool AppInfo::GetExecutablePath(char *Buffer, size_t &BufferSize, const char *Argv0)
		{
			if (!GetExecutableFilename(Buffer, BufferSize, Argv0))  return false;

			while (BufferSize && Buffer[BufferSize - 1] != '/' && Buffer[BufferSize - 1] != '\\')  BufferSize--;
			Buffer[BufferSize++] = '\0';

			return true;
		}
	}
}
