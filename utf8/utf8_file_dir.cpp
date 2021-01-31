// Cross-platform UTF-8, large file (> 2GB) and directory manipulation classes.
// (C) 2016 CubicleSoft.  All Rights Reserved.

#define _CRT_SECURE_NO_WARNINGS

#include "utf8_file_dir.h"

namespace CubicleSoft
{
	namespace UTF8
	{
#if defined(_WIN32) || defined(WIN32) || defined(_WIN64) || defined(WIN64)
		// Apparently this structure is only included in the Windows DDK.
		typedef struct _REPARSE_DATA_BUFFER {
			ULONG  ReparseTag;
			USHORT ReparseDataLength;
			USHORT Reserved;
			union {
				struct {
					USHORT SubstituteNameOffset;
					USHORT SubstituteNameLength;
					USHORT PrintNameOffset;
					USHORT PrintNameLength;
					ULONG  Flags;
					WCHAR  PathBuffer[1];
				} SymbolicLinkReparseBuffer;
				struct {
					USHORT SubstituteNameOffset;
					USHORT SubstituteNameLength;
					USHORT PrintNameOffset;
					USHORT PrintNameLength;
					WCHAR  PathBuffer[1];
				} MountPointReparseBuffer;
				struct {
					UCHAR DataBuffer[1];
				} GenericReparseBuffer;
			};
		} REPARSE_DATA_BUFFER, *PREPARSE_DATA_BUFFER;

#ifndef SYMBOLIC_LINK_FLAG_DIRECTORY
	#define SYMBOLIC_LINK_FLAG_DIRECTORY   0x1
#endif

		File::File() : MxFile(NULL), MxRead(false), MxWrite(false), MxCurrPos(0), MxMaxPos(0)
		{
		}

		File::~File()
		{
			Close();
		}

		bool File::IsOpen() const
		{
			return (MxFile != NULL);
		}

		bool File::Open(const char *Filename, int Flags, ShareType ShareFlag, int)
		{
			Close();

			DWORD WinAccess, WinShareMode, WinCreation;

			int Access = Flags & O_ACCMODE;

			if (Access == O_RDONLY)
			{
				WinAccess = GENERIC_READ;
				MxRead = true;
				MxWrite = false;
			}
			else if (Access == O_WRONLY)
			{
				WinAccess = GENERIC_WRITE;
				MxRead = false;
				MxWrite = true;
			}
			else if (Access == O_RDWR)
			{
				WinAccess = GENERIC_READ | GENERIC_WRITE;
				MxRead = true;
				MxWrite = true;
			}
			else
			{
				return false;
			}

			if (ShareFlag == ShareNone)  WinShareMode = 0;
			else if (ShareFlag == ShareRead)  WinShareMode = FILE_SHARE_READ;
			else if (ShareFlag == ShareWrite)  WinShareMode = FILE_SHARE_WRITE;
			else if (ShareFlag == ShareBoth)  WinShareMode = FILE_SHARE_READ | FILE_SHARE_WRITE;
			else  return false;

			if (Flags & O_CREAT)  WinCreation = (Flags & O_EXCL ? CREATE_NEW : OPEN_ALWAYS);
			else  WinCreation = OPEN_EXISTING;

			// Verify and retrieve a UTF-16 version of the filename.
			WCHAR Filename2[8192];
			if (!GetWindowsPlatformFilename(Filename2, 8192, Filename, (Flags & O_UNSAFE) != 0))  return false;

			// Open the file.
			SECURITY_ATTRIBUTES SecAttr;
			SecAttr.nLength = sizeof(SecAttr);
			SecAttr.lpSecurityDescriptor = NULL;
			SecAttr.bInheritHandle = TRUE;

			MxFile = ::CreateFileW(Filename2, WinAccess, WinShareMode | FILE_SHARE_DELETE, &SecAttr, WinCreation, FILE_ATTRIBUTE_NORMAL, NULL);
			if (MxFile == INVALID_HANDLE_VALUE)  MxFile = NULL;

			// Deal with post-open initialization.
			if (MxFile != NULL)
			{
				if (Flags & O_TRUNC)
				{
					::SetEndOfFile(MxFile);
					MxCurrPos = 0;
					MxMaxPos = 0;
				}
				else if (Flags & O_APPEND)
				{
					UpdateMaxPos();
					Seek(File::SeekEnd, 0);
				}
				else
				{
					MxCurrPos = 0;
					UpdateMaxPos();
				}
			}

			return (MxFile != NULL);
		}

		bool File::Seek(SeekType whence, std::uint64_t Pos)
		{
			if (MxFile == NULL)  return false;

			std::uint64_t TempPos;

			switch (whence)
			{
				case File::SeekStart:  TempPos = (Pos <= MxMaxPos ? Pos : MxMaxPos);  break;
				case File::SeekEnd:  TempPos = (Pos <= MxMaxPos ? MxMaxPos - Pos : 0);  break;
				case File::SeekForward:  TempPos = (MxCurrPos + Pos <= MxMaxPos ? MxCurrPos + Pos : MxMaxPos);  break;
				case File::SeekBackward:  TempPos = (MxCurrPos >= Pos ? MxCurrPos - Pos : 0);  break;
				default:  TempPos = MxCurrPos;  break;
			}

			LONG LowWord, HighWord;
			LowWord = (LONG)(std::uint32_t)TempPos;
			HighWord = (LONG)(std::uint32_t)(TempPos >> 32);
			DWORD Result = ::SetFilePointer(MxFile, LowWord, &HighWord, FILE_BEGIN);
			if (Result == INVALID_SET_FILE_POINTER && ::GetLastError() != NO_ERROR)  return false;

			MxCurrPos = TempPos;

			return true;
		}

		bool File::Read(std::uint8_t *Data, size_t DataSize, size_t &DataRead)
		{
			if (MxFile == NULL || !MxRead)  return false;

			if ((std::uint64_t)DataSize > MxMaxPos - MxCurrPos)
			{
				UpdateMaxPos();
				if ((std::uint64_t)DataSize > MxMaxPos - MxCurrPos)  DataSize = (size_t)(MxMaxPos - MxCurrPos);
			}

			DataRead = 0;
			if (DataSize == 0)  return true;
			DWORD BytesRead = 0;
			if (!::ReadFile(MxFile, Data, (DWORD)DataSize, &BytesRead, NULL))  return false;
			DataRead = (size_t)BytesRead;
			MxCurrPos += (std::uint64_t)BytesRead;

			return true;
		}

		bool File::Write(const std::uint8_t *Data, size_t DataSize, size_t &DataWritten)
		{
			if (MxFile == NULL || !MxWrite)  return false;

			DataWritten = 0;
			if (DataSize == 0)  return true;
			DWORD BytesWritten = 0;
			if (!::WriteFile(MxFile, Data, (DWORD)DataSize, &BytesWritten, NULL))  return false;
			DataWritten = (size_t)BytesWritten;
			MxCurrPos += (std::uint64_t)BytesWritten;
			if (MxCurrPos > MxMaxPos)  MxMaxPos = MxCurrPos;

			return true;
		}

		bool File::Flush()
		{
			if (MxFile == NULL || !MxWrite)  return false;

			if (!::FlushFileBuffers(MxFile))  return false;

			return true;
		}

		bool File::UpdateMaxPos()
		{
			if (MxFile == NULL)  return false;

			DWORD LowWord, HighWord;
			LowWord = ::GetFileSize(MxFile, &HighWord);
			if (LowWord == INVALID_FILE_SIZE && ::GetLastError() != NO_ERROR)  MxMaxPos = 0;
			else  MxMaxPos = (((std::uint64_t)HighWord) << 32) | ((std::uint64_t)LowWord);

			return true;
		}

		bool File::Close()
		{
			if (MxFile != NULL)
			{
				::CloseHandle(MxFile);
				MxFile = NULL;
			}

			return true;
		}

