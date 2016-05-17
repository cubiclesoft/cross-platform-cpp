// Cross-platform thread local storage memory allocation class.  Built for short-lived, in-thread memory allocations.
// (C) 2013 CubicleSoft.  All Rights Reserved.

#include "sync_tls.h"
#include "../templates/fast_find_replace.h"
#include <cstdlib>
#include <cstring>
#include <new>

namespace CubicleSoft
{
	namespace Sync
	{
#if defined(_WIN32) || defined(WIN32) || defined(_WIN64) || defined(WIN64)
		// Windows.
		TLS::TLS()
		{
			MxTlsIndex = ::TlsAlloc();
		}

		TLS::~TLS()
		{
			::TlsFree(MxTlsIndex);
		}

		bool TLS::SetMainPtr(StaticVector<Queue<char>> *MainPtr)
		{
			return (::TlsSetValue(MxTlsIndex, MainPtr) != 0);
		}

		StaticVector<Queue<char>> *TLS::GetMainPtr()
		{
			return (StaticVector<Queue<char>> *)::TlsGetValue(MxTlsIndex);
		}
#else
		// POSIX pthreads.
		TLS::TLS()
		{
			pthread_key_create(&MxKey, NULL);
		}

		TLS::~TLS()
		{
			pthread_key_delete(MxKey);
		}

		bool TLS::SetMainPtr(StaticVector<Queue<char>> *MainPtr)
		{
			return (pthread_setspecific(MxKey, MainPtr) == 0);
		}

		StaticVector<Queue<char>> *TLS::GetMainPtr()
		{
			return (StaticVector<Queue<char>> *)pthread_getspecific(MxKey);
		}
#endif

		// All platforms.
		bool TLS::ThreadInit(size_t MaxCacheBits)
		{
			StaticVector<Queue<char>> *MainPtr = GetMainPtr();
			if (MainPtr != NULL)  return true;

			return SetMainPtr(new StaticVector<Queue<char>>(MaxCacheBits));
		}

		void *TLS::malloc(size_t Size)
		{
			if (Size == 0)  return NULL;

			StaticVector<Queue<char>> *MainPtr = GetMainPtr();
			if (MainPtr == NULL)  return NULL;

			void *Data;
			size_t Pos = NormalizeBitPosition(Size);
			Size++;
			if (MainPtr->GetSize() <= Pos)
			{
				Data = ::malloc(Size);
				Data = (std::uint8_t *)Data + 1;
			}
			else
			{
				QueueNode<char> *Node = (*MainPtr)[Pos].Shift();
				if (Node == NULL)
				{
					if (Size < sizeof(QueueNode<char>))  Size = sizeof(QueueNode<char>);
					Data = ::malloc(Size);
					if (Data == NULL)  return NULL;

					Node = (QueueNode<char> *)Data;
				}

				Data = ((std::uint8_t *)Node) + 1;
			}

			// Store the position.  Safe for QueueNode since data storage is after the NextNode pointer.
			((std::uint8_t *)Data)[-1] = (std::uint8_t)Pos;

			return Data;
		}

		void *TLS::realloc(void *Data, size_t NewSize, bool Cache)
		{
			if (NewSize == 0)
			{
				if (Data != NULL)  free(Data, Cache);

				return NULL;
			}

			if (Data == NULL)  return malloc(NewSize);

			// See if it fits in the current size.
			size_t Pos = NormalizeBitPosition(NewSize);
			size_t Pos2 = (size_t)((std::uint8_t *)Data)[-1];
			if (Pos <= Pos2)  return Data;

			StaticVector<Queue<char>> *MainPtr = GetMainPtr();
			if (MainPtr == NULL)  return NULL;

			void *Data2;
			if (MainPtr->GetSize() <= Pos && MainPtr->GetSize() <= Pos2)
			{
				Data2 = ::realloc(((std::uint8_t *)Data) - 1, 1 + NewSize);
				Data2 = (std::uint8_t *)Data2 + 1;
			}
			else
			{
				// Allocate data.
				Data2 = malloc(NewSize);

				// Copy the data.
				memcpy(Data2, Data, (1 << Pos2));

				// Free the previous object.
				free(Data, Cache);
			}

			return Data2;
		}

