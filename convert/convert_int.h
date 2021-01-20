// Cross-platform integer conversion functions.
// (C) 2021 CubicleSoft.  All Rights Reserved.

#ifndef CUBICLESOFT_CONVERT_INT
#define CUBICLESOFT_CONVERT_INT

#include <cstdint>
#include <cstring>

namespace CubicleSoft
{
	namespace Convert
	{
		class Int
		{
		public:
			static bool ToString(char *Result, size_t Size, std::uint64_t Num, char Separator = '\0', size_t Radix = 10);
			static bool ToString(char *Result, size_t Size, std::int64_t Num, char Separator = '\0', size_t Radix = 10);

			static bool ToFilesizeString(char *Result, size_t Size, std::uint64_t Num, size_t NumFrac = 2, char NumSeparator = '\0', char DecimalSeparator = '.', char UnitSeparator = ' ', const char *BytesUnit = "bytes");
			static bool ToFilesizeString(char *Result, size_t Size, std::int64_t Num, size_t NumFrac = 2, char NumSeparator = '\0', char DecimalSeparator = '.', char UnitSeparator = ' ', const char *BytesUnit = "bytes");
		};
	}
}

#endif
