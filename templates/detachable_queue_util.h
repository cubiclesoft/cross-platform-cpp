// Queue with detachable nodes.
// (C) 2013 CubicleSoft.  All Rights Reserved.

// NOTE:  This file is intended to be included from 'detachable_queue.h'.

// Implements a queue that can detach and attach compatible nodes.
template <class T>
#ifdef CUBICLESOFT_DETACHABLE_QUEUE_NOCOPYASSIGN
class QueueNoCopy
#else
class Queue
#endif
{
public:
#ifdef CUBICLESOFT_DETACHABLE_QUEUE_NOCOPYASSIGN
	QueueNoCopy() : FirstNode(NULL), LastNode(NULL), NumNodes(0)
#else
	Queue() : FirstNode(NULL), LastNode(NULL), NumNodes(0)
#endif
	{
	}

#ifdef CUBICLESOFT_DETACHABLE_QUEUE_NOCOPYASSIGN
	~QueueNoCopy()
#else
	~Queue()
#endif
	{
		Empty();
	}

#ifdef CUBICLESOFT_DETACHABLE_QUEUE_NOCOPYASSIGN
	QueueNoCopy(T Value) : FirstNode(NULL), LastNode(NULL), NumNodes(0)
#else
	Queue(T Value) : FirstNode(NULL), LastNode(NULL), NumNodes(0)
#endif
	{
		Push(Value);
	}

#ifdef CUBICLESOFT_DETACHABLE_QUEUE_NOCOPYASSIGN
private:
	QueueNoCopy(const Queue<T> &);
	QueueNoCopy<T> &operator=(const QueueNoCopy<T> &);

public:
#else
	Queue(const Queue<T> &TempQueue)
	{
		FirstNode = NULL;
		LastNode = NULL;
		NumNodes = 0;

		QueueNode<T> *Node = TempQueue.FirstNode;
		while (Node != NULL)
		{
			Push(Node->Value);
			Node = Node->NextNode;
		}
	}

	Queue<T> &operator=(const Queue<T> &TempQueue)
	{
		if (&TempQueue != this)
		{
			Empty();

			QueueNode<T> *Node = TempQueue.FirstNode;
			while (Node != NULL)
			{
				Push(Node->Value);
				Node = Node->NextNode;
			}
		}

		return *this;
	}
#endif

#ifdef CUBICLESOFT_DETACHABLE_QUEUE_NOCOPYASSIGN
	inline QueueNoCopy<T> &operator+=(const T &Value)
#else
	inline Queue<T> &operator+=(const T &Value)
#endif
	{
		Push(Value);

		return *this;
	}

#ifdef CUBICLESOFT_DETACHABLE_QUEUE_NOCOPYASSIGN
	inline QueueNoCopy<T> &operator+=(QueueNode<T> *Node)
#else
	inline Queue<T> &operator+=(QueueNode<T> *Node)
#endif
	{
		Push(Node);

		return *this;
	}

	QueueNode<T> *Push(const T &Value)
	{
		QueueNode<T> *Node;

		Node = new QueueNode<T>;
		Node->Value = Value;

		return Push(Node);
	}

	// Only use with detached nodes.
	QueueNode<T> *Push(QueueNode<T> *Node)
	{
		if (Node->NextNode != NULL)  return NULL;

		if (LastNode != NULL)  LastNode->NextNode = Node;
		LastNode = Node;
		if (FirstNode == NULL)  FirstNode = LastNode;
		NumNodes++;

		return Node;
	}

	QueueNode<T> *Unshift(const T &Value)
	{
		QueueNode<T> *Node;

		Node = new QueueNode<T>;
		Node->Value = Value;

		return Unshift(Node);
	}

	// Only use with detached nodes.
	QueueNode<T> *Unshift(QueueNode<T> *Node)
	{
		if (Node->NextNode != NULL)  return NULL;

		Node->NextNode = FirstNode;
		FirstNode = Node;
		if (LastNode == NULL)  LastNode = FirstNode;
		NumNodes++;

		return Node;
	}

	QueueNode<T> *Shift()
	{
		if (FirstNode == NULL)  return NULL;

		QueueNode<T> *Node = FirstNode;
		FirstNode = Node->NextNode;
		if (FirstNode == NULL)  LastNode = NULL;
		Node->NextNode = NULL;
		NumNodes--;

		return Node;
	}

	static inline QueueNode<T> *CreateNode()
	{
		return new QueueNode<T>;
	}

	static inline QueueNode<T> *CreateNode(const T &Value)
	{
		QueueNode<T> *Node = new QueueNode<T>;
		Node->Value = Value;

		return Node;
	}

	// Detaches all of the nodes in TempQueue and appends them to the end of the queue.
#ifdef CUBICLESOFT_DETACHABLE_QUEUE_NOCOPYASSIGN
	void DetachAllAndAppend(QueueNoCopy<T> &TempQueue)
#else
	void DetachAllAndAppend(Queue<T> &TempQueue)
#endif
	{
		if (TempQueue.FirstNode == NULL)  return;

		if (LastNode != NULL)
		{
			LastNode->NextNode = TempQueue.FirstNode;
			LastNode = TempQueue.LastNode;
		}
		else
		{
			FirstNode = TempQueue.FirstNode;
			LastNode = TempQueue.LastNode;
		}
		NumNodes += TempQueue.NumNodes;

		TempQueue.FirstNode = NULL;
		TempQueue.LastNode = NULL;
		TempQueue.NumNodes = 0;
	}

	void Empty()
	{
		QueueNode<T> *Node;

		while (FirstNode != NULL)
		{
			Node = Shift();
			delete Node;
		}

		NumNodes = 0;
	}

	inline QueueNode<T> *First() const  { return FirstNode; }
	inline QueueNode<T> *Last() const  { return LastNode; }
	inline size_t GetSize() const  { return NumNodes; }

private:
	QueueNode<T> *FirstNode, *LastNode;
	size_t NumNodes;
};
