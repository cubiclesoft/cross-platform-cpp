// Cross-platform Unicode application information.
// (C) 2016 CubicleSoft.  All Rights Reserved.

#ifndef CUBICLESOFT_UTF8_APPINFO
#define CUBICLESOFT_UTF8_APPINFO

#include "utf8_util.h"

#if defined(_WIN32) || defined(WIN32) || defined(_WIN64) || defined(WIN64)
	#include <windows.h>
#else
	#include <sys/types.h>
	#include <unistd.h>
#endif

namespace CubicleSoft
{
	namespace UTF8
	{
		class AppInfo
		{
		public:
			// Retrieves the current executable's full path and filename.
			// Argv0 may be NULL and is only used on some platforms.
			static bool GetExecutableFilename(char *Buffer, size_t &BufferSize, const char *Argv0 = NULL);

			// Retrieves the current executable's full path without filename.
			// Argv0 may be NULL and is only used on some platforms.
			static bool GetExecutablePath(char *Buffer, size_t &BufferSize, const char *Argv0 = NULL);

			// Retrieves the appropriate system-level storage directory for the application to store data (e.g. All Users\AppData\AppDir\, /var/lib/AppDir/).
			// AppDir, if not NULL, is appended.
			static bool GetSystemAppStorageDir(char *Buffer, size_t &BufferSize, const char *AppDir = NULL);

			// Retrieves the current user's application data storage directory (e.g. AppData\Local\AppDir/, /home/user/.AppDir/).
			// AppDir, if not NULL, is appended.
			static bool GetCurrentUserAppStorageDir(char *Buffer, size_t &BufferSize, const char *AppDir = NULL);

			// Retrieves the temporary file storage directory (e.g. C:\TEMP\, /tmp/).
			static bool GetTempStorageDir(char *Buffer, size_t &BufferSize);

		private:
#if defined(_WIN32) || defined(WIN32) || defined(_WIN64) || defined(WIN64)
			static bool GetWindowsUTF8StorageDir(WCHAR *Dirname, char *Buffer, size_t &BufferSize, const char *AppDir, size_t z, size_t z2);
#else
			static bool GetNIXStorageDir(char *Buffer, size_t &BufferSize, const char *AppDirPrefix, const char *AppDir, size_t z, size_t z2);
#endif
		};
	}
}

#endif
