// Cross-platform UTF-8, large file (> 2GB) and directory manipulation classes.
// (C) 2016 CubicleSoft.  All Rights Reserved.

#ifndef CUBICLESOFT_UTF8_FILE
#define CUBICLESOFT_UTF8_FILE

#ifndef _LARGEFILE64_SOURCE
	#define _LARGEFILE64_SOURCE
#endif

#include "utf8_util.h"

#include <cstdio>
#include <cstring>

#if defined(_WIN32) || defined(WIN32) || defined(_WIN64) || defined(WIN64)
	#include <sys/types.h>
	#include <sys/stat.h>
	#include <windows.h>
#else
	#include <sys/types.h>
	#include <sys/stat.h>
	#include <sys/file.h>
	#include <sys/time.h>
	#include <fcntl.h>
	#include <unistd.h>
	#include <pwd.h>
	#include <errno.h>
	#include <utime.h>
	#include <dirent.h>

	#ifdef __APPLE__
		#define lseek64 lseek
		#define open64 open
		#define off64_t off_t
	#endif
#endif

#ifndef O_RDONLY
	#define O_RDONLY      00
#endif
#ifndef O_WRONLY
	#define O_WRONLY      01
#endif
#ifndef O_RDWR
	#define O_RDWR        02
#endif
#ifndef O_ACCMODE
	#define O_ACCMODE     (O_RDONLY | O_WRONLY | O_RDWR)
#endif

#ifndef O_CREAT
	#define O_CREAT       0100
#endif
#ifndef O_EXCL
	#define O_EXCL        0200
#endif
#ifndef O_TRUNC
	#define O_TRUNC       01000
#endif
#ifndef O_APPEND
	#define O_APPEND      02000
#endif

// O_UNSAFE allows for devices to be opened for raw reading/writing.
#if defined(_WIN32) || defined(WIN32) || defined(_WIN64) || defined(WIN64)
	#ifndef O_UNSAFE
		#define O_UNSAFE      04000
	#endif
#endif

#ifndef _S_IFLNK
	#ifdef S_IFLNK
		#define _S_IFLNK   S_IFLNK
	#else
		#define _S_IFLNK   0120000
	#endif
#endif

#ifndef S_ISLNK
	#define S_ISLNK(mode)   (((mode) & S_IFMT) == _S_IFLNK)
#endif


namespace CubicleSoft
{
	namespace UTF8
	{
		class File
		{
		public:
			enum SeekType
			{
				SeekStart,
				SeekEnd,
				SeekForward,
				SeekBackward
			};

			enum ShareType
			{
				ShareBoth,
				ShareRead,
				ShareWrite,
				ShareNone
			};

			struct FilenameInfo
			{
				size_t StartVolume, StartPath, StartFilename, StartExtension, StartLastExtension, Length;
			};

#if defined(_WIN32) || defined(WIN32) || defined(_WIN64) || defined(WIN64)
			typedef struct __stat64 FileStat;
#else
			typedef struct stat FileStat;
#endif

			File();
			virtual ~File();

			virtual bool IsOpen() const;
			bool Open(const char *Filename, int Flags, ShareType ShareFlag = ShareBoth, int Mode = 0666);
			virtual bool Seek(SeekType whence, std::uint64_t Pos);
			virtual bool Read(std::uint8_t *Data, size_t DataSize, size_t &DataRead);

			// Result is not UTF-8 and also not intended for binary data.  '\0' replaced with ' ' (space).
			// Result will have been terminated by EOF or an ASCII newline '\r', '\n', '\r\n' but trimmed.
			// SizeHint helps optimize memory allocation performance.
			// Check out Sync::TLS for a high-performance allocator.
			virtual char *LineInput(size_t SizeHint = 1024, void *AltMallocManager = NULL, void *(*AltRealloc)(void *, void *, size_t) = NULL);

			virtual bool Write(const char *Data, size_t &DataWritten);
			virtual bool Write(const std::uint8_t *Data, size_t DataSize, size_t &DataWritten);
			virtual bool Flush();
			inline std::uint64_t GetCurrPos() const { return MxCurrPos; }
			inline std::uint64_t GetMaxPos() const { return MxMaxPos; }
			virtual bool UpdateMaxPos();
			virtual bool Close();

