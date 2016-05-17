// Binary data single pattern finder with templated replace function.  Mostly for arrays of plain ol' data (POD) types.
// General performance is probably better than std::search and even possibly better than Boyer–Moore and Knuth–Morris–Pratt.
// (C) 2016 CubicleSoft.  All Rights Reserved.

#ifndef CUBICLESOFT_FAST_FIND_REPLACE
#define CUBICLESOFT_FAST_FIND_REPLACE

#include <cstddef>
#include <new>

namespace CubicleSoft
{
	// FastFind and FastReplace.
	#include "fast_find_replace_util.h"

	// FastFind and FastReplace with custom allocator.
	#define CUBICLESOFT_FAST_FIND_REPLACE_MEMALLOC
	#include "fast_find_replace_util.h"
	#undef CUBICLESOFT_FAST_FIND_REPLACE_MEMALLOC

	// FastFind and FastReplace with custom element comparison.
	#define CUBICLESOFT_FAST_FIND_REPLACE_COMPAREFUNC
	#include "fast_find_replace_util.h"
	#undef CUBICLESOFT_FAST_FIND_REPLACE_COMPAREFUNC

	// FastFind and FastReplace with custom allocator and custom element comparison.
	#define CUBICLESOFT_FAST_FIND_REPLACE_MEMALLOC
	#define CUBICLESOFT_FAST_FIND_REPLACE_COMPAREFUNC
	#include "fast_find_replace_util.h"
	#undef CUBICLESOFT_FAST_FIND_REPLACE_COMPAREFUNC
	#undef CUBICLESOFT_FAST_FIND_REPLACE_MEMALLOC
}

#endif
