// Cross-platform integer conversion functions.
// (C) 2021 CubicleSoft.  All Rights Reserved.

#define _CRT_SECURE_NO_WARNINGS

#include "convert_int.h"

#include <cstring>

namespace CubicleSoft
{
	namespace Convert
	{
		bool Int::ToString(char *Result, size_t Size, std::uint64_t Num, char Separator, size_t Radix)
		{
			if (Size < 2 || Radix < 2 || Radix > 36)  return false;

			size_t x = Size, y = 0, z;

			Result[--x] = '\0';
			if (!Num)  Result[--x] = '0';
			else
			{
				while (Num && x)
				{
					if (Separator != '\0' && y && y % 3 == 0)
					{
						Result[--x] = Separator;
						if (!x)  return false;
					}

					z = Num % Radix;
					Result[--x] = (char)(z > 9 ? z - 10 + 'A' : z + '0');
					Num /= Radix;
					y++;
				}

				if (Num)  return false;
			}

			memmove(Result, Result + x, Size - x);

			return true;
		}

		bool Int::ToString(char *Result, size_t Size, std::int64_t Num, char Separator, size_t Radix)
		{
			if (Num >= 0)  return ToString(Result, Size, (std::uint64_t)Num, Separator, Radix);

			if (Size < 2)  return false;
			Result[0] = '-';

			return ToString(Result + 1, Size - 1, (std::uint64_t)-Num, Separator, Radix);
		}

		bool Int::ToFilesizeString(char *Result, size_t Size, std::uint64_t Num, size_t NumFrac, char NumSeparator, char DecimalSeparator, char UnitSeparator, const char *BytesUnit)
		{
			char Trail;
			std::uint64_t Divider, Extra;

			if (Size < 5 + strlen(BytesUnit))  return false;

			size_t x = Size, x2, y = 0;

			if (NumFrac)
			{
				// Windows/web browser format.
				if (Num >= 1125899906842624000ULL)
				{
					Trail = 'E';
					Divider = 1152921504606846976ULL;
				}
				else if (Num >= 1099511627776000ULL)
				{
					Trail = 'P';
					Divider = 1125899906842624ULL;
				}
				else if (Num >= 1073741824000ULL)
				{
					Trail = 'T';
					Divider = 1099511627776ULL;
				}
				else if (Num >= 1048576000ULL)
				{
					Trail = 'G';
					Divider = 1073741824ULL;
				}
				else if (Num >= 1024000ULL)
				{
					Trail = 'M';
					Divider = 1048576ULL;
				}
				else if (Num >= 1024ULL)
				{
					Trail = 'K';
					Divider = 1024ULL;
					NumFrac = 0;
				}
				else
				{
					Trail = '\0';
					Divider = 1ULL;
					NumFrac = 0;
				}
			}
			else
			{
				// Strict format.
				if (Num >= 1152921504606846976ULL)
				{
					Trail = 'E';
					Divider = 1152921504606846976ULL;
				}
				else if (Num >= 1125899906842624ULL)
				{
					Trail = 'P';
					Divider = 1125899906842624ULL;
				}
				else if (Num >= 1099511627776ULL)
				{
					Trail = 'T';
					Divider = 1099511627776ULL;
				}
				else if (Num >= 1073741824ULL)
				{
					Trail = 'G';
					Divider = 1073741824ULL;
				}
				else if (Num >= 1048576ULL)
				{
					Trail = 'M';
					Divider = 1048576ULL;
				}
				else if (Num >= 1024ULL)
				{
					Trail = 'K';
					Divider = 1024ULL;
					NumFrac = 0;
				}
				else
				{
					Trail = '\0';
					Divider = 1ULL;
					NumFrac = 0;
				}
			}

			if (Trail != '\0')
			{
				Result[--x] = '\0';
				Result[--x] = 'B';
				Result[--x] = Trail;
			}
			else
			{
				x = Size - strlen(BytesUnit) - 1;
				strcpy(Result + x, BytesUnit);
			}

			if (UnitSeparator != '\0')  Result[--x] = UnitSeparator;

			if (NumFrac)
			{
				if (x < NumFrac + 2)  return false;

				Extra = Num % Divider;
				for (x2 = 0; x2 < NumFrac; x2++)  Extra *= 10ULL;
				Extra /= Divider;

				while (Extra && NumFrac && x)
				{
					Result[--x] = (char)(Extra % 10) + '0';
					Extra /= 10;
					NumFrac--;
				}

				while (NumFrac && x)
				{
					Result[--x] = '0';
					NumFrac--;
				}

				if (Extra || !x)  return false;

				Result[--x] = DecimalSeparator;

				if (!x)  return false;
			}

			Num /= Divider;

			if (!Num)  Result[--x] = '0';
			else
			{
				while (Num && x)
				{
					if (NumSeparator != '\0' && y && y % 3 == 0)
					{
						Result[--x] = NumSeparator;
						if (!x)  return false;
					}

					Result[--x] = (char)(Num % 10) + '0';
					Num /= 10;
					y++;
				}

				if (Num)  return false;
			}

			memmove(Result, Result + x, Size - x);

			return true;
		}

		bool Int::ToFilesizeString(char *Result, size_t Size, std::int64_t Num, size_t NumFrac, char NumSeparator, char DecimalSeparator, char UnitSeparator, const char *BytesUnit)
		{
			if (Num >= 0)  return ToFilesizeString(Result, Size, (std::uint64_t)Num, NumFrac, NumSeparator, DecimalSeparator, UnitSeparator, BytesUnit);

			if (Size < 2)  return false;
			Result[0] = '-';

			return ToFilesizeString(Result + 1, Size - 1, (std::uint64_t)-Num, NumFrac, NumSeparator, DecimalSeparator, UnitSeparator, BytesUnit);
		}
	}
}
