// Cross-platform UTF-8 string conversion and lightweight parser.
// (C) 2013 CubicleSoft.  All Rights Reserved.

#include "utf8_util.h"

namespace CubicleSoft
{
	namespace UTF8
	{
		char *Util::ConvertToUTF8(const void *SrcData, size_t SrcWidth, size_t *LastPos, void *AltMallocManager, void *(*AltMalloc)(void *, size_t))
		{
			size_t x = 0, y;
			std::uint8_t *DestData;

			if (SrcWidth == 1)
			{
				const std::uint8_t *SrcData2 = (const std::uint8_t *)SrcData;

				do
				{
					x++;
				} while (*SrcData2++);
			}
			else if (SrcWidth == 2)
			{
				const uint16_t *SrcData2 = (const uint16_t *)SrcData;

				do
				{
					x++;
				} while (*SrcData2++);
			}
			else if (SrcWidth == 4)
			{
				const std::uint32_t *SrcData2 = (const std::uint32_t *)SrcData;

				do
				{
					x++;
				} while (*SrcData2++);
			}
			else  return NULL;

			ConvertToUTF8(SrcData, x, SrcWidth, NULL, y, NULL);
			DestData = (AltMalloc != NULL ? (std::uint8_t *)AltMalloc(AltMallocManager, y) : new std::uint8_t[y]);
			ConvertToUTF8(SrcData, x, SrcWidth, DestData, y, LastPos);

			return (char *)DestData;
		}

		void *Util::ConvertFromUTF8(const char *SrcData, size_t DestWidth, void *AltMallocManager, void *(*AltMalloc)(void *, size_t))
		{
			size_t x, y;
			void *DestData;

			x = strlen(SrcData);
			ConvertFromUTF8((const std::uint8_t *)SrcData, x, NULL, y, DestWidth);
			if (DestWidth == 1)  DestData = (AltMalloc != NULL ? (std::uint8_t *)AltMalloc(AltMallocManager, y) : new std::uint8_t[y]);
			else if (DestWidth == 2)  DestData = (AltMalloc != NULL ? (std::uint16_t *)AltMalloc(AltMallocManager, y * sizeof(uint16_t)) : new uint16_t[y]);
			else if (DestWidth == 4)  DestData = (AltMalloc != NULL ? (std::uint32_t *)AltMalloc(AltMallocManager, y * sizeof(uint32_t)) : new std::uint32_t[y]);
			else  return NULL;
			ConvertFromUTF8((const std::uint8_t *)SrcData, x, DestData, y, DestWidth);

			return DestData;
		}

		bool Util::AppendUTF8CodePoint(std::uint32_t TempCP, std::uint8_t *DestData, size_t &DestDataSize, size_t MaxDestDataSize)
		{
			// 0xD800-0xDFFF are for UTF-16 surrogate pairs.  Invalid characters.
			// 0xFDD0-0xFDEF are non-characters.
			// 0x*FFFE and 0x*FFFF are reserved.
			if ((TempCP >= 0xD800 && TempCP <= 0xDFFF) || (TempCP >= 0xFDD0 && TempCP <= 0xFDEF) || (TempCP & 0xFFFE) == 0xFFFE)  return false;

			// First character can't be a combining code point.
			if (!DestDataSize && ((TempCP >= 0x0300 && TempCP <= 0x036F) || (TempCP >= 0x1DC0 && TempCP <= 0x1DFF) || (TempCP >= 0x20D0 && TempCP <= 0x20FF) || (TempCP >= 0xFE20 && TempCP <= 0xFE2F)))  return false;

			if (TempCP <= 0x7F)
			{
				if (DestData == NULL)  DestDataSize++;
				else if (DestDataSize >= MaxDestDataSize)  return false;
				else  DestData[DestDataSize++] = (std::uint8_t)TempCP;

				return true;
			}
			else if (TempCP <= 0x07FF)
			{
				if (DestData == NULL)  DestDataSize += 2;
				else if (DestDataSize + 1 >= MaxDestDataSize)  return false;
				else
				{
					DestData[DestDataSize++] = (std::uint8_t)(0xC0 | (TempCP >> 6));
					DestData[DestDataSize++] = (std::uint8_t)(0x80 | (TempCP & 0x3F));
				}

				return true;
			}
			else if (TempCP <= 0xFFFF)
			{
				if (DestData == NULL)  DestDataSize += 3;
				else if (DestDataSize + 2 >= MaxDestDataSize)  return false;
				else
				{
					DestData[DestDataSize++] = (std::uint8_t)(0xE0 | (TempCP >> 12));
					DestData[DestDataSize++] = (std::uint8_t)(0x80 | ((TempCP >> 6) & 0x3F));
					DestData[DestDataSize++] = (std::uint8_t)(0x80 | (TempCP & 0x3F));
				}

				return true;
			}
			else if (TempCP <= 0x10FFFF)
			{
				if (DestData == NULL)  DestDataSize += 4;
				else if (DestDataSize + 3 >= MaxDestDataSize)  return false;
				else
				{
					DestData[DestDataSize++] = (std::uint8_t)(0xF0 | (TempCP >> 18));
					DestData[DestDataSize++] = (std::uint8_t)(0x80 | ((TempCP >> 12) & 0x3F));
					DestData[DestDataSize++] = (std::uint8_t)(0x80 | ((TempCP >> 6) & 0x3F));
					DestData[DestDataSize++] = (std::uint8_t)(0x80 | (TempCP & 0x3F));
				}

				return true;
			}

			return false;
		}

