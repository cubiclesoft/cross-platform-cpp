// Cross-platform JSON serialization class.
// (C) 2021 CubicleSoft.  All Rights Reserved.

#define _CRT_SECURE_NO_WARNINGS

#include "json_serializer.h"
#include <cstdio>

namespace CubicleSoft
{
	namespace JSON
	{
		Serializer::Serializer(const bool EscapeSlashes, const char *KeySplitter, const char *ValSplitter, size_t MaxDepth)
			: MxModes(MaxDepth), MxModeDepth(0), MxBuffer(NULL), MxBufferPos(0), MxBufferSize(0), MxEscapeSlashes(EscapeSlashes), MxKeySplitter(KeySplitter), MxValSplitter(ValSplitter)
		{
			MxModes[0] = ModeRootFirst;
			MxKeySplitterLen = strlen(KeySplitter);
			MxValSplitterLen = strlen(ValSplitter);
		}

		void Serializer::Reset()
		{
			MxModes[0] = ModeRootFirst;
			MxModeDepth = 0;
			MxBufferPos = 0;
		}

		void Serializer::SetBuffer(std::uint8_t *Buffer, size_t BufferSize)
		{
			MxBuffer = Buffer;
			MxBufferPos = 0;
			MxBufferSize = BufferSize - 1;
		}

		bool Serializer::StartObject(const char *Key)
		{
			if (MxModeDepth + 1 >= MxModes.GetSize() || !InternalAppendNextPrefix(Key, 1))  return false;

			MxBuffer[MxBufferPos++] = '{';
			MxBuffer[MxBufferPos] = '\0';

			MxModeDepth++;
			MxModes[MxModeDepth] = ModeObjectFirst;

			return true;
		}

		bool Serializer::EndObject()
		{
			if (MxModes[MxModeDepth] != ModeObjectFirst && MxModes[MxModeDepth] != ModeObjectNext)  return false;

			if (MxBufferPos + 1 > MxBufferSize)  return false;

			MxBuffer[MxBufferPos++] = '}';
			MxBuffer[MxBufferPos] = '\0';

			MxModeDepth--;

			return true;
		}

		bool Serializer::StartArray(const char *Key)
		{
			if (MxModeDepth + 1 >= MxModes.GetSize() || !InternalAppendNextPrefix(Key, 1))  return false;

			MxBuffer[MxBufferPos++] = '[';
			MxBuffer[MxBufferPos] = '\0';

			MxModeDepth++;
			MxModes[MxModeDepth] = ModeArrayFirst;

			return true;
		}

		bool Serializer::EndArray()
		{
			if (MxModes[MxModeDepth] != ModeArrayFirst && MxModes[MxModeDepth] != ModeArrayNext)  return false;

			if (MxBufferPos + 1 > MxBufferSize)  return false;

			MxBuffer[MxBufferPos++] = ']';
			MxBuffer[MxBufferPos] = '\0';

			MxModeDepth--;

			return true;
		}

		bool Serializer::StartStr(const char *Key)
		{
			if (MxModeDepth + 1 >= MxModes.GetSize() || !InternalAppendNextPrefix(Key, 1))  return false;

			MxBuffer[MxBufferPos++] = '"';
			MxBuffer[MxBufferPos] = '\0';

			MxModeDepth++;
			MxModes[MxModeDepth] = ModeStr;

			return true;
		}

		bool Serializer::EndStr()
		{
			if (MxModes[MxModeDepth] != ModeStr)  return false;

			if (MxBufferPos + 1 > MxBufferSize)  return false;

			MxBuffer[MxBufferPos++] = '"';
			MxBuffer[MxBufferPos] = '\0';

			MxModeDepth--;

			return true;
		}

		bool Serializer::AppendNull(const char *Key)
		{
			if (!InternalAppendNextPrefix(Key, 4))  return false;

			strcpy((char *)MxBuffer + MxBufferPos, "null");
			MxBufferPos += 4;

			return true;
		}

		bool Serializer::AppendBool(const char *Key, const bool Val)
		{
			if (!InternalAppendNextPrefix(Key, 5))  return false;

			if (Val)
			{
				strcpy((char *)MxBuffer + MxBufferPos, "true");
				MxBufferPos += 4;
			}
			else
			{
				strcpy((char *)MxBuffer + MxBufferPos, "false");
				MxBufferPos += 5;
			}

			return true;
		}

		bool Serializer::AppendInt(const char *Key, const std::int64_t Val)
		{
			char TempBuffer[44];
			if (!IntToString(TempBuffer, sizeof(TempBuffer), Val))  return false;

			size_t x = strlen(TempBuffer);
			if (!InternalAppendNextPrefix(Key, x))  return false;

			strcpy((char *)MxBuffer + MxBufferPos, TempBuffer);
			MxBufferPos += x;

			return true;
		}

		bool Serializer::AppendUInt(const char *Key, const std::uint64_t Val)
		{
			char TempBuffer[44];
			if (!IntToString(TempBuffer, sizeof(TempBuffer), Val))  return false;

			size_t x = strlen(TempBuffer);
			if (!InternalAppendNextPrefix(Key, x))  return false;

			strcpy((char *)MxBuffer + MxBufferPos, TempBuffer);
			MxBufferPos += x;

			return true;
		}

