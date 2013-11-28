// Double linked list with detachable nodes.
// (C) 2013 CubicleSoft.  All Rights Reserved.

#ifndef CUBICLESOFT_DETACHABLE_LIST
#define CUBICLESOFT_DETACHABLE_LIST

namespace CubicleSoft
{
	template <class T>
	class List;
	template <class T>
	class ListNoCopy;

	template <class T>
	class ListNode
	{
		friend class List<T>;
		friend class ListNoCopy<T>;

	public:
		ListNode() : PrevNode(NULL), NextNode(NULL)
		{
		}

		inline ListNode<T> *Prev()  { return PrevNode; }
		inline ListNode<T> *Next()  { return NextNode; }

	private:
		ListNode<T> *PrevNode;
		ListNode<T> *NextNode;

	public:
		T Value;
	};

	// List.  A double linked list.
	#include "detachable_list_util.h"

	// ListNoCopy.  A double linked list with a private copy constructor and assignment operator.
	#define CUBICLESOFT_DETACHABLE_LIST_NOCOPYASSIGN
	#include "detachable_list_util.h"
	#undef CUBICLESOFT_DETACHABLE_LIST_NOCOPYASSIGN
}

#endif