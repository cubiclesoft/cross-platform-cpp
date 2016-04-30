// Ordered hash map with integer and string keys with detachable nodes.
// (C) 2014 CubicleSoft.  All Rights Reserved.

// NOTE:  This file is intended to be included from 'detachable_ordered_hash.h'.

// Implements an ordered hash that grows dynamically and uses integer and string keys.
template <class T>
#ifdef CUBICLESOFT_DETACHABLE_ORDEREDHASH_NOCOPYASSIGN
class OrderedHashNoCopy
#else
class OrderedHash
#endif
{
public:
	// Implements djb2 (DJBX33X).
	// WARNING:  This algorithm is weak security-wise!
	// https://www.youtube.com/watch?v=R2Cq3CLI6H8
	// https://www.youtube.com/watch?v=wGYj8fhhUVA
	// For much better security with a slight performance reduction, use the other constructor, which implements SipHash.
#ifdef CUBICLESOFT_DETACHABLE_ORDEREDHASH_NOCOPYASSIGN
	OrderedHashNoCopy
#else
	OrderedHash
#endif
		(size_t EstimatedSize = 23, std::uint64_t HashKey = 5381) : UseSipHash(false), Key1(HashKey), Key2(0), HashNodes(NULL), HashSize(0), NextPrimePos(0), FirstListNode(NULL), LastListNode(NULL), NumListNodes(0)
	{
		ResizeHash(EstimatedSize);
	}

	// Keys are securely hashed via SipHash-2-4.
	// Assumes good (CSPRNG generated) inputs for HashKey1 and HashKey2.
#ifdef CUBICLESOFT_DETACHABLE_ORDEREDHASH_NOCOPYASSIGN
	OrderedHashNoCopy
#else
	OrderedHash
#endif
		(size_t EstimatedSize, std::uint64_t HashKey1, std::uint64_t HashKey2) : UseSipHash(true), Key1(HashKey1), Key2(HashKey2), HashNodes(NULL), HashSize(0), NextPrimePos(0), FirstListNode(NULL), LastListNode(NULL), NumListNodes(0)
	{
		ResizeHash(EstimatedSize);
	}

#ifdef CUBICLESOFT_DETACHABLE_ORDEREDHASH_NOCOPYASSIGN
	~OrderedHashNoCopy()
#else
	~OrderedHash()
#endif
	{
		if (HashNodes != NULL)
		{
			Empty();

			delete[] HashNodes;
		}
	}

#ifdef CUBICLESOFT_DETACHABLE_ORDEREDHASH_NOCOPYASSIGN
	OrderedHashNoCopy(const OrderedHashNoCopy<T> &TempHash);
	OrderedHashNoCopy<T> &operator=(const OrderedHashNoCopy<T> &TempHash);
#else
	OrderedHash(const OrderedHash<T> &TempHash)
	{
		size_t x;

		UseSipHash = TempHash.UseSipHash;
		Key1 = TempHash.Key1;
		Key2 = TempHash.Key2;

		HashSize = TempHash.HashSize;
		HashNodes = new OrderedHashNode<T> *[HashSize];
		for (x = 0; x < HashSize; x++)  HashNodes[x] = NULL;
		FirstListNode = NULL;
		LastListNode = NULL;
		OrderedHashNode<T> *Node = TempHash.FirstListNode;
		OrderedHashNode<T> *Node2, *Node3;
		while (Node != NULL)
		{
			// Attach each cloned node to the end of the list.
			Node2 = new OrderedHashNode<T>(*Node);
			Node2->NextListNode = NULL;

			if (FirstListNode == NULL)  FirstListNode = Node2;
			else
			{
				Node2->PrevListNode = LastListNode;
				LastListNode->NextListNode = Node2;
			}

			LastListNode = Node2;

			// Attach cloned node to hash.
			Node2->PrevHashNode = NULL;
			x = (size_t)(Node2->HashKey % HashSize);
			Node2->NextHashNode = HashNodes[x];
			if (HashNodes[x] != NULL)  HashNodes[x]->PrevHashNode = Node2;

			Node = Node->NextListNode;
		}

		NextPrimePos = TempHash.NextPrimePos;
		NumListNodes = TempHash.NumListNodes;
	}

	OrderedHash<T> &operator=(const OrderedHash<T> &TempHash)
	{
		if (&TempHash != this)
		{
			size_t x;

			UseSipHash = TempHash.UseSipHash;
			Key1 = TempHash.Key1;
			Key2 = TempHash.Key2;

			if (HashNodes != NULL)  delete[] HashNodes;
			OrderedHashNode<T> *Node = FirstListNode;
			OrderedHashNode<T> *Node2, *Node3;
			while (Node != NULL)
			{
				Node2 = Node->NextListNode;
				delete Node;

				Node = Node2;
			}

			HashSize = TempHash.HashSize;
			HashNodes = new OrderedHashNode<T> *[HashSize];
			for (x = 0; x < HashSize; x++)  HashNodes[x] = NULL;
			FirstListNode = NULL;
			LastListNode = NULL;
			Node = TempHash.FirstListNode;
			while (Node != NULL)
			{
				// Attach each cloned node to the end of the list.
				Node2 = new OrderedHashNode<T>(*Node);
				Node2->NextListNode = NULL;

				if (FirstListNode == NULL)  FirstListNode = Node2;
				else
				{
					Node2->PrevListNode = LastListNode;
					LastListNode->NextListNode = Node2;
				}

				LastListNode = Node2;

				// Attach cloned node to hash.
				Node2->PrevHashNode = NULL;
				x = (size_t)(Node2->HashKey % HashSize);
				Node2->NextHashNode = HashNodes[x];
				if (HashNodes[x] != NULL)  HashNodes[x]->PrevHashNode = Node2;

				Node = Node->NextListNode;
			}

			NextPrimePos = TempHash.NextPrimePos;
			NumListNodes = TempHash.NumListNodes;
		}

		return *this;
	}
#endif

	inline OrderedHashNode<T> *Push(const std::int64_t IntKey, const T &Value)
	{
		return InsertBefore(NULL, IntKey, Value);
	}

	inline OrderedHashNode<T> *Push(const char *StrKey, const size_t StrLen, const T &Value)
	{
		return InsertBefore(NULL, StrKey, StrLen, Value);
	}

	// Only use with detached nodes.
	inline OrderedHashNode<T> *Push(OrderedHashNode<T> *Node)
	{
		return InsertBefore(NULL, Node);
	}

	OrderedHashNode<T> *Pop(bool ResetHashKey)
	{
		OrderedHashNode<T> *Node = LastListNode;
		Detach(Node, ResetHashKey);

		return Node;
	}

	inline OrderedHashNode<T> *Unshift(const std::int64_t IntKey, const T &Value)
	{
		return InsertAfter(NULL, IntKey, Value);
	}

	inline OrderedHashNode<T> *Unshift(const char *StrKey, const size_t StrLen, const T &Value)
	{
		return InsertAfter(NULL, StrKey, StrLen, Value);
	}

	// Only use with detached nodes.
	inline OrderedHashNode<T> *Unshift(OrderedHashNode<T> *Node)
	{
		return InsertAfter(NULL, Node);
	}

	OrderedHashNode<T> *Shift(bool ResetHashKey)
	{
		OrderedHashNode<T> *Node = FirstListNode;
		Detach(Node, ResetHashKey);

		return Node;
	}

	static inline OrderedHashNode<T> *CreateNode()
	{
		return new OrderedHashNode<T>;
	}

	static inline OrderedHashNode<T> *CreateNode(const std::int64_t IntKey, const T &Value)
	{
		OrderedHashNode<T> *Node = new OrderedHashNode<T>;
		Node->IntKey = IntKey;
		Node->Value = Value;

		return Node;
	}

	static inline OrderedHashNode<T> *CreateNode(const char *StrKey, const size_t StrLen, const T &Value)
	{
		OrderedHashNode<T> *Node = new OrderedHashNode<T>;

		Node->IntKey = (std::int64_t)StrLen;
		Node->StrKey = new char[StrLen];
		memcpy(Node->StrKey, StrKey, StrLen);

		Node->Value = Value;

		return Node;
	}

	OrderedHashNode<T> *InsertBefore(OrderedHashNode<T> *Next, const std::int64_t IntKey, const T &Value)
	{
		OrderedHashNode<T> *Node = CreateNode(IntKey, Value);
		OrderedHashNode<T> *Node2 = InsertBefore(Next, Node);
		if (Node2 == NULL)  delete Node;

		return Node2;
	}

	OrderedHashNode<T> *InsertBefore(OrderedHashNode<T> *Next, const char *StrKey, const size_t StrLen, const T &Value)
	{
		OrderedHashNode<T> *Node = CreateNode(StrKey, StrLen, Value);
		OrderedHashNode<T> *Node2 = InsertBefore(Next, Node);
		if (Node2 == NULL)  delete Node;

		return Node2;
	}

	// Only use with detached nodes.
	OrderedHashNode<T> *InsertBefore(OrderedHashNode<T> *Next, OrderedHashNode<T> *Node)
	{
		if (Node->PrevHashNode != NULL || Node->NextHashNode != NULL)  return NULL;

		// Calculate and cache the hash key.
		if (Node->HashKey == 0)
		{
			if (Node->StrKey != NULL)  Node->HashKey = GetHashKey((std::uint8_t *)Node->StrKey, (size_t)Node->IntKey);
			else  Node->HashKey = GetHashKey((std::uint8_t *)&(Node->IntKey), sizeof(std::int64_t));
		}

		// Unable to have two of the same key in the list.
		if (Find(Node) != NULL)  return NULL;

		// Insert into the list.
		if (Next == NULL)
		{
			Node->PrevListNode = LastListNode;
			if (LastListNode != NULL)  LastListNode->NextListNode = Node;
			LastListNode = Node;
			if (FirstListNode == NULL)  FirstListNode = LastListNode;
		}
		else
		{
			Node->NextListNode = Next;
			Node->PrevListNode = Next->PrevListNode;
			if (Node->PrevListNode != NULL)  Node->PrevListNode->NextListNode = Node;
			Next->PrevListNode = Node;
			if (Next == FirstListNode)  FirstListNode = Node;
		}
		NumListNodes++;

		// Resize the hash or insert the node into the hash.
		if (NextPrimePos < sizeof(OrderedHashUtil::Primes) / sizeof(size_t) && NumListNodes >= OrderedHashUtil::Primes[NextPrimePos])  ResizeHash(OrderedHashUtil::Primes[NextPrimePos]);
		else
		{
			size_t x = (size_t)(Node->HashKey % (std::uint64_t)HashSize);
			Node->NextHashNode = HashNodes[x];
			if (HashNodes[x] != NULL)  HashNodes[x]->PrevHashNode = Node;
			HashNodes[x] = Node;
		}

		return Node;
	}

	OrderedHashNode<T> *InsertAfter(OrderedHashNode<T> *Prev, const std::int64_t IntKey, const T &Value)
	{
		OrderedHashNode<T> *Node = CreateNode(IntKey, Value);
		OrderedHashNode<T> *Node2 = InsertAfter(Prev, Node);
		if (Node2 == NULL)  delete Node;

		return Node2;
	}

	OrderedHashNode<T> *InsertAfter(OrderedHashNode<T> *Prev, const char *StrKey, const size_t StrLen, const T &Value)
	{
		OrderedHashNode<T> *Node = CreateNode(StrKey, StrLen, Value);
		OrderedHashNode<T> *Node2 = InsertAfter(Prev, Node);
		if (Node2 == NULL)  delete Node;

		return Node2;
	}

	// Only use with detached nodes.
	OrderedHashNode<T> *InsertAfter(OrderedHashNode<T> *Prev, OrderedHashNode<T> *Node)
	{
		if (Node->PrevHashNode != NULL || Node->NextHashNode != NULL)  return NULL;

		// Calculate and cache the hash key.
		if (Node->HashKey == 0)
		{
			if (Node->StrKey != NULL)  Node->HashKey = GetHashKey((std::uint8_t *)Node->StrKey, (size_t)Node->IntKey);
			else  Node->HashKey = GetHashKey((std::uint8_t *)&(Node->IntKey), sizeof(std::int64_t));
		}

		// Unable to have two of the same key in the list.
		if (Find(Node) != NULL)  return NULL;

		// Insert into the list.
		if (Prev == NULL)
		{
			Node->NextListNode = FirstListNode;
			if (FirstListNode != NULL)  FirstListNode->PrevListNode = Node;
			FirstListNode = Node;
			if (LastListNode == NULL)  LastListNode = FirstListNode;
		}
		else
		{
			Node->PrevListNode = Prev;
			Node->NextListNode = Prev->NextListNode;
			if (Node->NextListNode != NULL)  Node->NextListNode->PrevListNode = Node;
			Prev->NextListNode = Node;
			if (Prev == LastListNode)  LastListNode = Node;
		}
		NumListNodes++;

		// Resize the hash or insert the node into the hash.
		if (NextPrimePos < sizeof(OrderedHashUtil::Primes) / sizeof(size_t) && NumListNodes >= OrderedHashUtil::Primes[NextPrimePos])  ResizeHash(OrderedHashUtil::Primes[NextPrimePos]);
		else
		{
			size_t x = (size_t)(Node->HashKey % (std::uint64_t)HashSize);
			Node->NextHashNode = HashNodes[x];
			if (HashNodes[x] != NULL)  HashNodes[x]->PrevHashNode = Node;
			HashNodes[x] = Node;
		}

		return Node;
	}

	// ResetHashKey forces the hash key to be recalculated if the node is attached later.
	bool Detach(OrderedHashNode<T> *Node, bool ResetHashKey)
	{
		if (Node == NULL)  return false;

		// Disconnect the first/last node (if necessary).
		if (Node == FirstListNode)
		{
			FirstListNode = FirstListNode->NextListNode;
			if (FirstListNode != NULL)  FirstListNode->PrevListNode = NULL;
			else  LastListNode = NULL;
		}
		else if (Node == LastListNode)
		{
			LastListNode = LastListNode->PrevListNode;
			if (LastListNode != NULL)  LastListNode->NextListNode = NULL;
			else  FirstListNode = NULL;
		}
		else
		{
			if (Node->NextListNode != NULL)  Node->NextListNode->PrevListNode = Node->PrevListNode;
			if (Node->PrevListNode != NULL)  Node->PrevListNode->NextListNode = Node->NextListNode;
		}
		Node->NextListNode = NULL;
		Node->PrevListNode = NULL;

		NumListNodes--;

		// Detach the hash node.
		size_t x = (size_t)(Node->HashKey % (std::uint64_t)HashSize);
		if (Node->PrevHashNode != NULL)  Node->PrevHashNode->NextHashNode = Node->NextHashNode;
		else if (HashNodes[x] == Node)  HashNodes[x] = Node->NextHashNode;

		if (Node->NextHashNode != NULL)  Node->NextHashNode->PrevHashNode = Node->PrevHashNode;

		Node->PrevHashNode = NULL;
		Node->NextHashNode = NULL;

		if (ResetHashKey)  Node->HashKey = 0;

		return true;
	}

	bool Remove(OrderedHashNode<T> *Node)
	{
		if (!Detach(Node, false))  return false;

		delete Node;

		return true;
	}

	void Empty(bool DeleteNodes = true, bool ResetHashKey = false)
	{
		OrderedHashNode<T> *Node = FirstListNode;
		OrderedHashNode<T> *Node2;
		while (Node != NULL)
		{
			Node2 = Node->NextListNode;
			if (DeleteNodes)  delete Node;
			else
			{
				// Detach the node's hash pointers and optionally reset the hash key.
				Node->PrevHashNode = NULL;
				Node->NextHashNode = NULL;

				if (ResetHashKey)  Node->HashKey = 0;
			}

			Node = Node2;
		}

		size_t x;

		for (x = 0; x < HashSize; x++)  HashNodes[x] = NULL;

		FirstListNode = NULL;
		LastListNode = NULL;
		NumListNodes = 0;
	}

	OrderedHashNode<T> *Find(const std::int64_t IntKey) const
	{
		std::uint64_t HashKey = GetHashKey((const std::uint8_t *)&IntKey, sizeof(std::int64_t));
		size_t x = (size_t)(HashKey % (std::uint64_t)HashSize);
		OrderedHashNode<T> *Node = HashNodes[x];

		while (Node != NULL && (Node->StrKey != NULL || Node->IntKey != IntKey))  Node = Node->NextHashNode;

		return Node;
	}

	OrderedHashNode<T> *Find(const char *StrKey, const size_t StrLen) const
	{
		std::uint64_t HashKey = GetHashKey((const std::uint8_t *)StrKey, StrLen);
		size_t x = (size_t)(HashKey % (std::uint64_t)HashSize);
		OrderedHashNode<T> *Node = HashNodes[x];

		while (Node != NULL && (Node->StrKey == NULL || Node->HashKey != HashKey || (size_t)Node->IntKey != StrLen || memcmp(Node->StrKey, StrKey, StrLen)))  Node = Node->NextHashNode;

		return Node;
	}

	OrderedHashNode<T> *Find(OrderedHashNode<T> *FindNode) const
	{
		std::uint64_t HashKey;

		if (FindNode->HashKey)  HashKey = FindNode->HashKey;
		else
		{
			if (FindNode->StrKey != NULL)  HashKey = GetHashKey((std::uint8_t *)FindNode->StrKey, (size_t)FindNode->IntKey);
			else  HashKey = GetHashKey((std::uint8_t *)&(FindNode->IntKey), sizeof(std::int64_t));
		}

		size_t x = (size_t)(HashKey % (std::uint64_t)HashSize);
		OrderedHashNode<T> *Node = HashNodes[x];

		if (FindNode->StrKey == NULL)
		{
			while (Node != NULL && (Node->StrKey != NULL || Node->IntKey != FindNode->IntKey))  Node = Node->NextHashNode;
		}
		else
		{
			while (Node != NULL && (Node->StrKey == NULL || Node->HashKey != HashKey || Node->IntKey != FindNode->IntKey || memcmp(Node->StrKey, FindNode->StrKey, (size_t)FindNode->IntKey)))  Node = Node->NextHashNode;
		}

		return Node;
	}

	bool ResizeHash(size_t NewHashSize)
	{
		if (!NewHashSize)  return false;

		if (HashNodes != NULL)
		{
			delete[] HashNodes;

			HashNodes = NULL;
		}

		size_t x;
		HashSize = NewHashSize;
		HashNodes = new OrderedHashNode<T> *[NewHashSize];
		for (x = 0; x < NewHashSize; x++)  HashNodes[x] = NULL;

		// Attach the list nodes to the new hash.
		OrderedHashNode<T> *Node = FirstListNode;
		while (Node != NULL)
		{
			Node->PrevHashNode = NULL;
			x = (size_t)(Node->HashKey % HashSize);
			Node->NextHashNode = HashNodes[x];
			if (HashNodes[x] != NULL)  HashNodes[x]->PrevHashNode = Node;
			HashNodes[x] = Node;

			Node = Node->NextListNode;
		}

		// Calculate the next auto-resize point.
		for (x = 0; x < sizeof(OrderedHashUtil::Primes) / sizeof(size_t) && OrderedHashUtil::Primes[x] <= NewHashSize; x++);
		NextPrimePos = x;

		return true;
	}

	inline OrderedHashNode<T> **RawHash() const  { return HashNodes; }
	inline size_t GetHashSize() const  { return HashSize; }

	inline OrderedHashNode<T> *FirstList() const  { return FirstListNode; }
	inline OrderedHashNode<T> *LastList() const  { return LastListNode; }
	inline size_t GetListSize() const  { return NumListNodes; }

private:
	inline std::uint64_t GetHashKey(const std::uint8_t *Str, size_t Size) const
	{
		return (UseSipHash ? OrderedHashUtil::GetSipHashKey(Str, Size, Key1, Key2, 2, 4) : (std::uint64_t)OrderedHashUtil::GetDJBX33XHashKey(Str, Size, (size_t)Key1));
	}

	bool UseSipHash;
	std::uint64_t Key1, Key2;

	OrderedHashNode<T> **HashNodes;
	size_t HashSize, NextPrimePos;

	OrderedHashNode<T> *FirstListNode, *LastListNode;
	size_t NumListNodes;
};