		bool Serializer::AppendDouble(const char *Key, const double Val, const size_t Precision)
		{
			char TempBuffer[100];
#if (defined(_WIN32) || defined(WIN32) || defined(_WIN64) || defined(WIN64)) && defined(_MSC_VER) && _MSC_VER < 1900
			_snprintf_s(TempBuffer, sizeof(TempBuffer), _TRUNCATE, "%1.*g", Precision, Val);
			TempBuffer[sizeof(TempBuffer) - 1] = '\0';
#else
			snprintf(TempBuffer, sizeof(TempBuffer), "%1.*g", (int)Precision, Val);
#endif

			size_t x = strlen(TempBuffer);
			if (!InternalAppendNextPrefix(Key, x))  return false;

			strcpy((char *)MxBuffer + MxBufferPos, TempBuffer);
			MxBufferPos += x;

			return true;
		}

		bool Serializer::AppendStr(const char *Key, const char *Val)
		{
			if (Val == NULL)  return false;

			size_t x = CalculateStrSize(Val, false);
			if ((MxModes[MxModeDepth] != ModeStr && !InternalAppendNextPrefix(Key, x)) || (MxModes[MxModeDepth] == ModeStr && MxBufferPos + x > MxBufferSize))  return false;

			InternalAppendStr(Val);

			return true;
		}

		bool Serializer::AppendStr(const char *Key, const char *Val, const size_t Size)
		{
			if (Val == NULL)  return false;

			size_t x = CalculateStrSize(Val, Size, false);
			if ((MxModes[MxModeDepth] != ModeStr && !InternalAppendNextPrefix(Key, x)) || (MxModes[MxModeDepth] == ModeStr && MxBufferPos + x > MxBufferSize))  return false;

			InternalAppendStr(Val, Size);

			return true;
		}

		bool Serializer::Append(const char *Val, size_t Size)
		{
			if (MxBufferPos + Size > MxBufferSize || Val == NULL)  return false;

			while (Size)
			{
				MxBuffer[MxBufferPos++] = *Val++;
				Size--;
			}

			MxBuffer[MxBufferPos] = '\0';

			return true;
		}

		bool Serializer::Finish()
		{
			while (MxModeDepth)
			{
				if (MxModes[MxModeDepth] == ModeObjectFirst || MxModes[MxModeDepth] == ModeObjectNext)
				{
					if (!EndObject())  return false;
				}
				if (MxModes[MxModeDepth] == ModeArrayFirst || MxModes[MxModeDepth] == ModeArrayNext)
				{
					if (!EndArray())  return false;
				}
				else if (MxModes[MxModeDepth] == ModeStr)
				{
					if (!EndStr())  return false;
				}
			}

			return true;
		}

		size_t Serializer::CalculateStrSize(const char *Val, bool AddKeySplitter)
		{
			size_t Result = (MxModes[MxModeDepth] != ModeStr ? 2 : 0);

			for (; *Val; Val++)
			{
				switch (*Val)
				{
					case '"':
					case '\\':
					case '\b':
					case '\f':
					case '\n':
					case '\r':
					case '\t':
					{
						Result++;

						break;
					}

					case '/':
					{
						if (MxEscapeSlashes)  Result++;

						break;
					}
				}

				Result++;
			}

			if (AddKeySplitter)  Result += MxKeySplitterLen;

			return Result;
		}

		size_t Serializer::CalculateStrSize(const char *Val, size_t Size, bool AddKeySplitter)
		{
			size_t Result = (MxModes[MxModeDepth] != ModeStr ? 2 : 0);

			while (Size)
			{
				switch (*Val)
				{
					case '"':
					case '\\':
					case '\b':
					case '\f':
					case '\n':
					case '\r':
					case '\t':
					{
						Result++;

						break;
					}

					case '/':
					{
						if (MxEscapeSlashes)  Result++;

						break;
					}
				}

				Result++;
				Size--;
			}

			if (AddKeySplitter)  Result += MxKeySplitterLen;

			return Result;
		}

		bool Serializer::InternalAppendNextPrefix(const char *Key, size_t ExtraSpace)
		{
			if (MxModes[MxModeDepth] == ModeRootNext || MxModes[MxModeDepth] == ModeStr || ((MxModes[MxModeDepth] == ModeObjectFirst || MxModes[MxModeDepth] == ModeObjectNext) && Key == NULL) || ((MxModes[MxModeDepth] == ModeRootFirst || MxModes[MxModeDepth] == ModeArrayFirst || MxModes[MxModeDepth] == ModeArrayNext) && Key != NULL))  return false;

			size_t x = (MxModes[MxModeDepth] == ModeObjectNext || MxModes[MxModeDepth] == ModeArrayNext ? MxValSplitterLen : 0);
			if (Key != NULL)  x += CalculateStrSize(Key, true);

			if (MxBufferPos + x + ExtraSpace > MxBufferSize)  return false;

			if (MxModes[MxModeDepth] == ModeObjectNext || MxModes[MxModeDepth] == ModeArrayNext)  Append(MxValSplitter, MxValSplitterLen);
			else if (MxModes[MxModeDepth] == ModeObjectFirst)  MxModes[MxModeDepth] = ModeObjectNext;
			else if (MxModes[MxModeDepth] == ModeArrayFirst)  MxModes[MxModeDepth] = ModeArrayNext;
			else if (MxModes[MxModeDepth] == ModeRootFirst)  MxModes[MxModeDepth] = ModeRootNext;

			if (Key != NULL)
			{
				InternalAppendStr(Key);
				Append(MxKeySplitter, MxKeySplitterLen);
			}

			return true;
		}

