// Ordered hash map with integer and string keys with detachable nodes.
// (C) 2014 CubicleSoft.  All Rights Reserved.

#ifndef CUBICLESOFT_DETACHABLE_ORDEREDHASH
#define CUBICLESOFT_DETACHABLE_ORDEREDHASH

#include <cstdint>
#include <cstddef>
#include <cstring>

namespace CubicleSoft
{
	template <class T>
	class OrderedHash;
	template <class T>
	class OrderedHashNoCopy;

	template <class T>
	class OrderedHashNode
	{
		friend class OrderedHash<T>;
		friend class OrderedHashNoCopy<T>;

	public:
		OrderedHashNode() : PrevHashNode(NULL), NextHashNode(NULL), PrevListNode(NULL), NextListNode(NULL), HashKey(0), IntKey(0), StrKey(NULL)
		{
		}

		~OrderedHashNode()
		{
			if (StrKey != NULL)  delete[] StrKey;
		}

		// Only copies the data.  Pointers will still need to be set.
		OrderedHashNode(const OrderedHashNode<T> &TempNode)
		{
			HashKey = TempNode.HashKey;
			IntKey = TempNode.IntKey;

			if (TempNode.StrKey == NULL)  StrKey = NULL;
			else
			{
				StrKey = new char[IntKey];
				std::memcpy(StrKey, TempNode.StrKey, IntKey);
			}

			Value = TempNode.Value;
		}

		OrderedHashNode<T> &operator=(const OrderedHashNode<T> &TempNode)
		{
			if (&TempNode != this)
			{
				HashKey = TempNode.HashKey;
				IntKey = TempNode.IntKey;

				if (StrKey != NULL)  delete[] StrKey;

				if (TempNode.StrKey == NULL)  StrKey = NULL;
				else
				{
					StrKey = new char[IntKey];
					std::memcpy(StrKey, TempNode.StrKey, IntKey);
				}

				Value = TempNode.Value;
			}

			return *this;
		}

		inline OrderedHashNode<T> *PrevHash()  { return PrevHashNode; }
		inline OrderedHashNode<T> *NextHash()  { return NextHashNode; }
		inline OrderedHashNode<T> *PrevList()  { return PrevListNode; }
		inline OrderedHashNode<T> *NextList()  { return NextListNode; }
		inline char *GetStrKey() { return StrKey; }
		inline std::int64_t GetIntKey() { return IntKey; }

		// FreeStrKey will delete[] an existing string in this node.
		// If you manage your own memory (see SetStrKey), be sure to set it to false.
		void SetIntKey(const std::int64_t NewIntKey, const bool FreeStrKey)
		{
			HashKey = 0;
			IntKey = NewIntKey;
			if (FreeStrKey && StrKey != NULL)  delete[] StrKey;
			StrKey = NULL;
		}

		// When CopyStr is false (i.e. managing your own memory),
		// SetStrKey() only copies the pointer and you are expected to
		// later call SetStrKey(NULL, 0, false) on the node.
		// Best used with OrderedHashNoCopy to avoid accidental
		// assignment/copy constructor issues.
		void SetStrKey(const char *NewStrKey, const size_t NewStrLen, const bool CopyStr)
		{
			HashKey = 0;

			if (!CopyStr)  StrKey = (char *)NewStrKey;
			else
			{
				if (StrKey == NULL)  StrKey = new char[NewStrLen];
				else if (IntKey < (std::int64_t)NewStrLen)
				{
					delete[] StrKey;
					StrKey = new char[NewStrLen];
				}

				std::memcpy(StrKey, NewStrKey, NewStrLen);
			}

			IntKey = (std::int64_t)NewStrLen;
		}

	private:
		OrderedHashNode<T> *PrevHashNode;
		OrderedHashNode<T> *NextHashNode;
		OrderedHashNode<T> *PrevListNode;
	public:
		// NextListNode is public to allow temporary chains of detached nodes to be forged.
		OrderedHashNode<T> *NextListNode;

	private:
		std::uint64_t HashKey;

		std::int64_t IntKey;
		char *StrKey;

	public:
		T Value;
	};

	class OrderedHashUtil
	{
	public:
		static const size_t Primes[31];
		static size_t GetDJBX33XHashKey(const std::uint8_t *Str, size_t Size, size_t InitVal);
		static std::uint64_t GetSipHashKey(const std::uint8_t *Str, size_t Size, std::uint64_t Key1, std::uint64_t Key2, size_t cRounds, size_t dRounds);
	};

	// OrderedHash.  An ordered hash.
	#include "detachable_ordered_hash_util.h"

	// OrderedHashNoCopy.  An ordered hash with a private copy constructor and assignment operator.
	#define CUBICLESOFT_DETACHABLE_ORDEREDHASH_NOCOPYASSIGN
	#include "detachable_ordered_hash_util.h"
	#undef CUBICLESOFT_DETACHABLE_ORDEREDHASH_NOCOPYASSIGN
}

#endif