			// Some static functions specifically for files.
			static bool GetPlatformFilename(char *Result, size_t ResultSize, const char *Filename, bool AllowUnsafe = false);
			static bool IsValidFilenameFormat(const char *Filename);
			static void GetPlatformFilenameInfo(FilenameInfo &Result, const char *Filename);
			static bool GetAbsoluteFilename(char *Result, size_t ResultSize, const char *BaseDir, const char *Filename, bool TrailingSlash = false);
			static bool Exists(const char *Filename);
			static bool Realpath(char *Result, size_t ResultSize, const char *Filename);
			static bool Chmod(const char *Filename, int Mode);
			static bool Chown(const char *Filename, const char *Owner);
			static bool Chgrp(const char *Filename, const char *Group);
			static bool Copy(const char *SrcFilename, const char *DestFilename);
			static bool Move(const char *SrcFilename, const char *DestFilename);
			static bool Delete(const char *Filename);
			static bool Stat(FileStat &Result, const char *Filename, bool Link = false);
			static bool Symlink(const char *Src, const char *Dest);
			static bool Readlink(char *Result, size_t ResultSize, const char *Filename);

			// For quickly loading relatively small files.
			// Check out Sync::TLS for a high-performance allocator.
			static bool LoadEntireFile(const char *Filename, char *&Result, size_t &BytesRead, void *AltMallocManager = NULL, void *(*AltMalloc)(void *, size_t) = NULL, void (*AltFree)(void *, void *) = NULL);

			// Timestamps are in Unix microsecond format.
			static bool SetFileTimes(const char *Filename, std::uint64_t *Creation, std::uint64_t *LastAccess, std::uint64_t *LastUpdate);

		private:
			// Deny copy construction and assignment.  Use a (smart) pointer instead.
			File(const File &);
			File &operator=(const File &);

#if defined(_WIN32) || defined(WIN32) || defined(_WIN64) || defined(WIN64)
public:
			static bool GetWindowsPlatformFilename(LPWSTR Result, size_t ResultSize, const char *Filename, bool AllowUnsafe = false);
			static bool GetWindowsFilenameInfo(BY_HANDLE_FILE_INFORMATION *Result, LPWSTR Filename);
private:
			static FILETIME ConvertUnixMicrosecondTimeToFILETIME(std::uint64_t TempTime);
			static __time64_t ConvertFILETIMEToUnixTime(FILETIME TempTime);
			typedef BOOL (APIENTRY *CreateSymbolicLinkWFunc)(LPWSTR, LPWSTR, DWORD);
			static bool SetThreadProcessPrivilege(LPCWSTR PrivilegeName, bool Enable);

			HANDLE MxFile;
#else
			int MxFile;
			bool MxLocked;
#endif

		protected:
			bool MxRead, MxWrite;
			std::uint64_t MxCurrPos, MxMaxPos;
		};

		class Dir
		{
		public:
			Dir();
			~Dir();

			bool Open(const char *Dirname);
			bool Read(char *Filename, size_t Size);
			bool Close();

			// Some static functions specifically for directories.  Some functions are possibly not thread-safe.
			static bool Getcwd(char *Result, size_t ResultSize);

			// Unlike most directory/file calls, UTF8::Dir::Mkdir() is more difficult to use because Dirname must reference an
			// absolute path but UTF8::File::Realpath() can't be used on the complete calculation since one or more directories
			// likely don't exist (yet).  Use UTF8::File::Realpath() to transform the known existing part of the path and then
			// carefully concatenate the new path item(s) and call UTF8::Dir::Mkdir().
			static bool Mkdir(const char *Dirname, int Mode = 0777, bool Recursive = false);

			static bool Rmdir(const char *Dirname, bool Recursive = false);

		private:
			// Deny copy construction and assignment.  Use a (smart) pointer instead.
			Dir(const Dir &);
			Dir &operator=(const Dir &);

#if defined(_WIN32) || defined(WIN32) || defined(_WIN64) || defined(WIN64)
			static void Rmdir_RecurseInternal(WCHAR *Dirname, size_t Size);

			HANDLE MxDir;
			WIN32_FIND_DATAW MxFindData;
#else
			static void Rmdir_RecurseInternal(char *Dirname, size_t Size);

			DIR *MxDir;
#endif
		};
	}
}

#endif
