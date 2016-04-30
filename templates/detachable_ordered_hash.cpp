// Ordered Hash Utilities.
// (C) 2014 CubicleSoft.  All Rights Reserved.

#include "detachable_ordered_hash.h"

namespace CubicleSoft
{
	// Prime numbers closest to powers of 2 make good hash table sizes (in theory).
	// Therefore, the best hash table primes are:
	const size_t OrderedHashUtil::Primes[31] = {
		3, 5, 11, 23, 47, 97, 191, 383, 769, 1531, 3067, 6143, 12289, 24571, 49157, 98299,
		196613, 393209, 786431, 1572869, 3145721, 6291449, 12582917, 25165813, 50331653,
		100663291, 201326611, 402653189, 805306357, 1610612741, 3221225473
	};

	size_t OrderedHashUtil::GetDJBX33XHashKey(const std::uint8_t *Str, size_t Size, size_t InitVal)
	{
		std::uint32_t Result = (std::uint32_t)InitVal;
		std::uint32_t y;
		const std::uint8_t *StrEnd = Str + Size - (Size % sizeof(std::uint32_t));
		const size_t NumLeft = Size & 3;

		while (Str != StrEnd)
		{
			// Changes the official implementation.
			y = *((const std::uint32_t *)Str);

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

	#define ROTL(x, b) (std::uint64_t)(((x) << (b)) | ((x) >> (64 - (b))))

	#define SIPROUND \
		do { \
			v0 += v1; v1 = ROTL(v1, 13); v1 ^= v0; v0 = ROTL(v0, 32); \
			v2 += v3; v3 = ROTL(v3, 16); v3 ^= v2; \
			v0 += v3; v3 = ROTL(v3, 21); v3 ^= v0; \
			v2 += v1; v1 = ROTL(v1, 17); v1 ^= v2; v2 = ROTL(v2, 32); \
		} while(0)

	std::uint64_t OrderedHashUtil::GetSipHashKey(const std::uint8_t *Str, size_t Size, std::uint64_t Key1, std::uint64_t Key2, size_t cRounds, size_t dRounds)
	{
		// "somepseudorandomlygeneratedbytes"
		std::uint64_t v0 = 0x736f6d6570736575ULL;
		std::uint64_t v1 = 0x646f72616e646f6dULL;
		std::uint64_t v2 = 0x6c7967656e657261ULL;
		std::uint64_t v3 = 0x7465646279746573ULL;
		std::uint64_t Result;
		std::uint64_t y;
		size_t x;
		const std::uint8_t *StrEnd = Str + Size - (Size % sizeof(std::uint64_t));
		const size_t NumLeft = Size & 7;

		Result = ((std::uint64_t)Size) << 56;
		v3 ^= Key2;
		v2 ^= Key1;
		v1 ^= Key2;
		v0 ^= Key1;
		while (Str != StrEnd)
		{
			// Minor change to the official implementation.  (Does endianness actually matter?)
			y = *((const std::uint64_t *)Str);

			v3 ^= y;

			for (x = 0; x < cRounds; ++x)  SIPROUND;

			v0 ^= y;

			Str += 8;
		}

		switch (NumLeft)
		{
			case 7:  Result |= ((std::uint64_t)Str[6]) << 48;
			case 6:  Result |= ((std::uint64_t)Str[5]) << 40;
			case 5:  Result |= ((std::uint64_t)Str[4]) << 32;
			case 4:  Result |= ((std::uint64_t)Str[3]) << 24;
			case 3:  Result |= ((std::uint64_t)Str[2]) << 16;
			case 2:  Result |= ((std::uint64_t)Str[1]) << 8;
			case 1:  Result |= ((std::uint64_t)Str[0]);  break;
			case 0:  break;
		}

		v3 ^= Result;

		for (x = 0; x < cRounds; ++x)  SIPROUND;

		v0 ^= Result;
		v2 ^= 0xff;

		for (x = 0; x < dRounds; ++x)  SIPROUND;

		Result = v0 ^ v1 ^ v2 ^ v3;

		return Result;
	}
}