		void Util::ConvertToUTF8(const void *SrcData, size_t SrcDataSize, size_t SrcWidth, std::uint8_t *DestData, size_t &DestDataSize, size_t *LastPos)
		{
			size_t x, y = DestDataSize;
			std::uint32_t TempCP;

			DestDataSize = 0;

			if (SrcWidth == 1)
			{
				const std::uint8_t *SrcData2 = (const std::uint8_t *)SrcData;

				x = 0;
				while (x < SrcDataSize)
				{
					TempCP = SrcData2[x];

					if (TempCP <= 0x7F)
					{
						x++;
						if (AppendUTF8CodePoint(TempCP, DestData, DestDataSize, y) && LastPos != NULL)  *LastPos = x;
					}
					else if (x + 1 < SrcDataSize && (TempCP & 0xE0) == 0xC0 && (SrcData2[x + 1] & 0xC0) == 0x80)
					{
						TempCP = (((TempCP & 0x1F) << 6) | ((std::uint32_t)SrcData2[x + 1] & 0x3F));
						x += 2;
						if (AppendUTF8CodePoint(TempCP, DestData, DestDataSize, y) && LastPos != NULL)  *LastPos = x;
					}
					else if (x + 2 < SrcDataSize && (TempCP & 0xF0) == 0xE0 && (SrcData2[x + 1] & 0xC0) == 0x80 && (SrcData2[x + 2] & 0xC0) == 0x80)
					{
						TempCP = (((TempCP & 0x0F) << 12) | (((std::uint32_t)SrcData2[x + 1] & 0x3F) << 6) | ((std::uint32_t)SrcData2[x + 2] & 0x3F));
						AppendUTF8CodePoint(TempCP, DestData, DestDataSize, y);
						x += 3;
					}
					else if (x + 3 < SrcDataSize && (TempCP & 0xF8) == 0xF0 && (SrcData2[x + 1] & 0xC0) == 0x80 && (SrcData2[x + 2] & 0xC0) == 0x80 && (SrcData2[x + 3] & 0xC0) == 0x80)
					{
						TempCP = (((TempCP & 0x07) << 18) | (((std::uint32_t)SrcData2[x + 1] & 0x3F) << 12) | (((std::uint32_t)SrcData2[x + 2] & 0x3F) << 6) | ((std::uint32_t)SrcData2[x + 3] & 0x3F));
						if (AppendUTF8CodePoint(TempCP, DestData, DestDataSize, y) && LastPos != NULL)  *LastPos = x;
						x += 4;
					}
					else  x++;
				}
			}
			else if (SrcWidth == 2)
			{
				const uint16_t *SrcData2 = (const uint16_t *)SrcData;

				x = 0;
				for (x = 0; x < SrcDataSize; x++)
				{
					TempCP = SrcData2[x];
					if (TempCP < 0xD800 || TempCP > 0xDBFF)
					{
						if (AppendUTF8CodePoint(TempCP, DestData, DestDataSize, y) && LastPos != NULL)  *LastPos = x;
					}
					else
					{
						x++;

						if (x < SrcDataSize && SrcData2[x] >= 0xDC00 && SrcData2[x] <= 0xDFFF)
						{
							TempCP = (((TempCP - 0xD800) << 10) | ((std::uint32_t)SrcData2[x] - 0xDC00)) + 0x010000;
							if (AppendUTF8CodePoint(TempCP, DestData, DestDataSize, y) && LastPos != NULL)  *LastPos = x;
						}
					}
				}
			}
			else if (SrcWidth == 4)
			{
				const std::uint32_t *SrcData2 = (const std::uint32_t *)SrcData;

				for (x = 0; x < SrcDataSize; x++)
				{
					TempCP = SrcData2[x];

					if (AppendUTF8CodePoint(TempCP, DestData, DestDataSize, y) && LastPos != NULL)  *LastPos = x;
				}
			}
		}

