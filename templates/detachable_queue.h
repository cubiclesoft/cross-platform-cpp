// Queue with detachable nodes.
// (C) 2013 CubicleSoft.  All Rights Reserved.

#ifndef CUBICLESOFT_DETACHABLE_QUEUE
#define CUBICLESOFT_DETACHABLE_QUEUE

#include <cstddef>

namespace CubicleSoft
{
	template <class T>
	class Queue;
	template <class T>
	class QueueNoCopy;

	template <class T>
	class QueueNode
	{
		friend class Queue<T>;
		friend class QueueNoCopy<T>;

	public:
		QueueNode() : NextNode(NULL)
		{
		}

		inline QueueNode<T> *Next()  { return NextNode; }

	private:
		QueueNode<T> *NextNode;

	public:
		T Value;
	};

	// Queue.  A single linked list.
	#include "detachable_queue_util.h"

	// QueueNoCopy.  A single linked list with a private copy constructor and assignment operator.
	#define CUBICLESOFT_DETACHABLE_QUEUE_NOCOPYASSIGN
	#include "detachable_queue_util.h"
	#undef CUBICLESOFT_DETACHABLE_QUEUE_NOCOPYASSIGN
}

#endif