		// All calculations prior to calling these internal functions must be applied correctly to avoid an overflow.
		void Serializer::InternalAppendStr(const char *Val)
		{
			if (MxModes[MxModeDepth] != ModeStr)  MxBuffer[MxBufferPos++] = '"';

			while (*Val)
			{
				switch (*Val)
				{
					case '"':  MxBuffer[MxBufferPos++] = '\\';  MxBuffer[MxBufferPos++] = '"';  Val++;  break;
					case '\\':  MxBuffer[MxBufferPos++] = '\\';  MxBuffer[MxBufferPos++] = '\\';  Val++;  break;
					case '/':  if (MxEscapeSlashes)  { MxBuffer[MxBufferPos++] = '\\';  MxBuffer[MxBufferPos++] = '/'; }  Val++;  break;
					case '\b':  MxBuffer[MxBufferPos++] = '\\';  MxBuffer[MxBufferPos++] = 'b';  Val++;  break;
					case '\f':  MxBuffer[MxBufferPos++] = '\\';  MxBuffer[MxBufferPos++] = 'f';  Val++;  break;
					case '\n':  MxBuffer[MxBufferPos++] = '\\';  MxBuffer[MxBufferPos++] = 'n';  Val++;  break;
					case '\r':  MxBuffer[MxBufferPos++] = '\\';  MxBuffer[MxBufferPos++] = 'r';  Val++;  break;
					case '\t':  MxBuffer[MxBufferPos++] = '\\';  MxBuffer[MxBufferPos++] = 't';  Val++;  break;
					default:  MxBuffer[MxBufferPos++] = *Val++;  break;
				}
			}

			if (MxModes[MxModeDepth] != ModeStr)  MxBuffer[MxBufferPos++] = '"';
			MxBuffer[MxBufferPos] = '\0';
		}

		void Serializer::InternalAppendStr(const char *Val, size_t Size)
		{
			if (MxModes[MxModeDepth] != ModeStr)  MxBuffer[MxBufferPos++] = '"';

			while (Size)
			{
				switch (*Val)
				{
					case '"':  MxBuffer[MxBufferPos++] = '\\';  MxBuffer[MxBufferPos++] = '"';  Val++;  break;
					case '\\':  MxBuffer[MxBufferPos++] = '\\';  MxBuffer[MxBufferPos++] = '\\';  Val++;  break;
					case '/':  if (MxEscapeSlashes)  { MxBuffer[MxBufferPos++] = '\\';  MxBuffer[MxBufferPos++] = '/'; }  Val++;  break;
					case '\b':  MxBuffer[MxBufferPos++] = '\\';  MxBuffer[MxBufferPos++] = 'b';  Val++;  break;
					case '\f':  MxBuffer[MxBufferPos++] = '\\';  MxBuffer[MxBufferPos++] = 'f';  Val++;  break;
					case '\n':  MxBuffer[MxBufferPos++] = '\\';  MxBuffer[MxBufferPos++] = 'n';  Val++;  break;
					case '\r':  MxBuffer[MxBufferPos++] = '\\';  MxBuffer[MxBufferPos++] = 'r';  Val++;  break;
					case '\t':  MxBuffer[MxBufferPos++] = '\\';  MxBuffer[MxBufferPos++] = 't';  Val++;  break;
					default:  MxBuffer[MxBufferPos++] = *Val++;  break;
				}

				Size--;
			}

			if (MxModes[MxModeDepth] != ModeStr)  MxBuffer[MxBufferPos++] = '"';
			MxBuffer[MxBufferPos] = '\0';
		}

		// Swiped and slightly modified from Int::ToString().
		bool Serializer::IntToString(char *Result, size_t Size, std::uint64_t Num)
		{
			if (Size < 2)  return false;

			size_t x = Size;

			Result[--x] = '\0';
			if (!Num)  Result[--x] = '0';
			else
			{
				while (Num && x)
				{
					Result[--x] = (char)(Num % 10) + '0';
					Num /= 10;
				}

				if (Num)  return false;
			}

			memmove(Result, Result + x, Size - x);

			return true;
		}

		bool Serializer::IntToString(char *Result, size_t Size, std::int64_t Num)
		{
			if (Num >= 0)  return IntToString(Result, Size, (std::uint64_t)Num);

			if (Size < 2)  return false;
			Result[0] = '-';

			return IntToString(Result + 1, Size - 1, (std::uint64_t)-Num);
		}
	}
}