		bool Util::AppendUTFCodePoint(std::uint32_t TempCP, void *DestData, size_t &DestDataSize, size_t DestWidth, size_t MaxDestDataSize)
		{
			if (DestWidth == 1)  return AppendUTF8CodePoint(TempCP, (std::uint8_t *)DestData, DestDataSize, MaxDestDataSize);

			// 0xD800-0xDFFF are for UTF-16 surrogate pairs.  Invalid characters.
			// 0xFDD0-0xFDEF are non-characters.
			// 0x*FFFE and 0x*FFFF are reserved.
			// The largest possible character is 0x10FFFF.
			if ((TempCP >= 0xD800 && TempCP <= 0xDFFF) || (TempCP >= 0xFDD0 && TempCP <= 0xFDEF) || (TempCP & 0xFFFE) == 0xFFFE || TempCP > 0x10FFFF)  return false;

			// First character can't be a combining code point.
			if (!DestDataSize && ((TempCP >= 0x0300 && TempCP <= 0x036F) || (TempCP >= 0x1DC0 && TempCP <= 0x1DFF) || (TempCP >= 0x20D0 && TempCP <= 0x20FF) || (TempCP >= 0xFE20 && TempCP <= 0xFE2F)))  return false;

			if (DestWidth == 2)
			{
				uint16_t *DestData2 = (uint16_t *)DestData;

				if (TempCP > 0xFFFF)
				{
					if (DestData2 == NULL)  DestDataSize += 2;
					else if (DestDataSize + 1 >= MaxDestDataSize)  return false;
					else
					{
						TempCP -= 0x010000;
						DestData2[DestDataSize++] = (uint16_t)(((TempCP >> 10) & 0x03FF) + 0xD800);
						DestData2[DestDataSize++] = (uint16_t)((TempCP & 0x03FF) + 0xDC00);
					}
				}
				else if (DestData2 == NULL)  DestDataSize++;
				else if (DestDataSize >= MaxDestDataSize)  return false;
				else  DestData2[DestDataSize++] = (uint16_t)TempCP;
			}
			else if (DestWidth == 4)
			{
				std::uint32_t *DestData2 = (std::uint32_t *)DestData;

				if (DestData2 == NULL)  DestDataSize++;
				else if (DestDataSize >= MaxDestDataSize)  return false;
				else  DestData2[DestDataSize++] = TempCP;
			}

			return true;
		}

		void Util::ConvertFromUTF8(const std::uint8_t *SrcData, size_t SrcDataSize, void *DestData, size_t &DestDataSize, size_t DestWidth)
		{
			size_t x, y;
			std::uint32_t TempCP;

			y = DestDataSize;
			DestDataSize = 0;

			x = 0;
			while (x < SrcDataSize && y)
			{
				TempCP = SrcData[x];

				if (TempCP <= 0x7F)
				{
					if (!AppendUTFCodePoint(TempCP, DestData, DestDataSize, DestWidth, y))  return;
					x++;
				}
				else if (x + 1 < SrcDataSize && (TempCP & 0xE0) == 0xC0 && (SrcData[x + 1] & 0xC0) == 0x80)
				{
					TempCP = (((TempCP & 0x1F) << 6) | ((std::uint32_t)SrcData[x + 1] & 0x3F));
					if (!AppendUTFCodePoint(TempCP, DestData, DestDataSize, DestWidth, y))  return;
					x += 2;
				}
				else if (x + 2 < SrcDataSize && (TempCP & 0xF0) == 0xE0 && (SrcData[x + 1] & 0xC0) == 0x80 && (SrcData[x + 2] & 0xC0) == 0x80)
				{
					TempCP = (((TempCP & 0x0F) << 12) | (((std::uint32_t)SrcData[x + 1] & 0x3F) << 6) | ((std::uint32_t)SrcData[x + 2] & 0x3F));
					if (!AppendUTFCodePoint(TempCP, DestData, DestDataSize, DestWidth, y))  return;
					x += 3;
				}
				else if (x + 3 < SrcDataSize && (TempCP & 0xF8) == 0xF0 && (SrcData[x + 1] & 0xC0) == 0x80 && (SrcData[x + 2] & 0xC0) == 0x80 && (SrcData[x + 3] & 0xC0) == 0x80)
				{
					TempCP = (((TempCP & 0x07) << 18) | (((std::uint32_t)SrcData[x + 1] & 0x3F) << 12) | (((std::uint32_t)SrcData[x + 2] & 0x3F) << 6) | ((std::uint32_t)SrcData[x + 3] & 0x3F));
					if (!AppendUTFCodePoint(TempCP, DestData, DestDataSize, DestWidth, y))  return;
					x += 4;
				}
				else  x++;
			}
		}

