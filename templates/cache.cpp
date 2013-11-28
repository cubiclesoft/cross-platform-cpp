// Cache.
// (C) 2013 CubicleSoft.  All Rights Reserved.

#include "cache.h"

namespace CubicleSoft
{
	// Implements djb2.
	// WARNING:  This algorithm is weak security-wise!  http://www.youtube.com/watch?v=R2Cq3CLI6H8
	// It is generally okay to use in this instance since the cache template is a partial hash.

	size_t CacheUtil::GetHashKey(char *Str)
	{
		size_t Result = 5381, TempChr;

		while ((TempChr = *Str++))
		{
			Result = ((Result << 5) + Result) + TempChr;
		}

		return Result;
	}

	size_t CacheUtil::GetHashKey(std::uint8_t *Str, size_t Size)
	{
		size_t Result = 5381, TempChr;

		while (Size)
		{
			TempChr = *Str++;
			Result = ((Result << 5) + Result) + TempChr;

			Size--;
		}

		return Result;
	}
}