		void *TLS::dup_malloc(void *Data, bool Cache)
		{
			if (Data == NULL)  return NULL;

			// Allocate the appropriate size buffer.
			size_t Size = (size_t)(1 << ((size_t)((std::uint8_t *)Data)[-1]));
			void *Data2 = ::malloc(Size);
			if (Data2 == NULL)  return NULL;

			// Copy the data.
			memcpy(Data2, Data, Size);

			// Free the previous object.
			free(Data, Cache);

			return Data2;
		}

		void TLS::free(void *Data, bool Cache)
		{
			if (Data == NULL)  return;

			StaticVector<Queue<char>> *MainPtr = GetMainPtr();
			if (MainPtr == NULL)  return;

			size_t Pos = (size_t)((std::uint8_t *)Data)[-1];
			if (MainPtr->GetSize() <= Pos)  ::free(((std::uint8_t *)Data) - 1);
			else
			{
				QueueNode<char> *Node = (QueueNode<char> *)(((std::uint8_t *)Data) - 1);

				if (!Cache)  ::free(Node);
				else
				{
					// Placement new.  Instantiates QueueNode.
					Node = new(Node) QueueNode<char>;

					(*MainPtr)[Pos].Push(Node);
				}
			}
		}

		bool TLS::GetBucketInfo(size_t Num, size_t &Nodes, size_t &Size)
		{
			StaticVector<Queue<char>> *MainPtr = GetMainPtr();
			if (MainPtr == NULL)  return false;

			if (Num >= MainPtr->GetSize())  return false;
			else
			{
				Nodes = (*MainPtr)[Num].GetSize();
				Size = (size_t)(1 << Num);
				if (Size < sizeof(QueueNode<char>))  Size = sizeof(QueueNode<char>);
				Size *= Nodes;
			}

			return true;
		}

		bool TLS::ThreadEnd()
		{
			StaticVector<Queue<char>> *MainPtr = GetMainPtr();
			if (MainPtr == NULL)  return false;

			SetMainPtr(NULL);

			// Free all cached data.
			size_t y = MainPtr->GetSize();
			Queue<char> *RawData = MainPtr->RawData();
			QueueNode<char> *Node;
			for (size_t x = 0; x < y; x++)
			{
				while ((Node = RawData[x].Shift()) != NULL)
				{
					::free(Node);
				}
			}

			delete MainPtr;

			return true;
		}

		size_t TLS::NormalizeBitPosition(size_t &Size)
		{
			size_t Pos = 3;

			while (((size_t)1 << Pos) < Size)  Pos++;
			Size = ((size_t)1 << Pos);

			return Pos;
		}


		TLS::AutoFree::AutoFree(TLS *TLSPtr, void *Data)
		{
			MxTLS = TLSPtr;
			MxData = Data;
		}

		TLS::AutoFree::~AutoFree()
		{
			MxTLS->free(MxData);
		}


		TLS::MixedVar::MixedVar(TLS *TLSPtr) : MxMode(TMV_None), MxInt(0), MxDouble(0.0), MxStr(NULL), MxStrPos(0), MxTLS(TLSPtr)
		{
		}

		TLS::MixedVar::~MixedVar()
		{
			if (MxStr != NULL)  MxTLS->free(MxStr);
		}

		// Copy constructor.
		TLS::MixedVar::MixedVar(const TLS::MixedVar &TempVar)
		{
			MxTLS = TempVar.MxTLS;

			if (TempVar.MxStr != NULL)  SetData(TempVar.MxStr, TempVar.MxStrPos);
			else
			{
				MxStr = NULL;
				MxStrPos = 0;
			}

			MxMode = TempVar.MxMode;
			MxInt = TempVar.MxInt;
			MxDouble = TempVar.MxDouble;
		}

