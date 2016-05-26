// Binary data single pattern finder with templated replace function.  Mostly for arrays of plain ol' data (POD) types.
// General performance is probably better than std::search and even possibly better than Boyer–Moore and Knuth–Morris–Pratt.
// (C) 2016 CubicleSoft.  All Rights Reserved.

// NOTE:  This file is intended to be included from 'fast_find_replace.h'.
#ifdef CUBICLESOFT_FAST_FIND_REPLACE_MEMALLOC
	#ifdef CUBICLESOFT_FAST_FIND_REPLACE_COMPAREFUNC
template <class T, class AllocType, class ElementsEqualType>
class FastFindAllocCompare
{
public:
	FastFindAllocCompare()
	#else
template <class T, class AllocType>
class FastFindAlloc
{
public:
	FastFindAlloc()
	#endif
#else
	#ifdef CUBICLESOFT_FAST_FIND_REPLACE_COMPAREFUNC
template <class T, class ElementsEqualType>
class FastFindCompare
{
public:
	FastFindCompare()
	#else
template <class T>
class FastFind
{
public:
	FastFind()
	#endif
#endif
	{
		MxPattern = MxPatternLast = MxPatternMin = MxPatternMin2 = MxPatternMidpoint = NULL;
		MxPatternLastPos = MxPatternMinPos = MxPatternMin2Pos = MxPatternMidpointPos = 0;
		MxData = MxDataEnd = MxCurrPos = NULL;

// Technically, THIS is all that is needed for initialization BUT lint tools hate it and therefore the above is necessary.  Sigh.
//		MxPattern = NULL;
//		MxPatternLastPos = 0;
//		MxData = NULL;
	}

