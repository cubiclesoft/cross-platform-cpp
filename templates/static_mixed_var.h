// A mixed, flexible variable data type (plain ol' data - POD) with all-public data access.
// (C) 2016 CubicleSoft.  All Rights Reserved.

#ifndef CUBICLESOFT_STATIC_MIXED_VAR
#define CUBICLESOFT_STATIC_MIXED_VAR

#include <cstdint>
#include <cstddef>
#include <cstring>

namespace CubicleSoft
{
	enum StaticMixedVarModes
	{
		MV_None,
		MV_Bool,
		MV_Int,
		MV_UInt,
		MV_Double,
		MV_Str
	};

	// Must be used like:  StaticMixedVar<char[8192]>
	// Designed to be extended but not overridden.
	template <class T>
	class StaticMixedVar
	{
	public:
		StaticMixedVarModes MxMode;
		std::int64_t MxInt;
		double MxDouble;
		T MxStr;
		size_t MxStrPos;

		StaticMixedVar() : MxMode(MV_None), MxInt(0), MxDouble(0.0), MxStrPos(0)
		{
		}

		// Some functions for those who prefer member functions over directly accessing raw class data.
		inline bool IsNone()
		{
			return (MxMode == MV_None);
		}

		inline bool IsBool()
		{
			return (MxMode == MV_Bool);
		}

		inline bool IsInt()
		{
			return (MxMode == MV_Int);
		}

		inline bool IsUInt()
		{
			return (MxMode == MV_UInt);
		}

		inline bool IsDouble()
		{
			return (MxMode == MV_Double);
		}

		inline bool IsStr()
		{
			return (MxMode == MV_Str);
		}

		inline bool GetBool()
		{
			return (MxInt != 0);
		}

		inline std::int64_t GetInt()
		{
			return MxInt;
		}

		inline std::uint64_t GetUInt()
		{
			return (std::uint64_t)MxInt;
		}

		inline double GetDouble()
		{
			return MxDouble;
		}

		inline char *GetStr()
		{
			return MxStr;
		}

		inline size_t GetSize()
		{
			return MxStrPos;
		}

		inline size_t GetMaxSize()
		{
			return sizeof(MxStr);
		}

		inline void SetBool(bool newbool)
		{
			MxMode = MV_Bool;
			MxInt = (int)newbool;
		}

		inline void SetInt(std::int64_t newint)
		{
			MxMode = MV_Int;
			MxInt = newint;
		}

		inline void SetUInt(std::uint64_t newint)
		{
			MxMode = MV_UInt;
			MxInt = (std::int64_t)newint;
		}

		inline void SetDouble(double newdouble)
		{
			MxMode = MV_Double;
			MxDouble = newdouble;
		}

		void SetStr(const char *str)
		{
			MxMode = MV_Str;
			MxStrPos = 0;
			while (MxStrPos < sizeof(MxStr) - 1 && *str)
			{
				MxStr[MxStrPos++] = *str++;
			}
			MxStr[MxStrPos] = '\0';
		}

		void SetData(const char *str, size_t size)
		{
			MxMode = MV_Str;
			MxStrPos = 0;
			while (MxStrPos < sizeof(MxStr) - 1 && size)
			{
				MxStr[MxStrPos++] = *str++;
				size--;
			}
			MxStr[MxStrPos] = '\0';
		}

		// Only prepends if there is enough space.
		void PrependStr(const char *str)
		{
			size_t y = strlen(str);
			if (MxStrPos + y < sizeof(MxStr) - 1)
			{
				memmove(MxStr + y, MxStr, MxStrPos + 1);
				memcpy(MxStr, str, y);
				MxStrPos += y;
			}
		}

		// Only prepends if there is enough space.
		void PrependData(const char *str, size_t size)
		{
			if (MxStrPos + size < sizeof(MxStr) - 1)
			{
				memmove(MxStr + size, MxStr, MxStrPos + 1);
				memcpy(MxStr, str, size);
				MxStrPos += size;
			}
		}

		void AppendStr(const char *str)
		{
			while (MxStrPos < sizeof(MxStr) - 1 && *str)
			{
				MxStr[MxStrPos++] = *str++;
			}
			MxStr[MxStrPos] = '\0';
		}

		void AppendData(const char *str, size_t size)
		{
			while (MxStrPos < sizeof(MxStr) - 1 && size)
			{
				MxStr[MxStrPos++] = *str++;
				size--;
			}
			MxStr[MxStrPos] = '\0';
		}

		inline void AppendChar(char chr)
		{
			if (MxStrPos < sizeof(MxStr) - 1)
			{
				MxStr[MxStrPos++] = chr;
				MxStr[MxStrPos] = '\0';
			}
		}

		inline void AppendMissingChar(char chr)
		{
			if ((!MxStrPos || MxStr[MxStrPos - 1] != chr) && MxStrPos < sizeof(MxStr) - 1)
			{
				MxStr[MxStrPos++] = chr;
				MxStr[MxStrPos] = '\0';
			}
		}

		inline bool RemoveTrailingChar(char chr)
		{
			if (!MxStrPos || MxStr[MxStrPos - 1] != chr)  return false;

			MxStr[--MxStrPos] = '\0';

			return true;
		}

		inline void SetSize(size_t size)
		{
			if (size < sizeof(MxStr))
			{
				MxStrPos = size;
				MxStr[MxStrPos] = '\0';
			}
		}
	};
}

#endif