		// Assignment operator.
		TLS::MixedVar &TLS::MixedVar::operator=(const TLS::MixedVar &TempVar)
		{
			if (this != &TempVar)
			{
				MxTLS = TempVar.MxTLS;

				if (TempVar.MxStr != NULL)  SetData(TempVar.MxStr, TempVar.MxStrPos);
				else
				{
					MxTLS->free(MxStr);

					MxStr = NULL;
					MxStrPos = 0;
				}

				MxMode = TempVar.MxMode;
				MxInt = TempVar.MxInt;
				MxDouble = TempVar.MxDouble;
			}

			return *this;
		}

		void TLS::MixedVar::SetData(const char *str, size_t size)
		{
			MxMode = TMV_Str;
			if (MxStr != NULL)  MxTLS->free(MxStr);
			MxStr = (char *)MxTLS->malloc(size + 1);
			memcpy(MxStr, str, size);
			MxStrPos = size;
			MxStr[MxStrPos] = '\0';
		}

		void TLS::MixedVar::SetStr(const char *str)
		{
			SetData(str, strlen(str));
		}

		void TLS::MixedVar::PrependData(const char *str, size_t size)
		{
			char *str2 = (char *)MxTLS->malloc(size + MxStrPos + 1);
			memcpy(str2, str, size);
			memcpy(str2 + size, MxStr, MxStrPos);
			MxTLS->free(MxStr);
			MxStr = str2;
			MxStrPos += size;
			MxStr[MxStrPos] = '\0';
		}

		void TLS::MixedVar::PrependStr(const char *str)
		{
			PrependData(str, strlen(str));
		}

		void TLS::MixedVar::AppendData(const char *str, size_t size)
		{
			MxStr = (char *)MxTLS->realloc(MxStr, MxStrPos + size + 1);
			memcpy(MxStr + MxStrPos, str, size);
			MxStrPos += size;
			MxStr[MxStrPos] = '\0';
		}

		void TLS::MixedVar::AppendStr(const char *str)
		{
			AppendData(str, strlen(str));
		}

		void TLS::MixedVar::AppendChar(char chr)
		{
			MxStr = (char *)MxTLS->realloc(MxStr, MxStrPos + 2);
			MxStr[MxStrPos++] = chr;
			MxStr[MxStrPos] = '\0';
		}

		void TLS::MixedVar::AppendMissingChar(char chr)
		{
			if (!MxStrPos || MxStr[MxStrPos - 1] != chr)  AppendChar(chr);
		}

		void TLS::MixedVar::SetSize(size_t pos)
		{
			if (MxStrPos < pos)  MxStr = (char *)MxTLS->realloc(MxStr, pos + 1);

			MxStrPos = pos;
			MxStr[MxStrPos] = '\0';
		}

		size_t TLS::MixedVar::ReplaceData(const char *src, size_t srcsize, const char *dest, size_t destsize)
		{
			size_t Num;

			if (srcsize < destsize)
			{
				char *Result;
				size_t ResultSize;

				Num = FastReplaceAlloc<char, TLS>::ReplaceAll(Result, ResultSize, MxStr, MxStrPos, src, srcsize, dest, destsize, MxTLS);

				MxTLS->free(MxStr);
				MxStr = Result;
				MxStrPos = ResultSize;
			}
			else
			{
				Num = FastReplaceAlloc<char, TLS>::StaticReplaceAll(MxStr, MxStrPos, MxStr, MxStrPos, src, srcsize, dest, destsize, MxTLS);
			}

			MxStr[MxStrPos] = '\0';

			return Num;
		}

		size_t TLS::MixedVar::ReplaceStr(const char *src, const char *dest)
		{
			return ReplaceData(src, strlen(src), dest, strlen(dest));
		}
	}
}