	// Sets the pattern to search for.  Does not duplicate the pattern.
	// Check out Sync::TLS for a higher-performance temporary allocator if SetPattern() is called frequently for larger patterns (> 1,204 values).
#ifdef CUBICLESOFT_FAST_FIND_REPLACE_MEMALLOC
	#ifdef CUBICLESOFT_FAST_FIND_REPLACE_COMPAREFUNC
	void SetPattern(const T *Pattern, size_t Size, AllocType *AltMallocManager, ElementsEqualType ElementsEqual)
	#else
	void SetPattern(const T *Pattern, size_t Size, AllocType *AltMallocManager)
	#endif
#else
	#ifdef CUBICLESOFT_FAST_FIND_REPLACE_COMPAREFUNC
	void SetPattern(const T *Pattern, size_t Size, ElementsEqualType ElementsEqual)
	#else
	void SetPattern(const T *Pattern, size_t Size)
	#endif
#endif
	{
		if (!Size)
		{
			MxPattern = NULL;
			MxPatternLastPos = 0;

			return;
		}

		// For the general case, this alone will produce extremely fast results.
		// "How many words/phrases of precisely Size length start and end with these values?"  Not many.
		// The most important observation here is the length of the pattern.
		MxData = NULL;
		MxPattern = Pattern;
		MxPatternLast = (Size > 1 ? Pattern + Size - 1 : NULL);
		MxPatternLastPos = Size - 1;

		// Worst-case complexity of the above involves small alphabets, a long-ish pattern consisting mostly of one value,
		// and a massive search space consisting mostly of one value.
		// For example:  aaaaaaaaaaaaaaaaZaaaaaaaaaaa and a search space of all 'a's and just one 'Z'.
		// Performance can be dramatically improved by locating a couple of "least-used" values.
		// The general case performance for multiple word patterns also dramatically improves too.  Average case becomes O(5n).
		MxPatternMin = NULL;
		MxPatternMin2 = NULL;
		MxPatternMidpoint = NULL;
		if (Size >= 5)
		{
			const T *Pos = Pattern + 1, *Pos2;
			size_t x, x2, y, Min = (size_t)-1, SeenSize = Size - 2;
			char SeenStack[1024];
			char *Seen = SeenStack;
#ifdef CUBICLESOFT_FAST_FIND_REPLACE_MEMALLOC
			if (SeenSize > 1024)  Seen = (char *)AllocType::malloc(AltMallocManager, SeenSize);
#else
			if (SeenSize > 1024)  Seen = new char[SeenSize];
#endif
			for (x = 0; x < SeenSize; x++)  Seen[x] = 0;

			x = 0;
			while (Pos < MxPatternLast)
			{
				if (!Seen[x])
				{
					y = 1;
					Seen[x] = 1;
					Pos2 = Pos + 1;
					for (x2 = x + 1; x2 < SeenSize; x2++)
					{
#ifdef CUBICLESOFT_FAST_FIND_REPLACE_COMPAREFUNC
						if (ElementsEqual(*Pos, *Pos2))
#else
						if (*Pos == *Pos2)
#endif
						{
							Seen[x2] = 1;
							y++;
						}

						Pos2++;
					}

					if (Min >= y)
					{
						Min = y;
						if (Size >= 10)  MxPatternMin2 = MxPatternMin;
						MxPatternMin = Pos;
					}
				}

				x++;
				Pos++;
			}

			if (SeenSize > 1024)
			{
#ifdef CUBICLESOFT_FAST_FIND_REPLACE_MEMALLOC
				AllocType::free(AltMallocManager, Seen);
#else
				delete[] Seen;
#endif
			}

			// In the unfortunate case, the start could be the minimum but nothing else in the pattern beats it.
			// For example:  1223334445556667778889 will select '2' for MxPatternMin but MxPatternMin2 will be NULL.
			// Set MxPatternMin2 to MxPatternMin and set MxPatternMin to the midpoint.
			if (MxPatternMin != NULL && MxPatternMin2 == NULL)
			{
				if (Size >= 10)
				{
					MxPatternMin2 = MxPatternMin;
					MxPatternMin = MxPatternMin2 + ((MxPatternLast - MxPatternMin2) / 2);
				}
				else if (MxPatternMin == MxPatternLast - 1)
				{
					MxPatternMin = MxPattern + ((MxPatternLast - MxPattern) / 2);
				}
			}

			// If the pattern is made up entirely of mostly unique values, MxPatternMin and MxPatternMin2 will
			// probably be clumped together at the end of the pattern.
			// For example:  1234567890abcdefghijklmnopqrstuvwxyz will select 'y' for MxPatternMin and 'x' for MxPatternMin2.
			// There is a significant reduction in compares if there is some spread between MxPatternMin and MxPatternMin2.
			// Move MxPatternMin2 to the beginning.
			if (MxPatternMin2 != NULL && MxPatternMin2 == MxPatternLast - 2)  MxPatternMin2 = MxPattern + 1;

			MxPatternMinPos = (size_t)(MxPatternMin - MxPattern);
			if (MxPatternMin2 != NULL)  MxPatternMin2Pos = (size_t)(MxPatternMin2 - MxPattern);

			// Ignoring clumping issues, larger patterns might also have a significant gap.
			// For example:  01112223334445556667778889990 will select the first '9' for MxPatternMin and the first '8' for MxPatternMin2.
			// Find the largest open distance and select the midpoint.
			if (MxPatternMin != NULL && MxPatternMin2 != NULL && Size >= 15)
			{
				Pos = NULL;
				Pos2 = NULL;
				y = 1;

				x = (size_t)(MxPatternMin2 - MxPattern);
				if (y < x)
				{
					y = x;
					Pos = MxPattern;
				}

				x = (size_t)(MxPatternMin - MxPatternMin2);
				if (y < x)
				{
					y = x;
					Pos = MxPatternMin2;
				}

				x = (size_t)(MxPatternLast - MxPatternMin);
				if (y < x)
				{
					y = x;
					Pos = MxPatternMin;
				}

				if (y > 1)
				{
					MxPatternMidpoint = Pos + (y / 2);
					MxPatternMidpointPos = (size_t)(MxPatternMidpoint - MxPattern);
				}
			}
		}
	}

	// Sets the various internal pointers to the data.  Does not duplicate the data.
	void SetData(const T *Data, size_t Size)
	{
		Size = (Size > MxPatternLastPos ? Size - MxPatternLastPos : 0);

		if (!Size)
		{
			MxData = NULL;

			return;
		}

		MxData = Data;
		MxDataEnd = Data + Size;
		MxCurrPos = Data;
	}

