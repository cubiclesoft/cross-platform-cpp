// Cross-platform UTF-8 string conversion and lightweight parser utilities.
// (C) 2016 CubicleSoft.  All Rights Reserved.

#ifndef CUBICLESOFT_UTF8_UTIL
#define CUBICLESOFT_UTF8_UTIL

#include <cstdint>
#include <cstdlib>

namespace CubicleSoft
{
	namespace UTF8
	{
		class Util
		{
		public:
			// Converts wide "characters" (Unicode) to and from UTF-8.
			// Strict transformation without Unicode normalization.  Some stream support.
			// Designed for OS/compiler "wide char" to/from UTF-8 transformations.
			// For example, SrcWidth and DestWidth might be sizeof(TCHAR) on Windows for a (TCHAR *) string.

			// Generally avoid these for small buffers.  Small, exact memory allocations = bad.
			// Check out Sync::TLS for a higher-performance allocator.
			static char *ConvertToUTF8(const void *SrcData, size_t SrcWidth, size_t *LastPos = NULL, void *AltMallocManager = NULL, void *(*AltMalloc)(void *, size_t) = NULL);
			static void *ConvertFromUTF8(const char *SrcData, size_t DestWidth, void *AltMallocManager = NULL, void *(*AltMalloc)(void *, size_t) = NULL);

			// More efficient for small buffers.
			static void ConvertToUTF8(const void *SrcData, size_t SrcDataSize, size_t SrcWidth, std::uint8_t *DestData, size_t &DestDataSize, size_t *LastPos = NULL);
			static void ConvertFromUTF8(const std::uint8_t *SrcData, size_t SrcDataSize, void *DestData, size_t &DestDataSize, size_t DestWidth);


			// Very basic parser functions for UTF-8 strings.
			// Literally anything beyond this *requires* a (multi-)MB library such as ICU or Boost.Locale.
			// Unicode strings are ideally treated as purely opaque within an application.
			// Realistically, some parsing will still take place.

			// Does not count combining code points (good) but not all grapheme clusters such as Hangul (not as good).
			static size_t strlen(const char *Str);

			static bool FindCodePoint(size_t &ResultPos, std::uint32_t CodePoint, const char *Str, bool AllowCombiningCP = false);

			// Returns false on invalid characters.  Use ConvertToUTF8 to clean up strings.
			// NOTE:  Pos must be initialized to 0 for the first call or the function will assume a continuation.
			static bool NextCodePoint(std::uint32_t &ResultCP, std::uint32_t &NextCP, const char *Str, size_t &Pos, size_t &Size, bool &HasCombiningCP);

			// Finds strict ASCII characters <= 0x7F (no combining code points).
			// NOTE:  Pos must be initialized to 0 for the first call or the function will assume a continuation.
			static bool NextASCIICodePoint(char &ResultCP, std::uint32_t &NextCP, const char *Str, size_t &Pos, size_t &Size);

			static bool IsCombiningCodePoint(std::uint32_t CodePoint);

		private:
			static bool AppendUTF8CodePoint(std::uint32_t TempCP, std::uint8_t *DestData, size_t &DestDataSize, size_t MaxDestDataSize);
			static bool AppendUTFCodePoint(std::uint32_t TempCP, void *DestData, size_t &DestDataSize, size_t DestWidth, size_t MaxDestDataSize);
		};
	}
}

#endif