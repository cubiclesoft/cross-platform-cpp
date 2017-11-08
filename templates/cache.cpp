// Cache.
// (C) 2013 CubicleSoft.  All Rights Reserved.

#include "cache.h"

namespace CubicleSoft
{
	// Implements djb2.
	// WARNING:  This algorithm is weak security-wise!  http://www.youtube.com/watch?v=R2Cq3CLI6H8
	// It is generally okay to use in this instance since the cache template is a partial hash.
	// For a more secure hash function, try SipHash:  https://github.com/veorq/SipHash

	size_t CacheUtil::GetHashKey(char *Str)
	{
		size_t Result = 5381, TempChr;

		while ((TempChr = *Str++))
		{
			Result = ((Result << 5) + Result) ^ TempChr;
		}

		return Result;
	}

	size_t CacheUtil::GetHashKey(std::uint8_t *Str, size_t Size)
	{
		std::uint32_t Result = 5381;
		std::uint32_t y;
		const size_t NumLeft = Size & 3;
		const std::uint8_t *StrEnd = Str + Size - NumLeft;

		while (Str != StrEnd)
		{
			// Changes the official implementation.
			y = *((std::uint32_t *)Str);

			Result = ((Result << 5) + Result) ^ y;

			Str += 4;
		}

		switch (NumLeft)
		{
			case 3:  Result = ((Result << 5) + Result) ^ ((std::uint32_t)Str[2]);
			case 2:  Result = ((Result << 5) + Result) ^ ((std::uint32_t)Str[1]);
			case 1:  Result = ((Result << 5) + Result) ^ ((std::uint32_t)Str[0]);  break;
			case 0:  break;
		}

		return (size_t)Result;
	}
}
