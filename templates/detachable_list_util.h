// Double linked list with detachable nodes.
// (C) 2013 CubicleSoft.  All Rights Reserved.

// NOTE:  This file is intended to be included from 'detachable_list.h'.

// Implements a list that can detach and attach compatible nodes.
template <class T>
#ifdef CUBICLESOFT_DETACHABLE_LIST_NOCOPYASSIGN
class ListNoCopy
#else
class List
#endif
{
public:
#ifdef CUBICLESOFT_DETACHABLE_LIST_NOCOPYASSIGN
	ListNoCopy() : FirstNode(NULL), LastNode(NULL), NumNodes(0)
#else
	List() : FirstNode(NULL), LastNode(NULL), NumNodes(0)
#endif
	{
	}

#ifdef CUBICLESOFT_DETACHABLE_LIST_NOCOPYASSIGN
	~ListNoCopy()
#else
	~List()
#endif
	{
		Empty();
	}

#ifdef CUBICLESOFT_DETACHABLE_LIST_NOCOPYASSIGN
	ListNoCopy(T Value)
#else
	List(T Value)
#endif
	{
		FirstNode = NULL;
		LastNode = NULL;
		NumNodes = 0;

		InsertBefore(NULL, Value);
	}

#ifdef CUBICLESOFT_DETACHABLE_LIST_NOCOPYASSIGN
private:
	ListNoCopy(const ListNoCopy<T> &);
	ListNoCopy<T> &operator=(const ListNoCopy<T> &);

public:
#else
	List(const List<T> &TempList)
	{
		FirstNode = NULL;
		LastNode = NULL;
		NumNodes = 0;

		ListNode<T> *Node = TempList.FirstNode;
		while (Node != NULL)
		{
			InsertBefore(NULL, Node->Value);
			Node = Node->NextNode;
		}
	}

	List<T> &operator=(const List<T> &TempList)
	{
		if (&TempList != this)
		{
			Empty();

			ListNode<T> *Node = TempList.FirstNode;
			while (Node != NULL)
			{
				InsertBefore(NULL, Node->Value);
				Node = Node->NextNode;
			}
		}

		return *this;
	}
#endif

#ifdef CUBICLESOFT_DETACHABLE_LIST_NOCOPYASSIGN
	inline ListNoCopy<T> &operator+=(const T &Value)
#else
	inline List<T> &operator+=(const T &Value)
#endif
	{
		InsertBefore(NULL, Value);

		return *this;
	}

#ifdef CUBICLESOFT_DETACHABLE_LIST_NOCOPYASSIGN
	inline ListNoCopy<T> &operator+=(ListNode<T> *Node)
#else
	inline List<T> &operator+=(ListNode<T> *Node)
#endif
	{
		InsertBefore(NULL, Node);

		return *this;
	}

	inline ListNode<T> *Push(const T &Value)
	{
		return InsertBefore(NULL, Value);
	}

	// Only use with detached nodes.
	inline ListNode<T> *Push(ListNode<T> *Node)
	{
		return InsertBefore(NULL, Node);
	}

	ListNode<T> *Pop()
	{
		ListNode<T> *Node = LastNode;
		Detach(Node);

		return Node;
	}

	inline ListNode<T> *Unshift(const T &Value)
	{
		return InsertAfter(NULL, Value);
	}

	// Only use with detached nodes.
	inline ListNode<T> *Unshift(ListNode<T> *Node)
	{
		return InsertAfter(NULL, Node);
	}

	ListNode<T> *Shift()
	{
		ListNode<T> *Node = FirstNode;
		Detach(Node);

		return Node;
	}

	static inline ListNode<T> *CreateNode()
	{
		return new ListNode<T>;
	}

	static inline ListNode<T> *CreateNode(const T &Value)
	{
		ListNode<T> *Node = new ListNode<T>;
		Node->Value = Value;

		return Node;
	}

	ListNode<T> *InsertBefore(ListNode<T> *Next, const T &Value)
	{
		ListNode<T> *Node;

		Node = new ListNode<T>;
		Node->Value = Value;
		if (Next == NULL)
		{
			Node->PrevNode = LastNode;
			if (LastNode != NULL)  LastNode->NextNode = Node;
			LastNode = Node;
			if (FirstNode == NULL)  FirstNode = LastNode;
		}
		else
		{
			Node->NextNode = Next;
			Node->PrevNode = Next->PrevNode;
			if (Node->PrevNode != NULL)  Node->PrevNode->NextNode = Node;
			Next->PrevNode = Node;
			if (Next == FirstNode)  FirstNode = Node;
		}
		NumNodes++;

		return Node;
	}

	// Only use with detached nodes.
	ListNode<T> *InsertBefore(ListNode<T> *Next, ListNode<T> *Node)
	{
		if (Node->NextNode != NULL || Node->PrevNode != NULL)  return NULL;

		if (Next == NULL)
		{
			Node->PrevNode = LastNode;
			if (LastNode != NULL)  LastNode->NextNode = Node;
			LastNode = Node;
			if (FirstNode == NULL)  FirstNode = LastNode;
		}
		else
		{
			Node->NextNode = Next;
			Node->PrevNode = Next->PrevNode;
			if (Node->PrevNode != NULL)  Node->PrevNode->NextNode = Node;
			Next->PrevNode = Node;
			if (Next == FirstNode)  FirstNode = Node;
		}
		NumNodes++;

		return Node;
	}

	ListNode<T> *InsertAfter(ListNode<T> *Prev, const T &Value)
	{
		ListNode<T> *Node;

		Node = new ListNode<T>;
		Node->Value = Value;
		if (Prev == NULL)
		{
			Node->NextNode = FirstNode;
			if (FirstNode != NULL)  FirstNode->PrevNode = Node;
			FirstNode = Node;
			if (LastNode == NULL)  LastNode = FirstNode;
		}
		else
		{
			Node->PrevNode = Prev;
			Node->NextNode = Prev->NextNode;
			if (Node->NextNode != NULL)  Node->NextNode->PrevNode = Node;
			Prev->NextNode = Node;
			if (Prev == LastNode)  LastNode = Node;
		}
		NumNodes++;

		return Node;
	}

	// Only use with detached nodes.
	ListNode<T> *InsertAfter(ListNode<T> *Prev, ListNode<T> *Node)
	{
		if (Node->NextNode != NULL || Node->PrevNode != NULL)  return NULL;

		if (Prev == NULL)
		{
			Node->NextNode = FirstNode;
			if (FirstNode != NULL)  FirstNode->PrevNode = Node;
			FirstNode = Node;
			if (LastNode == NULL)  LastNode = FirstNode;
		}
		else
		{
			Node->PrevNode = Prev;
			Node->NextNode = Prev->NextNode;
			if (Node->NextNode != NULL)  Node->NextNode->PrevNode = Node;
			Prev->NextNode = Node;
			if (Prev == LastNode)  LastNode = Node;
		}
		NumNodes++;

		return Node;
	}

	// Detaches all of the nodes in TempList and appends them to the end of the list.
#ifdef CUBICLESOFT_DETACHABLE_LIST_NOCOPYASSIGN
	void DetachAllAndAppend(ListNoCopy<T> &TempList)
#else
	void DetachAllAndAppend(List<T> &TempList)
#endif
	{
		if (TempList.FirstNode == NULL)  return;

		if (LastNode != NULL)
		{
			LastNode->NextNode = TempList.FirstNode;
			TempList.FirstNode->PrevNode = LastNode;
			LastNode = TempList.LastNode;
		}
		else
		{
			FirstNode = TempList.FirstNode;
			LastNode = TempList.LastNode;
		}
		NumNodes += TempList.NumNodes;

		TempList.FirstNode = NULL;
		TempList.LastNode = NULL;
		TempList.NumNodes = 0;
	}

	bool Detach(ListNode<T> *Node)
	{
		if (Node == NULL)  return false;

		// Disconnect the first/last node (if necessary).
		if (Node == FirstNode)
		{
			FirstNode = FirstNode->NextNode;
			if (FirstNode != NULL)  FirstNode->PrevNode = NULL;
			else  LastNode = NULL;
		}
		else if (Node == LastNode)
		{
			LastNode = LastNode->PrevNode;
			if (LastNode != NULL)  LastNode->NextNode = NULL;
			else  FirstNode = NULL;
		}
		else
		{
			if (Node->NextNode != NULL)  Node->NextNode->PrevNode = Node->PrevNode;
			if (Node->PrevNode != NULL)  Node->PrevNode->NextNode = Node->NextNode;
		}
		Node->NextNode = NULL;
		Node->PrevNode = NULL;

		NumNodes--;

		return true;
	}

	bool Remove(ListNode<T> *Node)
	{
		if (!Detach(Node))  return false;

		delete Node;

		return true;
	}

	void Empty()
	{
		while (FirstNode != NULL)  Remove(FirstNode);
		NumNodes = 0;
	}

	inline ListNode<T> *First() const  { return FirstNode; }
	inline ListNode<T> *Last() const  { return LastNode; }
	inline size_t GetSize() const  { return NumNodes; }

private:
	ListNode<T> *FirstNode, *LastNode;
	size_t NumNodes;
};
