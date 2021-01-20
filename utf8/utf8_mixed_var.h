// Adds UTF8 support to the MixedVar template.
// (C) 2021 CubicleSoft.  All Rights Reserved.

#ifndef CUBICLESOFT_UTF8_MIXED_VAR
#define CUBICLESOFT_UTF8_MIXED_VAR

#include "../templates/static_mixed_var.h"
#include "utf8_util.h"

#if defined(_WIN32) || defined(WIN32) || defined(_WIN64) || defined(WIN64)
	#include <windows.h>
#else
	#include <wchar.h>

	#ifndef WCHAR
		#define WCHAR wchar_t
	#endif
#endif

namespace CubicleSoft
{
	namespace UTF8
	{
		// Must be used like:  UTF8MixedVar<char[8192]>
		// Designed to be extended but not overridden.
		template <class T>
		class UTF8MixedVar : public StaticMixedVar<T>
		{
		public:
			void SetUTF8(const WCHAR *src, size_t srcsize)
			{
				this->MxMode = MV_Str;
				this->MxStrPos = sizeof(this->MxStr);
				Util::ConvertToUTF8(src, srcsize, sizeof(WCHAR), (std::uint8_t *)this->MxStr, this->MxStrPos);
				if (this->MxStrPos == sizeof(this->MxStr) || (this->MxStrPos && this->MxStr[this->MxStrPos - 1] == '\0'))  this->MxStrPos--;
				this->MxStr[this->MxStrPos] = '\0';
			}

			inline void SetUTF8(const WCHAR *src)
			{
				SetUTF8(src, wcslen(src));
			}

			void ConvertFromUTF8(WCHAR *DestData, size_t &DestDataSize)
			{
				Util::ConvertFromUTF8((std::uint8_t *)this->MxStr, this->MxStrPos + 1, DestData, DestDataSize, sizeof(WCHAR));
			}

			void AppendUTF8(const WCHAR *src)
			{
				size_t srcsize = wcslen(src);
				if (srcsize > sizeof(this->MxStr) - this->MxStrPos)  srcsize = sizeof(this->MxStr) - this->MxStrPos;
				Util::ConvertToUTF8(src, srcsize, sizeof(WCHAR), (std::uint8_t *)this->MxStr + this->MxStrPos, srcsize);
				this->MxStrPos += srcsize;
				if (this->MxStrPos == sizeof(this->MxStr) || (this->MxStrPos && this->MxStr[this->MxStrPos - 1] == '\0'))  this->MxStrPos--;
				this->MxStr[this->MxStrPos] = '\0';
			}
		};
	}
}

#endif