	// Locates the next match.  When using objects that overload the == operator, UseMemcmp should be false.
#ifdef CUBICLESOFT_FAST_FIND_REPLACE_COMPAREFUNC
	const T *FindNext(ElementsEqualType ElementsEqual)
#else
	const T *FindNext(bool UseMemcmp = true)
#endif
	{
		if (MxData == NULL || MxPattern == NULL)  return NULL;

		if (MxPatternMidpoint != NULL)
		{
			// Large search pattern.
#ifdef CUBICLESOFT_FAST_FIND_REPLACE_COMPAREFUNC
			size_t x;
#else
			size_t x = sizeof(T) * (MxPatternLastPos - 1);
#endif
			for (; MxCurrPos < MxDataEnd; MxCurrPos++)
			{
#ifdef CUBICLESOFT_FAST_FIND_REPLACE_COMPAREFUNC
				if (ElementsEqual(*MxPattern, *MxCurrPos) && ElementsEqual(*MxPatternLast, MxCurrPos[MxPatternLastPos]) && ElementsEqual(*MxPatternMin, MxCurrPos[MxPatternMinPos]) && ElementsEqual(*MxPatternMin2, MxCurrPos[MxPatternMin2Pos]) && ElementsEqual(*MxPatternMidpoint, MxCurrPos[MxPatternMidpointPos]))
				{
					for (x = 1; x < MxPatternLastPos && ElementsEqual(MxPattern[x], MxCurrPos[x]); x++)  {}
					if (x == MxPatternLastPos)  break;
				}
#else
				if (*MxPattern == *MxCurrPos && *MxPatternLast == MxCurrPos[MxPatternLastPos] && *MxPatternMin == MxCurrPos[MxPatternMinPos] && *MxPatternMin2 == MxCurrPos[MxPatternMin2Pos] && *MxPatternMidpoint == MxCurrPos[MxPatternMidpointPos])
				{
					if (UseMemcmp)
					{
						if (!memcmp(MxPattern + 1, MxCurrPos + 1, x))  break;
					}
					else
					{
						for (x = 1; x < MxPatternLastPos && MxPattern[x] == MxCurrPos[x]; x++)  {}
						if (x == MxPatternLastPos)  break;
					}
				}
#endif
			}
		}
		else if (MxPatternMin2 != NULL)
		{
			// Mid-size search pattern.
#ifdef CUBICLESOFT_FAST_FIND_REPLACE_COMPAREFUNC
			size_t x;
#else
			size_t x = sizeof(T) * (MxPatternLastPos - 1);
#endif
			for (; MxCurrPos < MxDataEnd; MxCurrPos++)
			{
#ifdef CUBICLESOFT_FAST_FIND_REPLACE_COMPAREFUNC
				if (ElementsEqual(*MxPattern, *MxCurrPos) && ElementsEqual(*MxPatternLast, MxCurrPos[MxPatternLastPos]) && ElementsEqual(*MxPatternMin, MxCurrPos[MxPatternMinPos]) && ElementsEqual(*MxPatternMin2, MxCurrPos[MxPatternMin2Pos]))
				{
					for (x = 1; x < MxPatternLastPos && ElementsEqual(MxPattern[x], MxCurrPos[x]); x++)  {}
					if (x == MxPatternLastPos)  break;
				}
#else
				if (*MxPattern == *MxCurrPos && *MxPatternLast == MxCurrPos[MxPatternLastPos] && *MxPatternMin == MxCurrPos[MxPatternMinPos] && *MxPatternMin2 == MxCurrPos[MxPatternMin2Pos])
				{
					if (UseMemcmp)
					{
						if (!memcmp(MxPattern + 1, MxCurrPos + 1, x))  break;
					}
					else
					{
						for (x = 1; x < MxPatternLastPos && MxPattern[x] == MxCurrPos[x]; x++)  {}
						if (x == MxPatternLastPos)  break;
					}
				}
#endif
			}
		}
		else if (MxPatternMin != NULL)
		{
			// Mid-size search pattern.
#ifdef CUBICLESOFT_FAST_FIND_REPLACE_COMPAREFUNC
			size_t x;
#else
			size_t x = sizeof(T) * (MxPatternLastPos - 1);
#endif
			for (; MxCurrPos < MxDataEnd; MxCurrPos++)
			{
#ifdef CUBICLESOFT_FAST_FIND_REPLACE_COMPAREFUNC
				if (ElementsEqual(*MxPattern, *MxCurrPos) && ElementsEqual(*MxPatternLast, MxCurrPos[MxPatternLastPos]) && ElementsEqual(*MxPatternMin, MxCurrPos[MxPatternMinPos]))
				{
					for (x = 1; x < MxPatternLastPos && ElementsEqual(MxPattern[x], MxCurrPos[x]); x++)  {}
					if (x == MxPatternLastPos)  break;
				}
#else
				if (*MxPattern == *MxCurrPos && *MxPatternLast == MxCurrPos[MxPatternLastPos] && *MxPatternMin == MxCurrPos[MxPatternMinPos])
				{
					if (UseMemcmp)
					{
						if (!memcmp(MxPattern + 1, MxCurrPos + 1, x))  break;
					}
					else
					{
						for (x = 1; x < MxPatternLastPos && MxPattern[x] == MxCurrPos[x]; x++)  {}
						if (x == MxPatternLastPos)  break;
					}
				}
#endif
			}
		}
		else if (MxPatternLast != NULL)
		{
			// Small search pattern.
			size_t x;
			for (; MxCurrPos < MxDataEnd; MxCurrPos++)
			{
#ifdef CUBICLESOFT_FAST_FIND_REPLACE_COMPAREFUNC
				if (ElementsEqual(*MxPattern, *MxCurrPos) && ElementsEqual(*MxPatternLast, MxCurrPos[MxPatternLastPos]))
				{
					for (x = 1; x < MxPatternLastPos && ElementsEqual(MxPattern[x], MxCurrPos[x]); x++)  {}
					if (x == MxPatternLastPos)  break;
				}
#else
				if (*MxPattern == *MxCurrPos && *MxPatternLast == MxCurrPos[MxPatternLastPos])
				{
					for (x = 1; x < MxPatternLastPos && MxPattern[x] == MxCurrPos[x]; x++)  {}
					if (x == MxPatternLastPos)  break;
				}
#endif
			}
		}
		else
		{
			// One element search pattern.  More useful when combined with FastReplace.
#ifdef CUBICLESOFT_FAST_FIND_REPLACE_COMPAREFUNC
			while (MxCurrPos < MxDataEnd && !ElementsEqual(*MxPattern, *MxCurrPos))  MxCurrPos++;
#else
			while (MxCurrPos < MxDataEnd && *MxPattern != *MxCurrPos)  MxCurrPos++;
#endif
		}

		const T *Result;
		if (MxCurrPos < MxDataEnd)
		{
			Result = MxCurrPos;
			MxCurrPos++;
		}
		else
		{
			Result = NULL;
			MxData = NULL;
		}

		return Result;
	}

