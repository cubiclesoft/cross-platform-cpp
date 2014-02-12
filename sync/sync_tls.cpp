// Cross-platform thread local storage memory allocation class.  Built for short-lived, in-thread memory allocations.
// (C) 2013 CubicleSoft.  All Rights Reserved.

#include "sync_tls.h"
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
	}
}
