// Cross-platform JSON serialization class.
// (C) 2021 CubicleSoft.  All Rights Reserved.

#ifndef CUBICLESOFT_JSON_SERIALIZER
#define CUBICLESOFT_JSON_SERIALIZER

#include <cstdint>
#include <cstddef>
#include <cstdlib>
#include <cstring>

#include "../templates/static_vector.h"

namespace CubicleSoft
{
	namespace JSON
	{
		class Serializer
		{
		public:
			enum ModeType
			{
				ModeRootFirst,
				ModeRootNext,
				ModeObjectFirst,
				ModeObjectNext,
				ModeArrayFirst,
				ModeArrayNext,
				ModeStr
			};

			Serializer(const bool EscapeSlashes = false, const char *KeySplitter = ": ", const char *ValSplitter = ", ", size_t MaxDepth = 512);

			void Reset();

			void SetBuffer(std::uint8_t *Buffer, size_t BufferSize);
			inline std::uint8_t *GetBuffer() const { return MxBuffer; }
			inline size_t GetCurrPos() const { return MxBufferPos; }
			inline size_t GetBufferSize() const { return MxBufferSize; }
			inline void ResetPos() { MxBufferPos = 0; }

			inline ModeType GetCurrMode() { return MxModes[MxModeDepth]; }

			inline void SetEscapeSlashes(const bool EscapeSlashes) { MxEscapeSlashes = EscapeSlashes; }
			inline void SetKeySplitter(const char *KeySplitter) { MxKeySplitter = KeySplitter; MxKeySplitterLen = strlen(KeySplitter); }
			inline void SetValSplitter(const char *ValSplitter) { MxValSplitter = ValSplitter; MxValSplitterLen = strlen(ValSplitter); }

			inline bool IsKeyRequired() { return (MxModes[MxModeDepth] == ModeObjectFirst || MxModes[MxModeDepth] == ModeObjectNext); }

			bool StartObject(const char *Key = NULL);
			bool EndObject();

			bool StartArray(const char *Key = NULL);
			bool EndArray();

			bool StartStr(const char *Key = NULL);
			bool EndStr();

			bool AppendNull(const char *Key = NULL);
			bool AppendBool(const char *Key, const bool Val);
			bool AppendInt(const char *Key, const std::int64_t Val);
			bool AppendUInt(const char *Key, const std::uint64_t Val);
			bool AppendDouble(const char *Key, const double Val, const size_t Precision = 100);
			bool AppendStr(const char *Key, const char *Val);
			bool AppendStr(const char *Key, const char *Val, const size_t Size);

			inline bool AppendBool(const bool Val)  { return AppendBool(NULL, Val); }
			inline bool AppendInt(const std::int64_t Val)  { return AppendInt(NULL, Val); }
			inline bool AppendUInt(const std::uint64_t Val)  { return AppendUInt(NULL, Val); }
			inline bool AppendDouble(const double Val, const size_t Precision = 16)  { return AppendDouble(NULL, Val, Precision); }
			inline bool AppendStr(const char *Val)  { return AppendStr(NULL, Val); }
			inline bool AppendStr(const char *Val, const size_t Size)  { return AppendStr(NULL, Val, Size); }

			// Intended for appending arbitrary content (e.g. whitespace).
			bool Append(const char *Val, size_t Size);
			inline bool Append(const char *Val)  { return Append(Val, strlen(Val)); };

			bool Finish();

			// Calculate the final size of a string including quotes.
			size_t CalculateStrSize(const char *Val, bool AddKeySplitter = false);
			size_t CalculateStrSize(const char *Val, size_t Size, bool AddKeySplitter = false);

		private:
			bool InternalAppendNextPrefix(const char *Key, size_t ExtraSpace);
			void InternalAppendStr(const char *Val);
			void InternalAppendStr(const char *Val, size_t Size);
			static bool IntToString(char *Result, size_t Size, std::uint64_t Num);
			static bool IntToString(char *Result, size_t Size, std::int64_t Num);

			StaticVector<ModeType> MxModes;
			size_t MxModeDepth;

			std::uint8_t *MxBuffer;
			size_t MxBufferPos, MxBufferSize;

			bool MxEscapeSlashes;
			const char *MxKeySplitter, *MxValSplitter;
			size_t MxKeySplitterLen, MxValSplitterLen;
		};
	}
}

#endif