		bool File::GetPlatformFilename(char *Result, size_t ResultSize, const char *Filename, bool AllowUnsafe)
		{
			// Check for invalid characters and strings (restricted device names) in the path/filename.  Replace '/' with '\'.
			char TempCP;
			size_t x, y = strlen(Filename), OrigResultSize = ResultSize, y2;
			if (y + 1 > ResultSize)  return false;
			Util::ConvertToUTF8(Filename, y + 1, sizeof(char), (std::uint8_t *)Result, ResultSize);
			if (!strlen(Result) || strcmp(Result, Filename))  return false;

			// Ignore combining code points since kernels are generally not fully UTF-8 aware.
			for (x = 0; x < y; x++)
			{
				if (Result[x] == '/')  Result[x] = '\\';
			}

			// Generally disallow direct device access for security reasons.
			if (!strncmp(Result, "\\??\\", 4))  Result[1] = '\\';
			if (!_strnicmp(Result, "\\\\?\\Device\\", 11) || !strncmp(Result, "\\\\.\\", 4))  return AllowUnsafe;

			bool UNCPath = false, UnicodePath = false, DriveLetter = false;
			if (!_strnicmp(Result, "\\\\?\\UNC\\", 8))
			{
				x = 8;
				UNCPath = true;
				UnicodePath = true;
			}
			else if (!strncmp(Result, "\\\\?\\", 4))
			{
				x = 4;
				UnicodePath = true;
			}
			else
			{
				x = 0;
			}

			if (Result[x] >= 'a' && Result[x] <= 'z' && Result[x + 1] == ':')  Result[x] = Result[x] - 'a' + 'A';
			if (Result[x] >= 'A' && Result[x] <= 'Z' && Result[x + 1] == ':')
			{
				x += 2;
				DriveLetter = true;
			}
			else if (!strncmp(Result, "Volume{", 7))  DriveLetter = true;

			size_t LastPos = 0;
			for (; x < y; x++)
			{
				TempCP = Result[x];

				if (TempCP < 0x20 || TempCP == 0x7F || TempCP == ':' || TempCP == '*' || TempCP == '?' || TempCP == '\"' || TempCP == '<' || TempCP == '>' || TempCP == '|')  return false;
				else if (TempCP == '.' || TempCP == '\\')
				{
					y2 = x - LastPos;
					if (y2 == 3 && (!_strnicmp(Result + LastPos, "NUL", 3) || !_strnicmp(Result + LastPos, "CON", 3) || !_strnicmp(Result + LastPos, "AUX", 3) || !_strnicmp(Result + LastPos, "PRN", 3)))  return false;
					else if (y2 == 6 && !_strnicmp(Result + LastPos, "CLOCK$", 6))  return false;
					else if (y2 == 4 && (!_strnicmp(Result + LastPos, "LPT", 3) || !_strnicmp(Result + LastPos, "COM", 3)) && Result[x - 1] >= '0' && Result[x - 1] <= '9')  return false;

					if (TempCP == '\\')  LastPos = x + 1;
				}
			}

			y2 = y - LastPos;
			if (y2 == 3 && (!_strnicmp(Result + LastPos, "NUL", 3) || !_strnicmp(Result + LastPos, "CON", 3) || !_strnicmp(Result + LastPos, "AUX", 3) || !_strnicmp(Result + LastPos, "PRN", 3)))  return false;
			else if (y2 == 6 && !_strnicmp(Result + LastPos, "CLOCK$", 6))  return false;
			else if (y2 == 4 && (!_strnicmp(Result + LastPos, "LPT", 3) || !_strnicmp(Result + LastPos, "COM", 3)) && Result[y - 1] >= '0' && Result[y - 1] <= '9')  return false;

			// Normalize absolute filenames so they have a '\\?\' prefix.
			if (DriveLetter)
			{
				if (!UnicodePath)
				{
					// Drive letter found without Unicode prefix.
					if (y + 4 > OrigResultSize)  return false;
					memmove(Result + 4, Result, y + 1);
					memcpy(Result, "\\\\?\\", 4);
				}

				// Attempt to resolve \\?\Volume{GUID}\ paths.
				char *Result3 = strchr(Result + 4, '\\');
				if (!strncmp(Result, "\\\\?\\Volume{", 11) && Result3 != NULL)
				{
					Result[1] = '?';

					HKEY RegKey;
					if (::RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SYSTEM\\MountedDevices", 0, KEY_READ, &RegKey) == ERROR_SUCCESS)
					{
						BYTE Buffer[1024];
						DWORD BufferSize = 1024;

						if (::RegQueryValueExA(RegKey, Result, NULL, NULL, Buffer, &BufferSize) == ERROR_SUCCESS)
						{
							// Found a match for \??\Volume{GUID}.
							char ValueName[100];
							BYTE Buffer2[1024];
							DWORD KeyIndex = 0, ValueNameSize = 100, BufferSize2 = 1024;

							while (::RegEnumValueA(RegKey, KeyIndex++, ValueName, &ValueNameSize, NULL, NULL, Buffer2, &BufferSize2) == ERROR_SUCCESS)
							{
								if (BufferSize == BufferSize2 && !strncmp(ValueName, "\\DosDevice\\", 11) && !memcmp(Buffer, Buffer2, BufferSize))
								{
									// Found a match.  Resolve the volume to the drive letter.
									Result[4] = ValueName[11];
									Result[5] = ':';
									memmove(Result + 6, Result3, y + 1 - (Result3 - Result));

									break;
								}

								ValueNameSize = 100;
								BufferSize2 = 1024;
							}
						}

						::RegCloseKey(RegKey);
					}

					Result[1] = '\\';
				}
			}
			else if (!UNCPath)
			{
				if (UnicodePath)
				{
					// Unicode network path found without UNC prefix.
					if (y + 4 > OrigResultSize)  return false;
					memmove(Result + 8, Result + 4, y + 1 - 4);
					memcpy(Result + 4, "UNC\\", 4);
				}
				else if (Result[0] == '\\' && Result[1] == '\\')
				{
					// Network path found without Unicode and UNC prefixes.
					if (y + 6 > OrigResultSize)  return false;
					memmove(Result + 7, Result + 1, y + 1 - 1);
					memcpy(Result + 1, "\\?\\UNC", 6);
				}
			}

			return true;
		}

		// Expects Filename to be a string returned from GetPlatformFilename().
		void File::GetPlatformFilenameInfo(FilenameInfo &Result, const char *Filename)
		{
			size_t x, y = strlen(Filename);

			Result.Length = y;

			if (!_strnicmp(Filename, "\\\\?\\UNC\\", 8))  Result.StartVolume = 8;
			else if (!strncmp(Filename, "\\\\?\\", 4))  Result.StartVolume = 4;
			else  Result.StartVolume = 0;

			if (!Result.StartVolume)  Result.StartPath = 0;
			else
			{
				for (x = Result.StartVolume; x < y && Filename[x] != '\\'; x++);
				Result.StartPath = x;
			}

			for (x = y; x > Result.StartPath && Filename[x - 1] != '\\'; x--);
			Result.StartFilename = x;

			for (; x < y && Filename[x] != '.'; x++);
			Result.StartLastExtension = Result.StartExtension = x;

			for (; x < y; x++)
			{
				if (Filename[x] == '.')  Result.StartLastExtension = x;
			}
		}

		bool File::GetAbsoluteFilename(char *Result, size_t ResultSize, const char *BaseDir, const char *Filename, bool TrailingSlash)
		{
			char Filename2[8192], Filename3[8192];

			if (!GetPlatformFilename(Filename3, 8192, Filename))  return false;

			// Replace '\.\' with '\'.
			size_t x, x2, y = strlen(Filename3);
			x2 = 0;
			for (x = 0; x < y; x++)
			{
				while (Filename3[x] == '\\' && Filename3[x + 1] == '.' && Filename3[x + 2] == '\\')  x += 2;

				Filename3[x2++] = Filename3[x];

				if (Filename3[x] == '\\' && Filename3[x + 1] == '.' && Filename3[x + 2] == '\0')  break;
			}
			if (x2 >= 3 && Filename3[x2 - 3] == '\\' && Filename3[x2 - 2] == '.' && Filename3[x2 - 1] == '.')
			{
				if (x2 >= 8191)  return false;
				Filename3[x2++] = '\\';
			}
			Filename3[x2] = '\0';

			// Get some parsed information about the input data.
			FilenameInfo FileInfo2, FileInfo3;
			GetPlatformFilenameInfo(FileInfo3, Filename3);

			// Copy the absolute volume.
			if (FileInfo3.StartVolume)
			{
				memcpy(Filename2, Filename3, FileInfo3.StartPath);
				Filename2[FileInfo3.StartPath] = '\0';
			}
			else
			{
				if (BaseDir != NULL)
				{
					if (!GetPlatformFilename(Filename2, 8192, BaseDir))  return false;
				}
				else
				{
					if (!Dir::Getcwd(Filename2, 8192))  return false;
				}

				// Require the base directory to be absolute.
				if (strncmp(Filename2, "\\\\?\\", 4))  return false;
			}

			y = strlen(Filename2);
			if (y && Filename2[y - 1] != '\\')
			{
				if (y >= 8191)  return false;
				Filename2[y++] = '\\';
				Filename2[y] = '\0';
			}

			GetPlatformFilenameInfo(FileInfo2, Filename2);
			if (Filename3[FileInfo3.StartPath] == '\\')  FileInfo2.Length = FileInfo2.StartPath;

			// Make sure the string always starts with a path separator.
			if (FileInfo2.Length >= 8191)  return false;
			if (FileInfo2.StartPath == FileInfo2.Length)  Filename2[FileInfo2.Length++] = '\\';

			// Parse the filename into the base directory.
			for (x = FileInfo3.StartPath; x < FileInfo3.Length; x++)
			{
				while (Filename3[x] == '\\' && Filename3[x + 1] == '.' && Filename3[x + 2] == '.' && Filename3[x + 3] == '\\')
				{
					if (FileInfo2.Length > FileInfo2.StartPath)
					{
						for (; FileInfo2.Length > FileInfo2.StartPath && Filename2[FileInfo2.Length - 1] == '\\'; FileInfo2.Length--);
						for (; FileInfo2.Length > FileInfo2.StartPath && Filename2[FileInfo2.Length - 1] != '\\'; FileInfo2.Length--);
					}

					x += 3;
				}

				if (FileInfo2.Length >= 8191)  return false;
				Filename2[FileInfo2.Length++] = Filename3[x];
			}

			// Copy the volume.
			for (x = 0; x < ResultSize && x < FileInfo2.StartPath; x++)  Result[x] = Filename2[x];

			// Copy the rest.
			x2 = x;
			for (; x2 < ResultSize && x < FileInfo2.Length; x++)
			{
				while (Filename2[x] == '\\' && Filename2[x + 1] == '\\')  x++;

				Result[x2++] = Filename2[x];
			}
			if (TrailingSlash && x2 && Result[x2 - 1] != '\\' && x2 < ResultSize)  Result[x2++] = '\\';
			if (!TrailingSlash && x2 && Result[x2 - 1] == '\\')  x2--;
			if (x2 >= ResultSize)  return false;
			Result[x2] = '\0';

			// Sanity check.
			for (x = 0; x < x2; x++)
			{
				if (Result[x] == '\\' && Result[x + 1] == '.')
				{
					if (Result[x + 2] == '\\')  return false;
					if (Result[x + 2] == '.' && Result[x + 3] == '\\')  return false;
				}
			}

			return true;
		}

		bool File::GetWindowsPlatformFilename(LPWSTR Result, size_t ResultSize, const char *Filename, bool AllowUnsafe)
		{
			char Filename2[8192];

			if (!GetPlatformFilename(Filename2, 8192, Filename, AllowUnsafe))  return false;

			Util::ConvertFromUTF8((std::uint8_t *)Filename2, strlen(Filename2) + 1, Result, ResultSize, sizeof(WCHAR));

			return true;
		}

		bool File::GetWindowsFilenameInfo(BY_HANDLE_FILE_INFORMATION *Result, LPWSTR Filename)
		{
			SECURITY_ATTRIBUTES SecAttr;
			SecAttr.nLength = sizeof(SecAttr);
			SecAttr.lpSecurityDescriptor = NULL;
			SecAttr.bInheritHandle = TRUE;

			HANDLE TempFile = ::CreateFileW(Filename, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, &SecAttr, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
			if (TempFile == INVALID_HANDLE_VALUE)  return false;

			// Retrieve the information about the file.
			bool Result2 = (::GetFileInformationByHandle(TempFile, Result) != 0);

			::CloseHandle(TempFile);

			return Result2;
		}

		bool File::Exists(const char *Filename)
		{
			WCHAR Filename2[8192];
			if (!GetWindowsPlatformFilename(Filename2, 8192, Filename))  return false;

			DWORD TempAttrs;
			if ((TempAttrs = ::GetFileAttributesW(Filename2)) == (DWORD)-1)  return false;

			return true;
		}

		// Only pass in absolute filenames.
		bool File::Realpath(char *Result, size_t ResultSize, const char *Filename)
		{
			char Filename2[8192], Filename3[8192];

			// Normalize input.
			if (!File::GetPlatformFilename(Filename2, 8192, Filename))  return false;

			// Require the filename to be absolute.
			if (strncmp(Filename2, "\\\\?\\", 4))  return false;

			// Get some parsed information about the input data.
			FilenameInfo FileInfo;
			GetPlatformFilenameInfo(FileInfo, Filename2);

			// Copy the prefix and volume.
			memcpy(Filename3, Filename2, FileInfo.StartPath);

			size_t x, x2 = FileInfo.StartPath, x3, NumLinks = 0;
			HANDLE FindHandle;
			WCHAR FindFilename[8192];
			WIN32_FIND_DATAW FindData;
			bool Processed;
			while (FileInfo.StartPath < FileInfo.Length)
			{
				// Append the next chunk.
				if (x2 >= 8191)  return false;
				Filename3[x2++] = '\\';
				x = x2;
				for (; FileInfo.StartPath < FileInfo.Length && Filename2[FileInfo.StartPath] == '\\'; FileInfo.StartPath++);
				for (; FileInfo.StartPath < FileInfo.Length && Filename2[FileInfo.StartPath] != '\\'; FileInfo.StartPath++)
				{
					if (x2 >= 8191)  return false;
					Filename3[x2++] = Filename2[FileInfo.StartPath];
				}
				Filename3[x2] = '\0';

				// If possible, convert the chunk to a long filename.
				x3 = 8192;
				Util::ConvertFromUTF8((std::uint8_t *)Filename3, x2 + 1, FindFilename, x3, sizeof(WCHAR));
				FindHandle = ::FindFirstFileW(FindFilename, &FindData);
				if (FindHandle != INVALID_HANDLE_VALUE)
				{
					::FindClose(FindHandle);

					Processed = false;
					if ((FindData.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) && (FindData.dwReserved0 == IO_REPARSE_TAG_SYMLINK || FindData.dwReserved0 == IO_REPARSE_TAG_MOUNT_POINT))
					{
						NumLinks++;
						if (NumLinks > 20)  return false;

						// Process the symbolic link/mount point.
						SECURITY_ATTRIBUTES SecAttr;
						SecAttr.nLength = sizeof(SecAttr);
						SecAttr.lpSecurityDescriptor = NULL;
						SecAttr.bInheritHandle = TRUE;

						FindHandle = ::CreateFileW(FindFilename, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, &SecAttr, OPEN_EXISTING, FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS, NULL);
						if (FindHandle != INVALID_HANDLE_VALUE)
						{
							char TempReparseBuffer[MAXIMUM_REPARSE_DATA_BUFFER_SIZE] = {0};
							REPARSE_DATA_BUFFER *TempReparse = (REPARSE_DATA_BUFFER *)TempReparseBuffer;
							DWORD ReparseSize;

							if (::DeviceIoControl(FindHandle, FSCTL_GET_REPARSE_POINT, NULL, 0, TempReparse, MAXIMUM_REPARSE_DATA_BUFFER_SIZE, &ReparseSize, NULL))
							{
								// A symbolic link or mount point has been successfully accessed.
								char Filename4[8192];
								x3 = 8192;

								if (FindData.dwReserved0 == IO_REPARSE_TAG_SYMLINK)  Util::ConvertToUTF8(TempReparse->SymbolicLinkReparseBuffer.PathBuffer + TempReparse->SymbolicLinkReparseBuffer.SubstituteNameOffset, TempReparse->SymbolicLinkReparseBuffer.SubstituteNameLength, sizeof(WCHAR), (std::uint8_t *)Filename4, x3);
								else  Util::ConvertToUTF8(TempReparse->MountPointReparseBuffer.PathBuffer + TempReparse->MountPointReparseBuffer.SubstituteNameOffset, TempReparse->MountPointReparseBuffer.SubstituteNameLength, sizeof(WCHAR), (std::uint8_t *)Filename4, x3);

								// Create a new absolute source path.
								Filename2[FileInfo.StartPath] = '\0';
								if (!GetAbsoluteFilename(Filename3, 8192, Filename2, Filename4, false))  return false;
								if (FileInfo.StartPath < FileInfo.Length)  Filename2[FileInfo.StartPath] = '\\';
								x = strlen(Filename3);
								if (x + FileInfo.Length - FileInfo.StartPath + 1 > 8191)  return false;
								memcpy(Filename3 + x, Filename2 + FileInfo.StartPath, FileInfo.Length - FileInfo.StartPath + 1);
								strcpy(Filename2, Filename3);

								// Reset the loop.
								// Require the filename to be absolute.
								if (strncmp(Filename2, "\\\\?\\", 4))  return false;

								// Get some parsed information about the input data.
								GetPlatformFilenameInfo(FileInfo, Filename2);

								// Copy the prefix and volume.
								memcpy(Filename3, Filename2, FileInfo.StartPath);

								x2 = FileInfo.StartPath;
								x3 = 0;

								Processed = true;
							}

							::CloseHandle(FindHandle);
						}
					}

					if (!Processed)
					{
						// Exchange the current chunk with the new one.
						x2 = 8192 - x;
						Util::ConvertToUTF8(FindData.cFileName, wcslen(FindData.cFileName), sizeof(WCHAR), (std::uint8_t *)Filename3 + x, x2);
						x2 += x;
					}
				}
			}
			Filename3[x2++] = '\0';
			if (ResultSize < x2)  return false;
			memcpy(Result, Filename3, x2);

			return Exists(Result);
		}

		bool File::Chmod(const char *Filename, int Mode)
		{
			WCHAR Filename2[8192];
			if (!GetWindowsPlatformFilename(Filename2, 8192, Filename))  return false;

			DWORD TempAttrs;

			if ((TempAttrs = ::GetFileAttributesW(Filename2)) == (DWORD)-1)  return false;
			if (TempAttrs & FILE_ATTRIBUTE_DIRECTORY)  return false;

			TempAttrs &= ~FILE_ATTRIBUTE_READONLY;
			TempAttrs &= ~FILE_ATTRIBUTE_NORMAL;
			if ((Mode & 0333) == 0 && (Mode & 0444) != 0)  TempAttrs |= FILE_ATTRIBUTE_READONLY;
			if (TempAttrs == 0)  TempAttrs |= FILE_ATTRIBUTE_NORMAL;
			if (!::SetFileAttributesW(Filename2, TempAttrs))  return false;

			return true;
		}

		// Owner and group are mostly Linux.  Windows has (D)ACLs, which are tricky to manipulate.
		bool File::Chown(const char *, const char *)
		{
			return true;
		}

		bool File::Chgrp(const char *, const char *)
		{
			return true;
		}

		bool File::Copy(const char *SrcFilename, const char *DestFilename)
		{
			WCHAR SrcFilename2[8192], DestFilename2[8192];
			if (!GetWindowsPlatformFilename(SrcFilename2, 8192, SrcFilename))  return false;
			if (!GetWindowsPlatformFilename(DestFilename2, 8192, DestFilename))  return false;

			// Prevent copying a file onto itself AND get file time information.
			BY_HANDLE_FILE_INFORMATION SrcFileInfo, DestFileInfo;
			if (!File::GetWindowsFilenameInfo(&SrcFileInfo, SrcFilename2))  return false;

			if (File::GetWindowsFilenameInfo(&DestFileInfo, DestFilename2) && SrcFileInfo.dwVolumeSerialNumber == DestFileInfo.dwVolumeSerialNumber && SrcFileInfo.nFileSizeHigh == DestFileInfo.nFileSizeHigh && SrcFileInfo.nFileSizeLow == DestFileInfo.nFileSizeLow && SrcFileInfo.nFileIndexHigh == DestFileInfo.nFileIndexHigh && SrcFileInfo.nFileIndexLow == DestFileInfo.nFileIndexLow)  return false;

			DWORD TempAttrs2 = ::GetFileAttributesW(DestFilename2);
			::SetFileAttributesW(DestFilename2, FILE_ATTRIBUTE_NORMAL);
			bool Result = (::CopyFileW(SrcFilename2, DestFilename2, FALSE) != 0);
			::SetFileAttributesW(DestFilename2, (Result ? SrcFileInfo.dwFileAttributes : (TempAttrs2 != (DWORD)-1 ? TempAttrs2 : FILE_ATTRIBUTE_NORMAL)));

			if (Result)
			{
				// Update the last modified timestamp (might have been changed by SetFileAttributesW()).
				std::uint64_t TempLastModified = (std::uint64_t)(((std::uint64_t)(((std::uint64_t)SrcFileInfo.ftLastWriteTime.dwHighDateTime) << 32 | ((std::uint64_t)SrcFileInfo.ftLastWriteTime.dwLowDateTime)) - (std::uint64_t)116444736000000000ULL) / (std::uint64_t)10);
				SetFileTimes(DestFilename, NULL, NULL, &TempLastModified);
			}

			return Result;
		}

		bool File::Move(const char *SrcFilename, const char *DestFilename)
		{
			WCHAR SrcFilename2[8192], DestFilename2[8192];
			if (!GetWindowsPlatformFilename(SrcFilename2, 8192, SrcFilename))  return false;
			if (!GetWindowsPlatformFilename(DestFilename2, 8192, DestFilename))  return false;

			bool Result = (::MoveFileW(SrcFilename2, DestFilename2) != 0);
			if (!Result)
			{
				Result = File::Copy(SrcFilename, DestFilename);
				if (Result)  ::DeleteFileW(SrcFilename2);
			}

			return Result;
		}

		bool File::Delete(const char *Filename)
		{
			WCHAR Filename2[8192];
			if (!GetWindowsPlatformFilename(Filename2, 8192, Filename))  return false;

			::SetFileAttributesW(Filename2, FILE_ATTRIBUTE_NORMAL);

			return (::DeleteFileW(Filename2) != 0);
		}

		bool File::Stat(FileStat &Result, const char *Filename, bool Link)
		{
			char Filename2[8192];
			WCHAR Filename3[8192];
			size_t TempSize = 8192;

			if (!GetPlatformFilename(Filename2, 8192, Filename))  return false;

			Util::ConvertFromUTF8((std::uint8_t *)Filename2, strlen(Filename2) + 1, Filename3, TempSize, sizeof(WCHAR));

			WIN32_FILE_ATTRIBUTE_DATA AttrData = {0};

			if (!::GetFileAttributesExW(Filename3, GetFileExInfoStandard, &AttrData))  return false;

			// Build the stat structure manually.
			// Get some parsed information about the input data for st_dev.
			FilenameInfo FileInfo;
			GetPlatformFilenameInfo(FileInfo, Filename2);

			if (FileInfo.StartPath - FileInfo.StartVolume == 2 && Filename2[FileInfo.StartVolume] >= 'A' && Filename2[FileInfo.StartVolume] <= 'Z' && Filename2[FileInfo.StartVolume + 1] == ':')
			{
				Result.st_dev = Filename2[FileInfo.StartVolume] - 'A';
			}
			else if (FileInfo.StartVolume > 0)
			{
				Result.st_dev = 0;
			}
			else
			{
				char Filename4[8192];

				if (!Dir::Getcwd(Filename4, 8192))  return false;

				FilenameInfo FileInfo2;
				GetPlatformFilenameInfo(FileInfo2, Filename4);

				if (FileInfo2.StartPath - FileInfo2.StartVolume == 2 && Filename4[FileInfo2.StartVolume] >= 'A' && Filename4[FileInfo2.StartVolume] <= 'Z' && Filename4[FileInfo2.StartVolume + 1] == ':')
				{
					Result.st_dev = Filename4[FileInfo2.StartVolume] - 'A';
				}
				else if (FileInfo2.StartVolume > 0)
				{
					Result.st_dev = 0;
				}
				else
				{
					Result.st_dev = -1;
				}
			}

			Result.st_ino = 0;
			Result.st_nlink = 1;
			bool Processed = false;
			if (Link && AttrData.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)
			{
				// Process the symbolic link/mount point.
				HANDLE TempHandle;
				SECURITY_ATTRIBUTES SecAttr;
				SecAttr.nLength = sizeof(SecAttr);
				SecAttr.lpSecurityDescriptor = NULL;
				SecAttr.bInheritHandle = TRUE;

				TempHandle = ::CreateFileW(Filename3, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, &SecAttr, OPEN_EXISTING, FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS, NULL);
				if (TempHandle != INVALID_HANDLE_VALUE)
				{
					char TempReparseBuffer[MAXIMUM_REPARSE_DATA_BUFFER_SIZE] = {0};
					REPARSE_DATA_BUFFER *TempReparse = (REPARSE_DATA_BUFFER *)TempReparseBuffer;
					DWORD ReparseSize;

					if (::DeviceIoControl(TempHandle, FSCTL_GET_REPARSE_POINT, NULL, 0, TempReparse, MAXIMUM_REPARSE_DATA_BUFFER_SIZE, &ReparseSize, NULL))
					{
						if (TempReparse->ReparseTag == IO_REPARSE_TAG_SYMLINK || TempReparse->ReparseTag == IO_REPARSE_TAG_MOUNT_POINT)
						{
							Result.st_mode = _S_IFLNK;

							Processed = true;
						}
					}

					::CloseHandle(TempHandle);
				}
			}

			if (!Processed)  Result.st_mode = (AttrData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ? (_S_IFDIR | 0111) : _S_IFREG);

			Result.st_mode |= (AttrData.dwFileAttributes & FILE_ATTRIBUTE_READONLY ? 0444 : 0666);

			if (!(AttrData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && FileInfo.Length > 4 && Filename2[FileInfo.Length - 4] == '.')
			{
				if (!_strnicmp(Filename2 + FileInfo.Length - 3, "exe", 3) || !_strnicmp(Filename2 + FileInfo.Length - 3, "com", 3) || !_strnicmp(Filename2 + FileInfo.Length - 3, "bat", 3) || !_strnicmp(Filename2 + FileInfo.Length - 3, "cmd", 3))
				{
					Result.st_mode |= 0111;
				}
			}

			Result.st_uid = 0;
			Result.st_gid = 0;
			Result.st_rdev = Result.st_dev;
			Result.st_size = (((std::uint64_t)AttrData.nFileSizeHigh) << 32) | ((std::uint64_t)AttrData.nFileSizeLow);
			Result.st_atime = ConvertFILETIMEToUnixTime(AttrData.ftLastAccessTime);
			Result.st_mtime = ConvertFILETIMEToUnixTime(AttrData.ftLastWriteTime);
			Result.st_ctime = ConvertFILETIMEToUnixTime(AttrData.ftCreationTime);

			return true;
		}

		bool File::SetThreadProcessPrivilege(LPCWSTR PrivilegeName, bool Enable)
		{
			HANDLE Token;
			TOKEN_PRIVILEGES TokenPrivs;
			LUID TempLuid;
			bool Result;

			if (!::LookupPrivilegeValueW(NULL, PrivilegeName, &TempLuid))  return false;

			if (!::OpenThreadToken(::GetCurrentThread(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, FALSE, &Token))
			{
				if (!::OpenProcessToken(::GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &Token))  return false;
			}

			TokenPrivs.PrivilegeCount = 1;
			TokenPrivs.Privileges[0].Luid = TempLuid;
			TokenPrivs.Privileges[0].Attributes = (Enable ? SE_PRIVILEGE_ENABLED : 0);

			Result = (::AdjustTokenPrivileges(Token, FALSE, &TokenPrivs, 0, NULL, NULL) && ::GetLastError() == ERROR_SUCCESS);

			::CloseHandle(Token);

			return Result;
		}

		// Unfortunately, creating a symbolic link on Windows requires the SE_CREATE_SYMBOLIC_LINK_NAME privilege.
		// The privilege is only available to elevated processes.  Run from a command-line:  whoami /priv
		bool File::Symlink(const char *Src, const char *Dest)
		{
			// Enable the privilege in the thread/process token if the user has the privilege but it is disabled.
			SetThreadProcessPrivilege(L"SeCreateSymbolicLinkPrivilege", true);

			HMODULE TempModule = ::LoadLibraryA("KERNEL32.DLL");
			if (TempModule == NULL)  return false;
			CreateSymbolicLinkWFunc CreateSymbolicLinkWPtr = (CreateSymbolicLinkWFunc)::GetProcAddress(TempModule, "CreateSymbolicLinkW");
			if (CreateSymbolicLinkWPtr == NULL)  return false;

			char Src2[8192], Dest2[8192];
			if (!GetAbsoluteFilename(Src2, 8192, NULL, Src, false))  return false;
			if (!GetAbsoluteFilename(Dest2, 8192, Src2, Dest, false))  return false;

			WCHAR Filename2[8192], Filename3[8192];
			if (!GetWindowsPlatformFilename(Filename2, 8192, Dest2))  return false;
			DWORD Flags = ::GetFileAttributesW(Filename2);
			if (Flags == (DWORD)-1)  return false;
			Flags = (Flags & FILE_ATTRIBUTE_DIRECTORY ? SYMBOLIC_LINK_FLAG_DIRECTORY : 0);

			if (!GetWindowsPlatformFilename(Filename2, 8192, Src))  return false;
			if (!GetWindowsPlatformFilename(Filename3, 8192, Dest))  return false;

			return ((*CreateSymbolicLinkWPtr)(Filename2, Filename3, Flags) != 0);
		}

		bool File::Readlink(char *Result, size_t ResultSize, const char *Filename)
		{
			WCHAR Filename2[8192];
			if (!GetWindowsPlatformFilename(Filename2, 8192, Filename))  return false;

			DWORD TempAttrs;
			if ((TempAttrs = ::GetFileAttributesW(Filename2)) == (DWORD)-1)  return false;

			bool Found = false;

			if (TempAttrs & FILE_ATTRIBUTE_REPARSE_POINT)
			{
				// Process the symbolic link/mount point.
				HANDLE TempHandle;
				SECURITY_ATTRIBUTES SecAttr;
				SecAttr.nLength = sizeof(SecAttr);
				SecAttr.lpSecurityDescriptor = NULL;
				SecAttr.bInheritHandle = TRUE;

				TempHandle = ::CreateFileW(Filename2, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, &SecAttr, OPEN_EXISTING, FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS, NULL);
				if (TempHandle != INVALID_HANDLE_VALUE)
				{
					char TempReparseBuffer[MAXIMUM_REPARSE_DATA_BUFFER_SIZE] = {0};
					REPARSE_DATA_BUFFER *TempReparse = (REPARSE_DATA_BUFFER *)TempReparseBuffer;
					DWORD ReparseSize;

					if (::DeviceIoControl(TempHandle, FSCTL_GET_REPARSE_POINT, NULL, 0, TempReparse, MAXIMUM_REPARSE_DATA_BUFFER_SIZE, &ReparseSize, NULL))
					{
						// Copy the symbolic link or mount point.
						if (TempReparse->ReparseTag == IO_REPARSE_TAG_SYMLINK || TempReparse->ReparseTag == IO_REPARSE_TAG_MOUNT_POINT)
						{
							size_t y = ResultSize;

							if (TempReparse->ReparseTag == IO_REPARSE_TAG_SYMLINK)  Util::ConvertToUTF8(TempReparse->SymbolicLinkReparseBuffer.PathBuffer + TempReparse->SymbolicLinkReparseBuffer.SubstituteNameOffset, TempReparse->SymbolicLinkReparseBuffer.SubstituteNameLength, sizeof(WCHAR), (std::uint8_t *)Result, y);
							else  Util::ConvertToUTF8(TempReparse->MountPointReparseBuffer.PathBuffer + TempReparse->MountPointReparseBuffer.SubstituteNameOffset, TempReparse->MountPointReparseBuffer.SubstituteNameLength, sizeof(WCHAR), (std::uint8_t *)Result, y);

							if (y < ResultSize)
							{
								Result[y] = '\0';

								Found = true;
							}
						}
					}

					::CloseHandle(TempHandle);
				}
			}

			return Found;
		}

		__time64_t File::ConvertFILETIMEToUnixTime(FILETIME TempTime)
		{
			__time64_t Result;
			ULARGE_INTEGER TempTime2;

			TempTime2.HighPart = TempTime.dwHighDateTime;
			TempTime2.LowPart = TempTime.dwLowDateTime;
			Result = (__time64_t)((TempTime2.QuadPart - (std::uint64_t)116444736000000000ULL) / (std::uint64_t)10000000);

			return Result;
		}

		FILETIME File::ConvertUnixMicrosecondTimeToFILETIME(std::uint64_t TempTime)
		{
			ULARGE_INTEGER TempTime2;
			FILETIME Result;

			TempTime = (TempTime * 10) + (std::uint64_t)116444736000000000ULL;

			TempTime2.QuadPart = TempTime;
			Result.dwHighDateTime = TempTime2.HighPart;
			Result.dwLowDateTime = TempTime2.LowPart;

			return Result;
		}

		bool File::SetFileTimes(const char *Filename, std::uint64_t *Creation, std::uint64_t *LastAccess, std::uint64_t *LastUpdate)
		{
			if (Creation == NULL && LastAccess == NULL && LastUpdate == NULL)  return false;

			WCHAR Filename2[8192];
			if (!GetWindowsPlatformFilename(Filename2, 8192, Filename))  return false;

			SECURITY_ATTRIBUTES SecAttr;
			SecAttr.nLength = sizeof(SecAttr);
			SecAttr.lpSecurityDescriptor = NULL;
			SecAttr.bInheritHandle = TRUE;

			HANDLE TempFile = ::CreateFileW(Filename2, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, &SecAttr, OPEN_EXISTING, 0, NULL);
			if (TempFile == INVALID_HANDLE_VALUE)  return false;

			// Prevent the OS from updating the timestamps.
			FILETIME TempTime, TempTime2, TempTime3;
			TempTime.dwHighDateTime = 0xFFFFFFFF;
			TempTime.dwLowDateTime = 0xFFFFFFFF;
			::SetFileTime(TempFile, NULL, &TempTime, &TempTime);

			// Update the desired timestamps.
			if (Creation != NULL)  TempTime = File::ConvertUnixMicrosecondTimeToFILETIME(*Creation);
			if (LastAccess != NULL)  TempTime2 = File::ConvertUnixMicrosecondTimeToFILETIME(*LastAccess);
			if (LastUpdate != NULL)  TempTime3 = File::ConvertUnixMicrosecondTimeToFILETIME(*LastUpdate);
			bool Result2 = (::SetFileTime(TempFile, (Creation != NULL ? &TempTime : NULL), (LastAccess != NULL ? &TempTime2 : NULL), (LastUpdate != NULL ? &TempTime3 : NULL)) != 0);

			::CloseHandle(TempFile);

			return Result2;
		}


		Dir::Dir() : MxDir(INVALID_HANDLE_VALUE)
		{
		}

		Dir::~Dir()
		{
			Close();
		}

		bool Dir::Open(const char *Dirname)
		{
			Close();

			char Dirname2[8192];
			size_t y;

			if (!File::GetAbsoluteFilename(Dirname2, 8192, NULL, Dirname, true))  return false;
			y = strlen(Dirname2);
			if (y >= 8191)  return false;
			Dirname2[y++] = '*';
			Dirname2[y++] = '\0';

			WCHAR Dirname3[8192];
			size_t y2 = 8192;
			Util::ConvertFromUTF8((std::uint8_t *)Dirname2, y, Dirname3, y2, sizeof(WCHAR));

			MxDir = ::FindFirstFileW(Dirname3, &MxFindData);

			return (MxDir != INVALID_HANDLE_VALUE);
		}

		bool Dir::Read(char *Filename, size_t Size)
		{
			if (MxDir == INVALID_HANDLE_VALUE)  return false;

			size_t y = Size;

			Util::ConvertToUTF8(MxFindData.cFileName, wcslen(MxFindData.cFileName) + 1, sizeof(WCHAR), (std::uint8_t *)Filename, Size);
			if (y == Size)  return false;

			if (!::FindNextFileW(MxDir, &MxFindData))  Close();

			return true;
		}

		bool Dir::Close()
		{
			if (MxDir == INVALID_HANDLE_VALUE)  return false;

			::FindClose(MxDir);
			MxDir = INVALID_HANDLE_VALUE;

			return true;
		}

		bool Dir::Getcwd(char *Result, size_t ResultSize)
		{
			WCHAR Dirname[8192];

			// This API is weird.  It isn't thread-safe and it has odd return values.
			if (!::GetCurrentDirectoryW(8192, Dirname))  return false;
			Dirname[8191] = L'\0';

			// Normalize result.
			char Dirname2[8192];
			size_t y = 8192;

			Util::ConvertToUTF8(Dirname, wcslen(Dirname) + 1, sizeof(WCHAR), (std::uint8_t *)Dirname2, y);
			if (!File::GetPlatformFilename(Result, ResultSize, Dirname2))  return false;

			return true;
		}

		bool Dir::Mkdir(const char *Dirname, int, bool Recursive)
		{
			char Dirname2[8192];
			size_t y;

			if (!File::GetAbsoluteFilename(Dirname2, 8192, NULL, Dirname, false))  return false;
			y = strlen(Dirname2) + 1;

			WCHAR Dirname3[8192];
			size_t y2 = 8192;
			Util::ConvertFromUTF8((std::uint8_t *)Dirname2, y, Dirname3, y2, sizeof(WCHAR));

			SECURITY_ATTRIBUTES SecAttr;
			SecAttr.nLength = sizeof(SecAttr);
			SecAttr.lpSecurityDescriptor = NULL;
			SecAttr.bInheritHandle = TRUE;

			if (Recursive)
			{
				size_t x;
				File::FilenameInfo FileInfo;

				File::GetPlatformFilenameInfo(FileInfo, Dirname2);

				for (x = FileInfo.StartVolume; Dirname3[x] != L'\0' && Dirname3[x] != L'\\'; x++);
				if (Dirname3[x] == L'\\')  x++;
				for (; Dirname3[x] != L'\0'; x++)
				{
					if (Dirname3[x] == L'\\')
					{
						Dirname3[x] = L'\0';
						::CreateDirectoryW(Dirname3, &SecAttr);
						Dirname3[x] = L'\\';
					}
				}
			}

			return (::CreateDirectoryW(Dirname3, &SecAttr) != 0);
		}

		bool Dir::Rmdir(const char *Dirname, bool Recursive)
		{
			char Dirname2[8192];
			size_t y;

			if (!File::GetAbsoluteFilename(Dirname2, 8192, NULL, Dirname, false))  return false;
			y = strlen(Dirname2) + 1;

			WCHAR Dirname3[8192];
			size_t y2 = 8192;
			Util::ConvertFromUTF8((std::uint8_t *)Dirname2, y, Dirname3, y2, sizeof(WCHAR));

			if (Recursive)
			{
				// Don't traverse reparse points.
				DWORD TempAttrs = ::GetFileAttributesW(Dirname3);
				if (TempAttrs != (DWORD)-1 && !(TempAttrs & FILE_ATTRIBUTE_REPARSE_POINT))  Rmdir_RecurseInternal(Dirname3, y2 - 1);
			}

			return (::RemoveDirectoryW(Dirname3) != 0);
		}

		void Dir::Rmdir_RecurseInternal(WCHAR *Dirname, size_t Size)
		{
			if (Size >= 8190)  return;
			Dirname[Size] = L'\\';
			Dirname[Size + 1] = L'*';
			Dirname[Size + 2] = L'\0';

			HANDLE TempHandle;
			WIN32_FIND_DATAW FindData;

			TempHandle = ::FindFirstFileW(Dirname, &FindData);
			if (TempHandle != INVALID_HANDLE_VALUE)
			{
				do
				{
					// Ignore . and .. directories.
					if (wcscmp(FindData.cFileName, L".") && wcscmp(FindData.cFileName, L".."))
					{
						size_t y = wcslen(FindData.cFileName) + 1;
						if (Size + 1 + y <= 8191)
						{
							wcscpy(Dirname + Size + 1, FindData.cFileName);
							if (!(FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))  ::DeleteFileW(Dirname);
							else
							{
								if (!(FindData.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT))  Rmdir_RecurseInternal(Dirname, Size + y);
								::RemoveDirectoryW(Dirname);
							}
						}
					}
				} while (::FindNextFileW(TempHandle, &FindData));

				::FindClose(TempHandle);
			}

			Dirname[Size] = L'\0';
		}
#else
		File::File() : MxFile(-1), MxLocked(false), MxRead(false), MxWrite(false), MxCurrPos(0), MxMaxPos(0)
		{
		}

		File::~File()
		{
			Close();
		}

		bool File::IsOpen() const
		{
			return (MxFile != -1);
		}

		bool File::Open(const char *Filename, int Flags, ShareType ShareFlag, int Mode)
		{
			Close();

			int Access = Flags & O_ACCMODE;

			if (Access == O_RDONLY)
			{
				MxRead = true;
				MxWrite = false;
			}
			else if (Access == O_WRONLY)
			{
				MxRead = false;
				MxWrite = true;
			}
			else if (Access == O_RDWR)
			{
				MxRead = true;
				MxWrite = true;
			}
			else
			{
				return false;
			}

			char Filename2[8192];
			if (!GetPlatformFilename(Filename2, 8192, Filename))  return false;

#ifdef O_BINARY
			Flags &= ~O_TEXT;
			Flags |= O_BINARY;
#endif

#ifdef O_LARGEFILE
			Flags |= O_LARGEFILE;
#endif

#ifdef O_CLOEXEC
			Flags |= O_CLOEXEC;
#endif

			MxFile = ::open(Filename2, Flags, Mode & 0777);
			if (MxFile < 0)
			{
				MxFile = -1;

				return false;
			}

			MxLocked = false;

			// Closest to Windows support is to just lock exclusively for ShareRead and ShareWrite.
			if (ShareFlag == ShareNone || ShareFlag == ShareRead || ShareFlag == ShareWrite)
			{
				if (::flock(MxFile, LOCK_EX | LOCK_NB) < 0)  return false;

				MxLocked = true;
			}
			else if (ShareFlag == ShareBoth)
			{
				if (::flock(MxFile, LOCK_SH | LOCK_NB) < 0)  return false;

				MxLocked = true;
			}
			else  return false;

			off64_t Pos = ::lseek64(MxFile, 0, SEEK_CUR);
			MxCurrPos = (Pos >= 0 ? (std::uint64_t)Pos : 0);
			UpdateMaxPos();

			return true;
		}

		bool File::Seek(SeekType whence, std::uint64_t Pos)
		{
			if (MxFile == -1)  return false;

			std::uint64_t TempPos;

			switch (whence)
			{
				case File::SeekStart:  TempPos = (Pos <= MxMaxPos ? Pos : MxMaxPos);  break;
				case File::SeekEnd:  TempPos = (Pos <= MxMaxPos ? MxMaxPos - Pos : 0);  break;
				case File::SeekForward:  TempPos = (MxCurrPos + Pos <= MxMaxPos ? MxCurrPos + Pos : MxMaxPos);  break;
				case File::SeekBackward:  TempPos = (MxCurrPos >= Pos ? MxCurrPos - Pos : 0);  break;
				default:  TempPos = MxCurrPos;  break;
			}

			if (::lseek64(MxFile, (off64_t)TempPos, SEEK_SET) < 0)  return false;

			MxCurrPos = TempPos;

			return true;
		}

		bool File::Read(std::uint8_t *Data, size_t DataSize, size_t &DataRead)
		{
			if (MxFile == -1 || !MxRead)  return false;

			if ((std::uint64_t)DataSize > MxMaxPos - MxCurrPos)
			{
				UpdateMaxPos();
				if ((std::uint64_t)DataSize > MxMaxPos - MxCurrPos)  DataSize = (size_t)(MxMaxPos - MxCurrPos);
			}

			DataRead = 0;
			if (DataSize == 0)  return true;

			ssize_t BytesRead;
			do
			{
				BytesRead = ::read(MxFile, Data, DataSize);
			} while (BytesRead < 0 && errno == EINTR);
			if (BytesRead < 0)  return false;
			DataRead = (size_t)BytesRead;
			MxCurrPos += (std::uint64_t)BytesRead;

			return true;
		}

		bool File::Write(const std::uint8_t *Data, size_t DataSize, size_t &DataWritten)
		{
			if (MxFile == -1 || !MxWrite)  return false;

			DataWritten = 0;
			if (DataSize == 0)  return true;

			ssize_t BytesWritten;
			do
			{
				BytesWritten = ::write(MxFile, Data, DataSize);
				if (BytesWritten < 0 && errno != EINTR)  return false;

				DataSize -= BytesWritten;
				Data += BytesWritten;
				DataWritten += BytesWritten;
			} while (DataSize);
			MxCurrPos += (std::uint64_t)DataWritten;
			if (MxCurrPos > MxMaxPos)  MxMaxPos = MxCurrPos;

			return true;
		}

		bool File::Flush()
		{
			if (MxFile == -1 || !MxWrite)  return false;

			::sync();

			return true;
		}

		bool File::UpdateMaxPos()
		{
			if (MxFile == -1)  return false;

			struct stat TempStat;
			if (::fstat(MxFile, &TempStat) < 0)  return false;
			MxMaxPos = (std::uint64_t)TempStat.st_size;

			return true;
		}

		bool File::Close()
		{
			if (MxFile != -1)
			{
				::close(MxFile);
				MxFile = -1;
			}

			return true;
		}

		bool File::GetPlatformFilename(char *Result, size_t ResultSize, const char *Filename, bool)
		{
			// Check for invalid characters in the path/filename.  Replace '\' with '/'.
			char TempCP;
			size_t x, y = strlen(Filename);
			if (y + 1 > ResultSize)  return false;
			Util::ConvertToUTF8(Filename, y + 1, sizeof(char), (std::uint8_t *)Result, ResultSize);
			if (!strlen(Result) || strcmp(Result, Filename))  return false;

			// Ignore combining code points since kernels are generally not fully UTF-8 aware.
			for (x = 0; x < y; x++)
			{
				TempCP = Result[x];

				if (TempCP < 0x20 || TempCP == 0x7F || TempCP == '*' || TempCP == '?' || TempCP == '\"' || TempCP == '<' || TempCP == '>' || TempCP == '|')  return false;
				else if (TempCP == '\\')  Result[x] = '/';
			}

			return true;
		}

		void File::GetPlatformFilenameInfo(FilenameInfo &Result, const char *Filename)
		{
			size_t x, y = strlen(Filename);

			Result.Length = y;

			Result.StartVolume = 0;
			Result.StartPath = 0;

			for (x = y; x > Result.StartPath && Filename[x - 1] != '/'; x--);
			Result.StartFilename = x;

			for (; x < y && Filename[x] != '.'; x++);
			Result.StartLastExtension = Result.StartExtension = x;

			for (; x < y; x++)
			{
				if (Filename[x] == '.')  Result.StartLastExtension = x;
			}
		}

		bool File::GetAbsoluteFilename(char *Result, size_t ResultSize, const char *BaseDir, const char *Filename, bool TrailingSlash)
		{
			char Filename2[8192], Filename3[8192];

			if (!GetPlatformFilename(Filename3, 8192, Filename))  return false;

			// Replace '/./' with '/'.
			size_t x, x2, y = strlen(Filename3);
			x2 = 0;
			for (x = 0; x < y; x++)
			{
				while (Filename3[x] == '/' && Filename3[x + 1] == '.' && Filename3[x + 2] == '/')  x += 2;

				Filename3[x2++] = Filename3[x];

				if (Filename3[x] == '/' && Filename3[x + 1] == '.' && Filename3[x + 2] == '\0')  break;
			}
			if (x2 >= 3 && Filename3[x2 - 3] == '/' && Filename3[x2 - 2] == '.' && Filename3[x2 - 1] == '.')
			{
				if (x2 >= 8191)  return false;
				Filename3[x2++] = '/';
			}
			Filename3[x2] = '\0';

			// Get some parsed information about the input data.
			FilenameInfo FileInfo2, FileInfo3;
			GetPlatformFilenameInfo(FileInfo3, Filename3);

			// Copy the absolute volume.
			if (FileInfo3.StartVolume)
			{
				memcpy(Filename2, Filename3, FileInfo3.StartPath);
				Filename2[FileInfo3.StartPath] = '\0';
			}
			else
			{
				if (BaseDir != NULL)
				{
					if (!GetPlatformFilename(Filename2, 8192, BaseDir))  return false;
				}
				else
				{
					if (!Dir::Getcwd(Filename2, 8192))  return false;
				}

				// Require the base directory to be absolute.
				if (Filename2[0] != '/')  return false;
			}

			y = strlen(Filename2);
			if (y && Filename2[y - 1] != '/')
			{
				if (y >= 8191)  return false;
				Filename2[y++] = '/';
				Filename2[y] = '\0';
			}

			GetPlatformFilenameInfo(FileInfo2, Filename2);
			if (Filename3[FileInfo3.StartPath] == '/')  FileInfo2.Length = FileInfo2.StartPath;

			// Make sure the string always starts with a path separator.
			if (FileInfo2.Length >= 8191)  return false;
			if (FileInfo2.StartPath == FileInfo2.Length)  Filename2[FileInfo2.Length++] = '/';

			// Parse the filename into the base directory.
			for (x = FileInfo3.StartPath; x < FileInfo3.Length; x++)
			{
				while (Filename3[x] == '/' && Filename3[x + 1] == '.' && Filename3[x + 2] == '.' && Filename3[x + 3] == '/')
				{
					if (FileInfo2.Length > FileInfo2.StartPath)
					{
						for (; FileInfo2.Length > FileInfo2.StartPath && Filename2[FileInfo2.Length - 1] == '/'; FileInfo2.Length--);
						for (; FileInfo2.Length > FileInfo2.StartPath && Filename2[FileInfo2.Length - 1] != '/'; FileInfo2.Length--);
					}

					x += 3;
				}

				if (FileInfo2.Length >= 8191)  return false;
				Filename2[FileInfo2.Length++] = Filename3[x];
			}

			// Copy the rest.
			x = 0;
			x2 = x;
			for (; x2 < ResultSize && x < FileInfo2.Length; x++)
			{
				while (Filename2[x] == '/' && Filename2[x + 1] == '/')  x++;

				Result[x2++] = Filename2[x];
			}
			if (TrailingSlash && x2 && Result[x2 - 1] != '/' && x2 < ResultSize)  Result[x2++] = '/';
			if (!TrailingSlash && x2 && Result[x2 - 1] == '/')  x2--;
			if (x2 >= ResultSize)  return false;
			Result[x2] = '\0';

			// Sanity check.
			for (x = 0; x < x2; x++)
			{
				if (Result[x] == '/' && Result[x + 1] == '.')
				{
					if (Result[x + 2] == '/')  return false;
					if (Result[x + 2] == '.' && Result[x + 3] == '/')  return false;
				}
			}

			return true;
		}

		bool File::Exists(const char *Filename)
		{
			FileStat TempStat;

			if (!Stat(TempStat, Filename))  return false;

			return true;
		}

		bool File::Realpath(char *Result, size_t ResultSize, const char *Filename)
		{
			char *TempResult = ::realpath(Filename, NULL);
			if (TempResult == NULL)  return false;

			size_t y = strlen(TempResult);
			if (y + 1 <= ResultSize)  strcpy(Result, TempResult);
			::free(TempResult);

			return (y + 1 <= ResultSize);
		}

		bool File::Chmod(const char *Filename, int Mode)
		{
			char Filename2[8192];
			if (!GetPlatformFilename(Filename2, 8192, Filename))  return false;

			return (::chmod(Filename2, (mode_t)Mode) == 0);
		}

		bool File::Chown(const char *Filename, const char *Owner)
		{
			char Filename2[8192];
			if (!GetPlatformFilename(Filename2, 8192, Filename))  return false;

			struct passwd *UserInfo = getpwnam(Owner);
			if (UserInfo == NULL)  return false;

			return (::chown(Filename2, UserInfo->pw_uid, -1) == 0);
		}

		bool File::Chgrp(const char *Filename, const char *Group)
		{
			char Filename2[8192];
			if (!GetPlatformFilename(Filename2, 8192, Filename))  return false;

			struct passwd *UserInfo = getpwnam(Group);
			if (UserInfo == NULL)  return false;

			return (::chown(Filename2, -1, UserInfo->pw_gid) == 0);
		}

		bool File::Copy(const char *SrcFilename, const char *DestFilename)
		{
			char SrcFilename2[8192], DestFilename2[8192];
			if (!GetPlatformFilename(SrcFilename2, 8192, SrcFilename))  return false;
			if (!GetPlatformFilename(DestFilename2, 8192, DestFilename))  return false;

			FileStat SrcStat, DestStat;
			if (stat(SrcFilename2, &SrcStat) != 0 || !S_ISREG(SrcStat.st_mode))  return false;

			if (stat(DestFilename2, &DestStat) == 0 && SrcStat.st_dev == DestStat.st_dev && SrcStat.st_ino == DestStat.st_ino)  return false;

			File SrcFile, DestFile;

			if (!SrcFile.Open(SrcFilename2, O_RDONLY, File::ShareBoth))  return false;
			if (!DestFile.Open(DestFilename2, O_WRONLY | O_CREAT, File::ShareNone, SrcStat.st_mode))  return false;

			std::uint8_t Buffer[8192];
			size_t y;

			// Copy the data.
			while (SrcFile.Read(Buffer, 8192, y) && y)
			{
				DestFile.Write(Buffer, 8192, y);
			}

			// Truncate the file to the current point.
			if (::ftruncate(DestFile.MxFile, (off_t)DestFile.MxCurrPos) == 0)  {}

			DestFile.Close();
			SrcFile.Close();

			// Clone user, group, and permissions.
			if (::chown(DestFilename2, SrcStat.st_uid, SrcStat.st_gid) == 0)  {}
			if (::chmod(DestFilename2, SrcStat.st_mode) == 0)  {}

			// Clone the modified timestamp.
			stat(DestFilename2, &DestStat);
			struct utimbuf TempUTime;
			TempUTime.actime = DestStat.st_atime;
			TempUTime.modtime = SrcStat.st_mtime;
			utime(DestFilename2, &TempUTime);

			return true;
		}

		bool File::Move(const char *SrcFilename, const char *DestFilename)
		{
			char SrcFilename2[8192], DestFilename2[8192];
			if (!GetPlatformFilename(SrcFilename2, 8192, SrcFilename))  return false;
			if (!GetPlatformFilename(DestFilename2, 8192, DestFilename))  return false;

			bool Result = (::rename(SrcFilename2, DestFilename2) == 0);
			if (!Result)
			{
				Result = File::Copy(SrcFilename2, DestFilename2);
				if (Result)  ::unlink(SrcFilename2);
			}

			return Result;
		}

		bool File::Delete(const char *Filename)
		{
			char Filename2[8192];
			if (!GetPlatformFilename(Filename2, 8192, Filename))  return false;

			return (::unlink(Filename2) == 0);
		}

		bool File::Stat(FileStat &Result, const char *Filename, bool Link)
		{
			char Filename2[8192];
			if (!GetPlatformFilename(Filename2, 8192, Filename))  return false;

			return (Link ? (::lstat(Filename2, &Result) == 0) : (::stat(Filename2, &Result) == 0));
		}

		bool File::Symlink(const char *Src, const char *Dest)
		{
			return (::symlink(Dest, Src) == 0);
		}

		bool File::Readlink(char *Result, size_t ResultSize, const char *Filename)
		{
			ssize_t Size = ::readlink(Filename, Result, ResultSize);

			return (Size != -1 && (size_t)Size < ResultSize);
		}

		// Only Windows supports Creation timestamp modification.
		bool File::SetFileTimes(const char *Filename, std::uint64_t *, std::uint64_t *LastAccess, std::uint64_t *LastUpdate)
		{
			char Filename2[8192];
			if (!GetPlatformFilename(Filename2, 8192, Filename))  return false;

			FileStat TempStat;

			if (stat(Filename2, &TempStat) != 0)  return false;

			struct timeval TempTimes[2];

			if (LastAccess == NULL)
			{
				TempTimes[0].tv_sec = TempStat.st_atime;
				TempTimes[0].tv_usec = 0;
			}
			else
			{
				TempTimes[0].tv_sec = (long)(*LastAccess / 1000000);
				TempTimes[0].tv_usec = (long)(*LastAccess % 1000000);
			}

			if (LastUpdate == NULL)
			{
				TempTimes[1].tv_sec = TempStat.st_mtime;
				TempTimes[1].tv_usec = 0;
			}
			else
			{
				TempTimes[1].tv_sec = (long)(*LastUpdate / 1000000);
				TempTimes[1].tv_usec = (long)(*LastUpdate % 1000000);
			}

			return (utimes(Filename2, TempTimes) == 0);
		}


		Dir::Dir() : MxDir(NULL)
		{
		}

		Dir::~Dir()
		{
			Close();
		}

		bool Dir::Open(const char *Dirname)
		{
			Close();

			char Dirname2[8192];

			if (!File::GetAbsoluteFilename(Dirname2, 8192, NULL, Dirname, false))  return false;

			MxDir = ::opendir(Dirname2);

			return (MxDir != NULL);
		}

		bool Dir::Read(char *Filename, size_t Size)
		{
			if (MxDir == NULL)  return false;

			// After much reading and deliberation, readdir() is correct.
			// readdir_r() is wrong, broken, and a bad idea.  Your favorite lint tool is wrong.
			struct dirent *Entry = ::readdir(MxDir);
			if (Entry == NULL)
			{
				Close();

				return false;
			}

			size_t y = strlen(Entry->d_name);
			if (y + 1 > Size)  return false;

			strcpy(Filename, Entry->d_name);

			return true;
		}

		bool Dir::Close()
		{
			if (MxDir == NULL)  return false;

			::closedir(MxDir);
			MxDir = NULL;

			return true;
		}

		bool Dir::Getcwd(char *Result, size_t ResultSize)
		{
			return (::getcwd(Result, ResultSize) != NULL);
		}

		bool Dir::Mkdir(const char *Dirname, int Mode, bool Recursive)
		{
			char Dirname2[8192];

			if (!File::GetAbsoluteFilename(Dirname2, 8192, NULL, Dirname, false))  return false;

			if (Recursive)
			{
				size_t x;

				for (x = 0; Dirname2[x] != '\0' && Dirname2[x] != '/'; x++);
				if (Dirname2[x] == '/')  x++;
				for (; Dirname2[x] != '\0'; x++)
				{
					if (Dirname2[x] == '/')
					{
						Dirname2[x] = '\0';
						::mkdir(Dirname2, Mode);
						Dirname2[x] = '/';
					}
				}
			}

			return (::mkdir(Dirname2, Mode) == 0);
		}

		bool Dir::Rmdir(const char *Dirname, bool Recursive)
		{
			char Dirname2[8192];

			if (!File::GetAbsoluteFilename(Dirname2, 8192, NULL, Dirname, false))  return false;

			if (Recursive)
			{
				// Don't traverse symlinks.
				File::FileStat TempStat;
				if (::lstat(Dirname2, &TempStat) == 0 && !S_ISLNK(TempStat.st_mode))  Rmdir_RecurseInternal(Dirname2, strlen(Dirname2));
			}

			return (::rmdir(Dirname2) == 0);
		}

		void Dir::Rmdir_RecurseInternal(char *Dirname, size_t Size)
		{
			if (Size >= 8190)  return;

			File::FileStat TempStat;
			Dir TempDir;

			if (TempDir.Open(Dirname))
			{
				Dirname[Size] = '/';

				while (TempDir.Read(Dirname + Size + 1, 8192 - Size - 1))
				{
					// Ignore . and .. directories.
					if (strcmp(Dirname + Size + 1, ".") && strcmp(Dirname + Size + 1, ".."))
					{
						if (::lstat(Dirname, &TempStat) == 0)
						{
							if (!S_ISDIR(TempStat.st_mode) || S_ISLNK(TempStat.st_mode))  ::unlink(Dirname);
							else
							{
								Rmdir_RecurseInternal(Dirname, strlen(Dirname));
								::rmdir(Dirname);
							}
						}
					}
				}

				TempDir.Close();

				Dirname[Size] = '\0';
			}
		}
#endif

		// All platforms.
		bool File::Write(const char *Data, size_t &DataWritten)
		{
			return Write((const std::uint8_t *)Data, strlen(Data), DataWritten);
		}

		bool File::IsValidFilenameFormat(const char *Filename)
		{
			char Filename2[8192];

			return GetPlatformFilename(Filename2, 8192, Filename);
		}

		char *File::LineInput(size_t SizeHint, void *AltMallocManager, void *(*AltRealloc)(void *, void *, size_t))
		{
			char *Result, *Result2, *RPos, *NPos;
			size_t x, CurrSize, ReadSize;

			if (!IsOpen() || !MxRead)  return NULL;

			Result = NULL;
			CurrSize = SizeHint;
			do
			{
				if (Result == NULL)  Result = (AltRealloc != NULL ? (char *)AltRealloc(AltMallocManager, NULL, CurrSize + 1) : new char[CurrSize + 1]);
				else
				{
					// Enlarge storage space by SizeHint.
					CurrSize += SizeHint;

					if (AltRealloc != NULL)  Result = (char *)AltRealloc(AltMallocManager, Result, CurrSize + 1);
					else
					{
						Result2 = new char[CurrSize + 1];
						memcpy(Result2, Result, CurrSize);
						delete[] Result;
						Result = Result2;
					}
				}

				Read((std::uint8_t *)(Result + CurrSize - SizeHint), SizeHint, ReadSize);
				if (ReadSize < SizeHint)
				{
					CurrSize -= (SizeHint - ReadSize);

					break;
				}

				NPos = (char *)memchr(Result, '\n', CurrSize);
				if (NPos != NULL)  break;
				RPos = (char *)memchr(Result, '\r', CurrSize);
				if (RPos != NULL && RPos < Result + CurrSize - 1)  break;
			} while (1);

			for (x = 0; x < CurrSize; x++)
			{
				if (Result[x] == '\0')  Result[x] = ' ';
			}

			NPos = (char *)memchr(Result, '\n', CurrSize);
			RPos = (char *)memchr(Result, '\r', CurrSize);

			if (NPos != NULL && RPos != NULL)
			{
				if (NPos < RPos)  RPos = NULL;
				else if (RPos < NPos - 1)  NPos = NULL;
			}

			if (NPos != NULL)
			{
				Seek(File::SeekBackward, CurrSize - (size_t)(NPos - Result) - 1);
				if (RPos != NULL)  *RPos = '\0';
				else  *NPos = '\0';
			}
			else if (RPos != NULL)
			{
				Seek(File::SeekBackward, CurrSize - (size_t)(RPos - Result) - 1);
				*RPos = '\0';
			}
			else
			{
				Result[CurrSize] = '\0';
			}

			return Result;
		}

		bool File::LoadEntireFile(const char *Filename, char *&Result, size_t &BytesRead, void *AltMallocManager, void *(*AltMalloc)(void *, size_t), void (*AltFree)(void *, void *))
		{
			File TempFile;

			Result = NULL;
			BytesRead = 0;

			if (!TempFile.Open(Filename, O_RDONLY))  return false;

			char *TempResult;
			size_t TempSize = (size_t)TempFile.GetMaxPos(), TempSize2;
			if (AltMallocManager != NULL)  TempResult = (char *)AltMalloc(AltMallocManager, TempSize);
			else  TempResult = new char[TempSize];

			if (!TempFile.Read((std::uint8_t *)TempResult, TempSize, TempSize2) || TempSize != TempSize2)
			{
				if (AltMallocManager != NULL)  AltFree(AltMallocManager, TempResult);
				else  delete[] TempResult;

				return false;
			}

			TempFile.Close();

			Result = TempResult;
			BytesRead = TempSize;

			return true;
		}
	}
}