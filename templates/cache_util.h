// Cache.
// (C) 2013 CubicleSoft.  All Rights Reserved.

// NOTE:  This file is intended to be included from 'cache.h'.

// Implements a hash for use primarily as a fixed-size cache.

template <class XKey, class XValue>
#ifdef CUBICLESOFT_CACHE_NOCOPYASSIGN
class CacheNoCopy
#else
class Cache
#endif
{
public:
#ifdef CUBICLESOFT_CACHE_NOCOPYASSIGN
	CacheNoCopy(size_t PrimeNum) : NumNodes(PrimeNum)
#else
	Cache(size_t PrimeNum) : NumNodes(PrimeNum)
#endif
	{
		Nodes = new CacheNode<XKey, XValue>[NumNodes];
	}

#ifdef CUBICLESOFT_CACHE_NOCOPYASSIGN
	~CacheNoCopy()
#else
	~Cache()
#endif
	{
		if (Nodes != NULL)  delete[] Nodes;
	}

#ifdef CUBICLESOFT_CACHE_NOCOPYASSIGN
	CacheNoCopy(const CacheNoCopy<XKey, XValue> &TempCache);
	CacheNoCopy<XKey, XValue> &operator=(const CacheNoCopy<XKey, XValue> &TempCache);
#else
	Cache(const Cache<XKey, XValue> &TempCache)
	{
		size_t x;

		NumNodes = TempCache.NumNodes;
		Nodes = new CacheNode<XKey, XValue>[NumNodes];
		for (x = 0; x < NumNodes; x++)  Nodes[x] = TempCache.Nodes[x];
	}

	Cache<XKey, XValue> &operator=(const Cache<XKey, XValue> &TempCache)
	{
		if (&TempCache != this)
		{
			size_t x;

			if (NumNodes != TempCache.NumNodes)
			{
				if (Nodes != NULL)  delete[] Nodes;

				NumNodes = TempCache.NumNodes;
				Nodes = new CacheNode<XKey, XValue>[NumNodes];
			}

			for (x = 0; x < NumNodes; x++)  Nodes[x] = TempCache.Nodes[x];
		}

		return *this;
	}
#endif

	void Empty()
	{
		size_t x;

		for (x = 0; x < NumNodes; x++)  Nodes[x].Used = false;
	}

	void Insert(size_t HashKey, const XKey &Key, const XValue &Value)
	{
		HashKey = HashKey % NumNodes;

		Nodes[HashKey].Used = true;
		Nodes[HashKey].Key = Key;
		Nodes[HashKey].Value = Value;
	}

	bool Remove(size_t HashKey, const XKey &Key)
	{
		HashKey = HashKey % NumNodes;
		if (Nodes[HashKey].Used && Nodes[HashKey].Key == Key)
		{
			Nodes[HashKey].Used = false;

			return true;
		}

		return false;
	}

	bool Exists(size_t HashKey, const XKey &Key)
	{
		HashKey = HashKey % NumNodes;

		return (Nodes[HashKey].Used && Nodes[HashKey].Key == Key);
	}

	bool Find(XValue &Result, size_t HashKey, const XKey &Key)
	{
		HashKey = HashKey % NumNodes;
		if (Nodes[HashKey].Used && Nodes[HashKey].Key == Key)
		{
			Result = Nodes[HashKey].Value;

			return true;
		}

		return false;
	}

	inline size_t GetSize()  { return NumNodes; }
	inline CacheNode<XKey, XValue> *RawData()  { return Nodes; }

private:
	CacheNode<XKey, XValue> *Nodes;
	size_t NumNodes;
};
