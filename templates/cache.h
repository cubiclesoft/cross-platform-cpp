// Cache.
// (C) 2013 CubicleSoft.  All Rights Reserved.

#ifndef CUBICLESOFT_CACHE
#define CUBICLESOFT_CACHE

#include <cstdint>
#include <cstring>

namespace CubicleSoft
{
	template <class XKey, class XValue>
	class CacheNode
	{
	public:
		CacheNode() : Used(false)
		{
		}

		bool Used;
		XKey Key;
		XValue Value;
	};

	// Prime numbers closest to powers of 2 make good hash table sizes (in theory).
	// Therefore, the best hash table primes are:
	//   3, 5, 11, 23, 47, 97, 191, 383, 769, 1531, 3067, 6143, 12289, 24571, 49157, 98299,
	//   196613, 393209, 786431, 1572869, 3145721, 6291449, 12582917, 25165813, 50331653,
	//   100663291, 201326611, 402653189, 805306357, 1610612741, 3221225473

	// Cache.  A hash of overwritable storage.
	#include "cache_util.h"

	// CacheNoCopy.  A hash of overwritable storage with a private copy constructor and assignment operator.
	#define CUBICLESOFT_CACHE_NOCOPYASSIGN
	#include "cache_util.h"
	#undef CUBICLESOFT_CACHE_NOCOPYASSIGN

	// Static functions that take a string and convert to integer.
	class CacheUtil
	{
	public:
		static size_t GetHashKey(char *Str);
		static size_t GetHashKey(std::uint8_t *Str, size_t Size);
	};
}

#endif