		size_t Util::strlen(const char *Str)
		{
			size_t x, y, Result;
			std::uint32_t ResultCP, NextCP;
			bool HasCombiningCP;

			x = 0;
			Result = 0;
			HasCombiningCP = false;
			while (NextCodePoint(ResultCP, NextCP, Str, x, y, HasCombiningCP))
			{
				if (!HasCombiningCP)  Result++;
			}
			if (HasCombiningCP)  Result++;

			return Result;
		}

		bool Util::FindCodePoint(size_t &ResultPos, std::uint32_t CodePoint, const char *Str, bool AllowCombiningCP)
		{
			size_t x, y;
			std::uint32_t ResultCP, NextCP;
			bool HasCombiningCP;

			if (!AllowCombiningCP && IsCombiningCodePoint(CodePoint))  return false;

			x = 0;
			while (NextCodePoint(ResultCP, NextCP, Str, x, y, HasCombiningCP))
			{
				if (ResultCP == CodePoint && (AllowCombiningCP || !HasCombiningCP))
				{
					ResultPos = x;

					return true;
				}
			}

			return false;
		}

		bool Util::NextCodePoint(std::uint32_t &ResultCP, std::uint32_t &NextCP, const char *Str, size_t &Pos, size_t &Size, bool &HasCombiningCP)
		{
			const std::uint8_t *Str2 = (const std::uint8_t *)Str;

			if (Pos)
			{
				ResultCP = NextCP;
				if (ResultCP == 0)  return false;
			}
			else
			{
				Size = strlen(Str);
				HasCombiningCP = false;

				ResultCP = Str2[0];
				if (ResultCP == 0)  return false;

				if (ResultCP <= 0x7F)  Pos++;
				else if (Pos + 1 < Size && (ResultCP & 0xE0) == 0xC0 && (Str2[Pos + 1] & 0xC0) == 0x80)  Pos += 2;
				else if (Pos + 2 < Size && (ResultCP & 0xF0) == 0xE0 && (Str2[Pos + 1] & 0xC0) == 0x80 && (Str2[Pos + 2] & 0xC0) == 0x80)  Pos += 3;
				else if (Pos + 3 < Size && (ResultCP & 0xF8) == 0xF0 && (Str2[Pos + 1] & 0xC0) == 0x80 && (Str2[Pos + 2] & 0xC0) == 0x80 && (Str2[Pos + 3] & 0xC0) == 0x80)  Pos += 4;
				else  return false;

				// First character can't be a combining code point.
				if ((ResultCP >= 0x0300 && ResultCP <= 0x036F) || (ResultCP >= 0x1DC0 && ResultCP <= 0x1DFF) || (ResultCP >= 0x20D0 && ResultCP <= 0x20FF) || (ResultCP >= 0xFE20 && ResultCP <= 0xFE2F))  return false;
			}

			NextCP = Str2[Pos];

			if (NextCP != 0)
			{
				if (NextCP <= 0x7F)  Pos++;
				else if (Pos + 1 < Size && (NextCP & 0xE0) == 0xC0 && (Str2[Pos + 1] & 0xC0) == 0x80)  Pos += 2;
				else if (Pos + 2 < Size && (NextCP & 0xF0) == 0xE0 && (Str2[Pos + 1] & 0xC0) == 0x80 && (Str2[Pos + 2] & 0xC0) == 0x80)  Pos += 3;
				else if (Pos + 3 < Size && (NextCP & 0xF8) == 0xF0 && (Str2[Pos + 1] & 0xC0) == 0x80 && (Str2[Pos + 2] & 0xC0) == 0x80 && (Str2[Pos + 3] & 0xC0) == 0x80)  Pos += 4;
				else  NextCP = 0;
			}

			HasCombiningCP = ((NextCP >= 0x0300 && NextCP <= 0x036F) || (NextCP >= 0x1DC0 && NextCP <= 0x1DFF) || (NextCP >= 0x20D0 && NextCP <= 0x20FF) || (NextCP >= 0xFE20 && NextCP <= 0xFE2F));

			return true;
		}

		bool Util::NextASCIICodePoint(char &ResultCP, std::uint32_t &NextCP, const char *Str, size_t &Pos, size_t &Size)
		{
			bool HasCombiningCP;
			std::uint32_t TempCP;

			while (NextCodePoint(TempCP, NextCP, Str, Pos, Size, HasCombiningCP))
			{
				if (!HasCombiningCP && TempCP <= 0x7F)
				{
					ResultCP = (char)TempCP;

					return true;
				}
			}

			return false;
		}

		bool Util::IsCombiningCodePoint(std::uint32_t CodePoint)
		{
			return ((CodePoint >= 0x0300 && CodePoint <= 0x036F) || (CodePoint >= 0x1DC0 && CodePoint <= 0x1DFF) || (CodePoint >= 0x20D0 && CodePoint <= 0x20FF) || (CodePoint >= 0xFE20 && CodePoint <= 0xFE2F));
		}
	}
}