	// By default, FindNext() moves to the next value.  This skips past the last complete match that was found.
	const T *SkipMatch(size_t Extra = 0)
	{
		if (MxData == NULL || MxPattern == NULL)  return NULL;

		MxCurrPos += MxPatternLastPos + Extra;

		if (MxCurrPos >= MxDataEnd)  MxData = NULL;

		return MxCurrPos;
	}

private:
	const T *MxPattern, *MxPatternLast, *MxPatternMin, *MxPatternMin2, *MxPatternMidpoint;
	size_t MxPatternLastPos, MxPatternMinPos, MxPatternMin2Pos, MxPatternMidpointPos;
	const T *MxData, *MxDataEnd, *MxCurrPos;
};

#ifdef CUBICLESOFT_FAST_FIND_REPLACE_MEMALLOC
	#ifdef CUBICLESOFT_FAST_FIND_REPLACE_COMPAREFUNC
template <class T, class AllocType, class ElementsEqualType>
class FastReplaceAllocCompare
{
public:
	FastReplaceAllocCompare()
	{
	#else
template <class T, class AllocType>
class FastReplaceAlloc
{
public:
	FastReplaceAlloc()
	{
	#endif

		MxAltMallocManager = NULL;

#else
	#ifdef CUBICLESOFT_FAST_FIND_REPLACE_COMPAREFUNC
template <class T, class ElementsEqualType>
class FastReplaceCompare
{
public:
	FastReplaceCompare()
	{
#ifdef CUBICLESOFT_FAST_FIND_REPLACE_MEMALLOC
		MxAltMallocManager = NULL;
#endif
	#else
template <class T>
class FastReplace
{
public:
	FastReplace()
	{
	#endif
#endif
		MxGrowthSize = 1024;
		MxExtraAlloc = 1;
		MxData = NULL;
	}

#ifdef CUBICLESOFT_FAST_FIND_REPLACE_MEMALLOC
	#ifdef CUBICLESOFT_FAST_FIND_REPLACE_COMPAREFUNC
	~FastReplaceAllocCompare()
	#else
	~FastReplaceAlloc()
	#endif
#else
	#ifdef CUBICLESOFT_FAST_FIND_REPLACE_COMPAREFUNC
	~FastReplaceCompare()
	#else
	~FastReplace()
	#endif
#endif
	{
		if (MxData != NULL && MxDataAllocated)
		{
#ifdef CUBICLESOFT_FAST_FIND_REPLACE_MEMALLOC
			AllocType::free(MxAltMallocManager, MxData);
#else
			delete[] reinterpret_cast<char *>(MxData);
#endif
		}
	}

	// Sets both the pattern and the alternate allocation manager (if any).
#ifdef CUBICLESOFT_FAST_FIND_REPLACE_MEMALLOC
	#ifdef CUBICLESOFT_FAST_FIND_REPLACE_COMPAREFUNC
	void SetPattern(const T *Pattern, size_t Size, AllocType *AltMallocManager, ElementsEqualType ElementsEqual)
	#else
	void SetPattern(const T *Pattern, size_t Size, AllocType *AltMallocManager)
	#endif
	{
		if (MxData != NULL && MxDataAllocated)
		{
			AllocType::free(MxAltMallocManager, MxData);

			MxData = NULL;
		}

	#ifdef CUBICLESOFT_FAST_FIND_REPLACE_COMPAREFUNC
		MxFind.SetPattern(Pattern, Size, AltMallocManager, ElementsEqual);
	#else
		MxFind.SetPattern(Pattern, Size, AltMallocManager);
	#endif

		MxAltMallocManager = AltMallocManager;
	}
#else
	#ifdef CUBICLESOFT_FAST_FIND_REPLACE_COMPAREFUNC
	void SetPattern(const T *Pattern, size_t Size, ElementsEqualType ElementsEqual)
	#else
	void SetPattern(const T *Pattern, size_t Size)
	#endif
	{
		if (MxData != NULL && MxDataAllocated)
		{
			delete[] reinterpret_cast<char *>(MxData);

			MxData = NULL;
		}

	#ifdef CUBICLESOFT_FAST_FIND_REPLACE_COMPAREFUNC
		MxFind.SetPattern(Pattern, Size, ElementsEqual);
	#else
		MxFind.SetPattern(Pattern, Size);
	#endif
	}
#endif

	inline void SetGrowthSize(size_t GrowthSize)
	{
		MxGrowthSize = GrowthSize;
	}

	inline void SetExtraAlloc(size_t ExtraAlloc)
	{
		MxExtraAlloc = ExtraAlloc;
	}

	// Only pass in ResultBuffer and ResultBufferSize for static buffers and replacements that will be smaller than the original.
	void SetData(const T *Data, size_t Size, T *ResultBuffer = NULL, size_t ResultBufferSize = 0)
	{
		MxFind.SetData(Data, Size);

		MxSrcCurr = Data;
		MxSrcEnd = Data + Size;
		MxDataPos = 0;
		if (ResultBuffer == NULL)
		{
			MxDataAllocated = true;
			MxDataSize = Size + MxGrowthSize;
#ifdef CUBICLESOFT_FAST_FIND_REPLACE_MEMALLOC
			MxData = reinterpret_cast<T *>(AllocType::malloc(MxAltMallocManager, (MxDataSize + MxExtraAlloc) * sizeof(T)));
#else
			MxData = reinterpret_cast<T *>(new char[(MxDataSize + MxExtraAlloc) * sizeof(T)]);
#endif
		}
		else
		{
			MxDataAllocated = false;
			MxDataSize = ResultBufferSize;
			MxData = ResultBuffer;
		}
	}

#ifdef CUBICLESOFT_FAST_FIND_REPLACE_COMPAREFUNC
	const T *FindNext(ElementsEqualType ElementsEqual)
	{
		const T *NextPos = MxFind.FindNext(ElementsEqual);
#else
	const T *FindNext(bool UseMemcmp = true)
	{
		const T *NextPos = MxFind.FindNext(UseMemcmp);
#endif

		size_t SrcDist = (NextPos == NULL ? MxSrcEnd : NextPos) - MxSrcCurr;

		// Determine if there is enough space or if the result needs to be resized.
		if (MxDataSize < MxDataPos + SrcDist || (MxDataAllocated && NextPos == NULL))
		{
			if (MxDataAllocated)
			{
				size_t NewSize = MxDataPos + SrcDist + (NextPos == NULL ? 0 : MxGrowthSize);

#ifdef CUBICLESOFT_FAST_FIND_REPLACE_MEMALLOC
				MxData = reinterpret_cast<T *>(AllocType::realloc(MxAltMallocManager, MxData, (NewSize + MxExtraAlloc) * sizeof(T)));
#else
				T *NewData = reinterpret_cast<T *>(new char[(NewSize + MxExtraAlloc) * sizeof(T)]);
				memcpy(NewData, MxData, NewSize * sizeof(T));
				delete[] reinterpret_cast<char *>(MxData);

				MxData = NewData;
#endif

				// Use placement new on MxExtraAlloc entries.  Should compile to nothing for POD types (char, int, etc.) but will call the constructor on objects.
				if (NextPos == NULL)
				{
					for (size_t x = 0; x < MxExtraAlloc; x++)  new (MxData + NewSize + x) T;
				}

				MxDataSize = NewSize;
			}
			else
			{
				SrcDist = MxDataSize - MxDataPos;

				NextPos = NULL;
			}
		}

		if (SrcDist)
		{
			memcpy(MxData + MxDataPos, MxSrcCurr, SrcDist * sizeof(T));
			MxDataPos += SrcDist;
		}

		return NextPos;
	}

	void Replace(const T *Data, size_t DataSize, size_t PostPatternSize = 0)
	{
		if (DataSize)
		{
			// Determine if there is enough space.
			if (MxDataSize < MxDataPos + DataSize)
			{
				if (MxDataAllocated)
				{
					size_t NewSize = MxDataPos + DataSize + MxGrowthSize;

#ifdef CUBICLESOFT_FAST_FIND_REPLACE_MEMALLOC
					MxData = reinterpret_cast<T *>(AllocType::realloc(MxAltMallocManager, MxData, (NewSize + MxExtraAlloc) * sizeof(T)));
#else
					T *NewData = reinterpret_cast<T *>(new char[(NewSize + MxExtraAlloc) * sizeof(T)]);
					memcpy(NewData, MxData, (MxDataSize + MxExtraAlloc) * sizeof(T));
					delete[] reinterpret_cast<char *>(MxData);

					MxData = NewData;
#endif

					MxDataSize = NewSize;
				}
				else
				{
					DataSize = MxDataSize - MxDataPos;
				}
			}

			memcpy(MxData + MxDataPos, Data, DataSize * sizeof(T));
			MxDataPos += DataSize;
		}

		MxSrcCurr = MxFind.SkipMatch(PostPatternSize);
		if (MxSrcCurr > MxSrcEnd)  MxSrcCurr = MxSrcEnd;
	}

	// DetachData() is a better option.
	inline T *GetData()
	{
		return MxData;
	}

	inline T *DetachData()
	{
		T *Result = MxData;

		MxData = NULL;

		return Result;
	}

	inline size_t GetDataSize()
	{
		return MxDataPos;
	}

private:
	static size_t GetObjectSize(const T *Obj)
	{
		size_t y;

		for (y = 0; Obj[y]; y++)  {}

		return y;
	}

public:
	// Replaces all exact instances of Pattern with ReplaceWith in Data up to Limit with a single function call into a dynamically allocated output buffer.
	// Check out Sync::TLS for a higher-performance allocator.
#ifdef CUBICLESOFT_FAST_FIND_REPLACE_MEMALLOC
	#ifdef CUBICLESOFT_FAST_FIND_REPLACE_COMPAREFUNC
	static size_t ReplaceAll(T *&Result, size_t &ResultSize, const T *Data, size_t DataSize, const T *Pattern, size_t PatternSize, const T *ReplaceWith, size_t ReplaceWithSize, AllocType *AltMallocManager, ElementsEqualType ElementsEqual, size_t Limit = (size_t)-1, size_t GrowthSize = 1024, size_t ExtraAlloc = 1)
	{
		FastReplaceAllocCompare<T, AllocType, ElementsEqualType> TempReplace;

		if (PatternSize == (size_t)-1)  PatternSize = GetObjectSize(Pattern);

		TempReplace.SetPattern(Pattern, PatternSize, AltMallocManager, ElementsEqual);
	#else
	static size_t ReplaceAll(T *&Result, size_t &ResultSize, const T *Data, size_t DataSize, const T *Pattern, size_t PatternSize, const T *ReplaceWith, size_t ReplaceWithSize, AllocType *AltMallocManager, size_t Limit = (size_t)-1, size_t GrowthSize = 1024, size_t ExtraAlloc = 1, bool UseMemcmp = true)
	{
		FastReplaceAlloc<T, AllocType> TempReplace;

		if (PatternSize == (size_t)-1)  PatternSize = GetObjectSize(Pattern);

		TempReplace.SetPattern(Pattern, PatternSize, AltMallocManager);
	#endif
#else
	#ifdef CUBICLESOFT_FAST_FIND_REPLACE_COMPAREFUNC
	static size_t ReplaceAll(T *&Result, size_t &ResultSize, const T *Data, size_t DataSize, const T *Pattern, size_t PatternSize, const T *ReplaceWith, size_t ReplaceWithSize, ElementsEqualType ElementsEqual, size_t Limit = (size_t)-1, size_t GrowthSize = 1024, size_t ExtraAlloc = 1)
	{
		FastReplaceCompare<T, ElementsEqualType> TempReplace;

		if (PatternSize == (size_t)-1)  PatternSize = GetObjectSize(Pattern);

		TempReplace.SetPattern(Pattern, PatternSize, ElementsEqual);
	#else
	static size_t ReplaceAll(T *&Result, size_t &ResultSize, const T *Data, size_t DataSize, const T *Pattern, size_t PatternSize, const T *ReplaceWith, size_t ReplaceWithSize, size_t Limit = (size_t)-1, size_t GrowthSize = 1024, size_t ExtraAlloc = 1, bool UseMemcmp = true)
	{
		FastReplace<T> TempReplace;

		if (PatternSize == (size_t)-1)  PatternSize = GetObjectSize(Pattern);

		TempReplace.SetPattern(Pattern, PatternSize);
	#endif
#endif

		if (ReplaceWithSize == (size_t)-1)  ReplaceWithSize = GetObjectSize(ReplaceWith);

		if (PatternSize < ReplaceWithSize && GrowthSize < (ReplaceWithSize - PatternSize) * 50)  GrowthSize = (ReplaceWithSize - PatternSize) * 50;
		TempReplace.SetGrowthSize(GrowthSize);

		TempReplace.SetExtraAlloc(ExtraAlloc);

		TempReplace.SetData(Data, DataSize);

		const T *Pos;
		size_t Num = 0;
#ifdef CUBICLESOFT_FAST_FIND_REPLACE_COMPAREFUNC
		while ((Pos = TempReplace.FindNext(ElementsEqual)) != NULL && Num < Limit)
#else
		while ((Pos = TempReplace.FindNext(UseMemcmp)) != NULL && Num < Limit)
#endif
		{
			TempReplace.Replace(ReplaceWith, ReplaceWithSize);

			Num++;
		}

		ResultSize = TempReplace.GetDataSize();
		Result = TempReplace.DetachData();

		return Num;
	}

	// Replaces all exact instances of Pattern with ReplaceWith in Data up to Limit with a single function call into a static output buffer.
	// Check out Sync::TLS for a higher-performance allocator.
#ifdef CUBICLESOFT_FAST_FIND_REPLACE_MEMALLOC
	#ifdef CUBICLESOFT_FAST_FIND_REPLACE_COMPAREFUNC
	static size_t StaticReplaceAll(T *Result, size_t &ResultSize, const T *Data, size_t DataSize, const T *Pattern, size_t PatternSize, const T *ReplaceWith, size_t ReplaceWithSize, AllocType *AltMallocManager, ElementsEqualType ElementsEqual, size_t Limit = (size_t)-1, size_t GrowthSize = 1024, size_t ExtraAlloc = 1)
	{
		FastReplaceAllocCompare<T, AllocType, ElementsEqualType> TempReplace;

		if (PatternSize == (size_t)-1)  PatternSize = GetObjectSize(Pattern);

		TempReplace.SetPattern(Pattern, PatternSize, AltMallocManager, ElementsEqual);
	#else
	static size_t StaticReplaceAll(T *Result, size_t &ResultSize, const T *Data, size_t DataSize, const T *Pattern, size_t PatternSize, const T *ReplaceWith, size_t ReplaceWithSize, AllocType *AltMallocManager, size_t Limit = (size_t)-1, size_t GrowthSize = 1024, size_t ExtraAlloc = 1, bool UseMemcmp = true)
	{
		FastReplaceAlloc<T, AllocType> TempReplace;

		if (PatternSize == (size_t)-1)  PatternSize = GetObjectSize(Pattern);

		TempReplace.SetPattern(Pattern, PatternSize, AltMallocManager);
	#endif
#else
	#ifdef CUBICLESOFT_FAST_FIND_REPLACE_COMPAREFUNC
	static size_t StaticReplaceAll(T *Result, size_t &ResultSize, const T *Data, size_t DataSize, const T *Pattern, size_t PatternSize, const T *ReplaceWith, size_t ReplaceWithSize, ElementsEqualType ElementsEqual, size_t Limit = (size_t)-1, size_t GrowthSize = 1024, size_t ExtraAlloc = 1)
	{
		FastReplaceCompare<T, ElementsEqualType> TempReplace;

		if (PatternSize == (size_t)-1)  PatternSize = GetObjectSize(Pattern);

		TempReplace.SetPattern(Pattern, PatternSize, ElementsEqual);
	#else
	static size_t StaticReplaceAll(T *Result, size_t &ResultSize, const T *Data, size_t DataSize, const T *Pattern, size_t PatternSize, const T *ReplaceWith, size_t ReplaceWithSize, size_t Limit = (size_t)-1, size_t GrowthSize = 1024, size_t ExtraAlloc = 1, bool UseMemcmp = true)
	{
		FastReplace<T> TempReplace;

		if (PatternSize == (size_t)-1)  PatternSize = GetObjectSize(Pattern);

		TempReplace.SetPattern(Pattern, PatternSize);
	#endif
#endif

		if (ReplaceWithSize == (size_t)-1)  ReplaceWithSize = GetObjectSize(ReplaceWith);

		if (PatternSize < ReplaceWithSize && GrowthSize < (ReplaceWithSize - PatternSize) * 50)  GrowthSize = (ReplaceWithSize - PatternSize) * 50;
		TempReplace.SetGrowthSize(GrowthSize);

		TempReplace.SetExtraAlloc(ExtraAlloc);

		TempReplace.SetData(Data, DataSize, Result, ResultSize);

		const T *Pos;
		size_t Num = 0;
#ifdef CUBICLESOFT_FAST_FIND_REPLACE_COMPAREFUNC
		while ((Pos = TempReplace.FindNext(ElementsEqual)) != NULL && Num < Limit)
#else
		while ((Pos = TempReplace.FindNext(UseMemcmp)) != NULL && Num < Limit)
#endif
		{
			TempReplace.Replace(ReplaceWith, ReplaceWithSize);

			Num++;
		}

		ResultSize = TempReplace.GetDataSize();

		return Num;
	}

private:
#ifdef CUBICLESOFT_FAST_FIND_REPLACE_MEMALLOC
	#ifdef CUBICLESOFT_FAST_FIND_REPLACE_COMPAREFUNC
	FastFindAllocCompare<T, AllocType, ElementsEqualType> MxFind;
	#else
	FastFindAlloc<T, AllocType> MxFind;
	#endif

	void *MxAltMallocManager;

#else
	#ifdef CUBICLESOFT_FAST_FIND_REPLACE_COMPAREFUNC
	FastFindCompare<T, ElementsEqualType> MxFind;
	#else
	FastFind<T> MxFind;
	#endif
#endif

	size_t MxGrowthSize, MxExtraAlloc;
	const T *MxSrcCurr, *MxSrcEnd;
	bool MxDataAllocated;
	T *MxData;
	size_t MxDataSize, MxDataPos;
};
