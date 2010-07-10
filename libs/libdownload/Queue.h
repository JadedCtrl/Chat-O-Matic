/* This file is Copyright 2005 Level Control Systems.  See the included LICENSE.txt file for details. */

#ifndef MuscleQueue_h
#define MuscleQueue_h

//#include "support/MuscleSupport.h"

#define ARRAYITEMS(x) (sizeof(x)/sizeof(x[0]))  /* returns # of items in array */
template<typename T> inline const T& muscleMax(const T& p1, const T& p2)
{
	return (p1 < p2) ? p2 : p1;
}
template<typename T> inline const T& muscleMin(const T& p1, const T& p2)
{
	return (p1 < p2) ? p1 : p2;
}

# define newnothrow new
# define newnothrow_array(T, count) newnothrow T[count]

#ifndef SMALL_QUEUE_SIZE
# define SMALL_QUEUE_SIZE 3
#endif

/** This class implements a templated double-ended queue data structure.
 *  Adding or removing items from the head or tail of a Queue is (on average)
 *  an O(1) operation.  A Queue also makes for a nice Vector, if that's all you need.
 */
template <class ItemType> class Queue
{
public:
	/** Constructor.
	 *  @param initialSlots Specifies how many slots to pre-allocate.  Defaults to (SMALL_QUEUE_SIZE).
	 */
	Queue(uint32 initialSlots = SMALL_QUEUE_SIZE);

	/** Copy constructor. */
	Queue(const Queue& copyMe);

	/** Destructor. */
	virtual ~Queue();

	/** Assigment operator. */
	Queue& operator=(const Queue& from);

	/** Equality operator.  Queues are equal if they are the same length, and
	  * every nth item in this queue is == to the corresponding item in (rhs). */
	bool operator==(const Queue& rhs) const;

	/** Returns the negation of the equality operator */
	bool operator!=(const Queue& rhs) const {
		return !(*this == rhs);
	}

	/** Appends (item) to the end of the queue.  Queue size grows by one.
	 *  @param item The item to append.
	 *  @return B_NO_ERROR on success, B_ERROR on failure (out of memory)
	 */
	status_t AddTail(const ItemType& item = ItemType());

	/** Appends some or all items in (queue) to the end of our queue.  Queue size
	 *  grows by (queue.GetNumItems()).
	 *  For example:
	 *    Queue a;   // contains 1, 2, 3, 4
	 *    Queue b;   // contains 5, 6, 7, 8
	 *    a.AddTail(b);      // a now contains 1, 2, 3, 4, 5, 6, 7, 8
	 *  @param queue The queue to append to our queue.
	 *  @param startIndex Index in (queue) to start adding at.  Default to zero.
	 *  @param numItems Number of items to add.  If this number is too large, it will be capped appropriately.  Default is to add all items.
	 *  @return B_NO_ERROR on success, B_ERROR on failure (out of memory)
	 */
	status_t AddTail(const Queue<ItemType> & queue, uint32 startIndex = 0, uint32 numItems = (uint32) - 1);

	/** Adds the given array of items to the tail of the Queue.  Equivalent
	 *  to adding them to the tail of the Queue one at a time, but somewhat
	 *  more efficient.  On success, the queue size grows by (numItems).
	 *  @param items Pointer to an array of items to add to the Queue.
	 *  @param numItems Number of items in the array
	 *  @return B_NO_ERROR on success, or B_ERROR on failure (out of memory)
	 */
	status_t AddTail(const ItemType* items, uint32 numItems);

	/** Prepends (item) to the head of the queue.  Queue size grows by one.
	 *  @param item The item to prepend.
	 *  @return B_NO_ERROR on success, B_ERROR on failure (out of memory)
	 */
	status_t AddHead(const ItemType& item = ItemType());

	/** Concatenates (queue) to the head of our queue.
	 *  Our queue size grows by (queue.GetNumItems()).
	 *  For example:
	 *    Queue a;      // contains 1, 2, 3, 4
	 *    Queue b;      // contains 5, 6, 7, 8
	 *    a.AddHead(b); // a now contains 5, 6, 7, 8, 1, 2, 3, 4
	 *  @param queue The queue to prepend to our queue.
	 *  @param startIndex Index in (queue) to start adding at.  Default to zero.
	 *  @param numItems Number of items to add.  If this number is too large, it will be capped appropriately.  Default is to add all items.
	 *  @return B_NO_ERROR on success, B_ERROR on failure (out of memory)
	 */
	status_t AddHead(const Queue<ItemType> & queue, uint32 startIndex = 0, uint32 numItems = (uint32) - 1);

	/** Concatenates the given array of items to the head of the Queue.
	 *  The semantics are the same as for AddHead(const Queue<ItemType> &).
	 *  @param items Pointer to an array of items to add to the Queue.
	 *  @param numItems Number of items in the array
	 *  @return B_NO_ERROR on success, or B_ERROR on failure (out of memory)
	 */
	status_t AddHead(const ItemType* items, uint32 numItems);

	/** Removes the item at the head of the queue.
	 *  @return B_NO_ERROR on success, B_ERROR if the queue was empty.
	 */
	status_t RemoveHead();

	/** Removes the item at the head of the queue and places it into (returnItem).
	 *  @param returnItem On success, the removed item is copied into this object.
	 *  @return B_NO_ERROR on success, B_ERROR if the queue was empty
	 */
	status_t RemoveHead(ItemType& returnItem);

	/** Removes the item at the tail of the queue.
	 *  @return B_NO_ERROR on success, B_ERROR if the queue was empty.
	 */
	status_t RemoveTail();

	/** Removes the item at the tail of the queue and places it into (returnItem).
	 *  @param returnItem On success, the removed item is copied into this object.
	 *  @return B_NO_ERROR on success, B_ERROR if the queue was empty
	 */
	status_t RemoveTail(ItemType& returnItem);

	/** Removes the item at the (index)'th position in the queue.
	 *  @param index Which item to remove--can range from zero
	 *               (head of the queue) to GetNumItems()-1 (tail of the queue).
	 *  @return B_NO_ERROR on success, B_ERROR on failure (i.e. bad index)
	 *  Note that this method is somewhat inefficient for indices that
	 *  aren't at the head or tail of the queue (i.e. O(n) time)
	 */
	status_t RemoveItemAt(uint32 index);

	/** Removes the item at the (index)'th position in the queue, and copies it into (returnItem).
	 *  @param index Which item to remove--can range from zero
	 *               (head of the queue) to (GetNumItems()-1) (tail of the queue).
	 *  @param returnItem On success, the removed item is copied into this object.
	 *  @return B_NO_ERROR on success, B_ERROR on failure (i.e. bad index)
	 */
	status_t RemoveItemAt(uint32 index, ItemType& returnItem);

	/** Copies the (index)'th item into (returnItem).
	 *  @param index Which item to get--can range from zero
	 *               (head of the queue) to (GetNumItems()-1) (tail of the queue).
	 *  @param returnItem On success, the retrieved item is copied into this object.
	 *  @return B_NO_ERROR on success, B_ERROR on failure (e.g. bad index)
	 */
	status_t GetItemAt(uint32 index, ItemType& returnItem) const;

	/** Returns a pointer to an item in the array (i.e. no copying of the item is done).
	 *  Included for efficiency; be careful with this: modifying the queue can invalidate
	 *  the returned pointer!
	 *  @param index Index of the item to return a pointer to.
	 *  @return a pointer to the internally held item, or NULL if (index) was invalid.
	 */
	ItemType* GetItemAt(uint32 index) const;

	/** Replaces the (index)'th item in the queue with (newItem).
	 *  @param index Which item to replace--can range from zero
	 *               (head of the queue) to (GetNumItems()-1) (tail of the queue).
	 *  @param newItem The item to place into the queue at the (index)'th position.
	 *  @return B_NO_ERROR on success, B_ERROR on failure (e.g. bad index)
	 */
	status_t ReplaceItemAt(uint32 index, const ItemType& newItem = ItemType());

	/** Inserts (item) into the (nth) slot in the array.  InsertItemAt(0)
	 *  is the same as AddHead(item), InsertItemAt(GetNumItems()) is the same
	 *  as AddTail(item).  Other positions will involve an O(n) shifting of contents.
	 *  @param index The position at which to insert the new item.
	 *  @param newItem The item to insert into the queue.
	 *  @return B_NO_ERROR on success, B_ERROR on failure (i.e. bad index).
	 */
	status_t InsertItemAt(uint32 index, const ItemType& newItem = ItemType());

	/** Removes all items from the queue.
	 *  @param releaseCachedBuffers If true, we will immediately free any buffers that we may be holding.  Otherwise
	 *                              we will keep them around so that they can be re-used in the future.
	 */
	void Clear(bool releaseCachedBuffers = false);

	/** Returns the number of items in the queue.  (This number does not include pre-allocated space) */
	uint32 GetNumItems() const {
		return _itemCount;
	}

	/** Returns the number of item-slots we have allocated space for.  Note that this is NOT
	 *  the same as the value returned by GetNumItems() -- it is generally larger, since we pre-allocate
	 *  additional slots in advance to cut down on the number of re-allocations we need to do.
	 */
	uint32 GetNumAllocatedItemSlots() const {
		return _queueSize;
	}

	/** Returns true iff their are no items in the queue. */
	bool IsEmpty() const {
		return (_itemCount == 0);
	}

	/** Returns a read-only reference the head item in the queue.  You must not call this when the queue is empty! */
	const ItemType& Head() const {
		return *GetItemAt(0);
	}

	/** Returns a read-only reference the tail item in the queue.  You must not call this when the queue is empty! */
	const ItemType& Tail() const {
		return *GetItemAt(_itemCount - 1);
	}

	/** Returns a writable reference the head item in the queue.  You must not call this when the queue is empty! */
	ItemType& Head() {
		return *GetItemAt(0);
	}

	/** Returns a writable reference the tail item in the queue.  You must not call this when the queue is empty! */
	ItemType& Tail() {
		return *GetItemAt(_itemCount - 1);
	}

	/** Returns a pointer to the first item in the queue, or NULL if the queue is empty */
	ItemType* HeadPointer() const {
		return (_itemCount > 0) ? GetItemAt(0) : NULL;
	}

	/** Returns a pointer to the last item in the queue, or NULL if the queue is empty */
	ItemType* TailPointer() const {
		return (_itemCount > 0) ? GetItemAt(_itemCount - 1) : NULL;
	}

	/** Convenient read-only array-style operator (be sure to only use valid indices!) */
	const ItemType& operator [](uint32 Index) const;

	/** Convenient read-write array-style operator (be sure to only use valid indices!) */
	ItemType& operator [](uint32 Index);

	/** Deprecated synonym for GetItemAt().  Don't call this method in new code, call GetItemAt() instead!
	  * @deprecated
	  */
	ItemType* GetItemPointer(uint32 index) const {
		return GetItemAt(index);
	}

	/** Makes sure there is enough space allocated for at least (numSlots) items.
	 *  You only need to call this if you wish to minimize the number of data re-allocations done,
	 *  or wish to add or remove a large number of default items at once (by specifying setNumItems=true).
	 *  @param numSlots the minimum amount of items to pre-allocate space for in the Queue.
	 *  @param setNumItems If true, the length of the Queue will be altered by adding or removing
	 *                     items to (from) the tail of the Queue until the Queue is the specified size.
	 *                     If false (the default), more slots may be pre-allocated, but the
	 *                     number of items officially in the Queue remains the same as before.
	 *  @param extraReallocItems If we have to do an array reallocation, this many extra slots will be
	 *                           added to the newly allocated array, above and beyond what is strictly
	 *                           necessary.  That way the likelihood of another reallocation being necessary
	 *                           in the near future is reduced.  Default value is zero, indicating that
	 *                           no extra slots will be allocated.  This argument is ignored if (setNumItems) is true.
	 *  @returns B_NO_ERROR on success, or B_ERROR on failure (out of memory)
	 */
	status_t EnsureSize(uint32 numSlots, bool setNumItems = false, uint32 extraReallocItems = 0);

	/** Returns the last index of the given (item), or -1 if (item) is
	 *  not found in the list.  O(n) search time.
	 *  @param item The item to look for.
	 *  @return The index of (item), or -1 if no such item is present.
	 */
	int32 IndexOf(const ItemType& item) const;

	/**
	 *  Swaps the values of the two items at the given indices.  This operation
	 *  will involve three copies of the held items.
	 *  @param fromIndex First index to swap.
	 *  @param toIndex   Second index to swap.
	 */
	void Swap(uint32 fromIndex, uint32 toIndex);

	/**
	 *  Reverses the ordering of the items in the given subrange.
	 *  (e.g. if the items were A,B,C,D,E, this would change them to E,D,C,B,A)
	 *  @param from Index of the start of the subrange.  Defaults to zero.
	 *  @param to Index of the next item after the end of the subrange.  If greater than
	 *         the number of items currently in the queue, this value will be clipped
	 *         to be equal to that number.  Defaults to the largest possible uint32.
	 */
	void ReverseItemOrdering(uint32 from = 0, uint32 to = ((uint32) - 1));

	/**
	 *  This is the signature of the type of callback function that you must pass
	 *  into the Sort() method.  This function should work like strcmp(), returning
	 *  a negative value if (item1) is less than item2, or zero if the items are
	 *  equal, or a positive value if (item1) is greater than item2.
	 *  @param item1 The first item
	 *  @param item2 The second item
	 *  @param cookie A user-defined value that was passed in to the Sort() method.
	 *  @return A value indicating which item is "larger", as defined above.
	 */
	typedef int (*ItemCompareFunc)(const ItemType& item1, const ItemType& item2, void* cookie);

	/**
	 *  Does an in-place, stable sort on a subrange of the contents of this Queue.
	 *  (The sort algorithm is a smart, in-place merge sort).
	 *  @param compareFunc A function that compares two items in this queue and returns
	 *         a value indicating which is "larger".  (negative indicates
	 *         that the second parameter is larger, positive indicate that the
	 *         first is larger, and zero indicates that the two parameters are equal)
	 *  @param from Index of the start of the subrange.  Defaults to zero.
	 *  @param to Index of the next item after the end of the subrange.
	 *         If greater than the number of items currently in the queue,
	 *         the subrange will extend to the last item.  Defaults to the largest possible uint32.
	 *  @param optCookie A user-defined value that will be passed to the (compareFunc).
	 */
	void Sort(ItemCompareFunc compareFunc, uint32 from = 0, uint32 to = ((uint32) - 1), void* optCookie = NULL);

	/**
	 *  Swaps our contents with the contents of (that), in an efficient manner.
	 *  @param that The queue whose contents are to be swapped with our own.
	 */
	void SwapContents(Queue<ItemType> & that);

	/**
	 *  Goes through the array and removes every item that is equal to (val).
	 *  @param val the item to look for and remove
	 *  @return The number of instances of (val) that were found and removed during this operation.
	 */
	uint32 RemoveAllInstancesOf(const ItemType& val);

	/**
	 *  Goes through the array and removes the first item that is equal to (val).
	 *  @param val the item to look for and remove
	 *  @return B_NO_ERROR if a matching item was found and removed, or B_ERROR if it wasn't found.
	 */
	status_t RemoveFirstInstanceOf(const ItemType& val);

	/**
	 *  Goes through the array and removes the last item that is equal to (val).
	 *  @param val the item to look for and remove
	 *  @return B_NO_ERROR if a matching item was found and removed, or B_ERROR if it wasn't found.
	 */
	status_t RemoveLastInstanceOf(const ItemType& val);

	/** Returns true iff the first item in our queue is equal to (prefix). */
	bool StartsWith(const ItemType& prefix) const {
		return ((GetNumItems() > 0) && (Head() == prefix));
	}

	/** Returns true iff the (prefixQueue) is a prefix of this queue. */
	bool StartsWith(const Queue<ItemType> & prefixQueue) const;

	/** Returns true iff the last item in our queue is equal to (suffix). */
	bool EndsWith(const ItemType& suffix) const {
		return ((GetNumItems() > 0) && (Tail() == suffix));
	}

	/** Returns true iff the (suffixQueue) is a suffix of this queue. */
	bool EndsWith(const Queue<ItemType> & suffixQueue) const;

	/**
	 *  Returns a pointer to the nth internally-held contiguous-Item-sub-array, to allow efficient
	 *  array-style access to groups of items in this Queue.  In the current implementation
	 *  there may be as many as two such sub-arrays present, depending on the internal state of the Queue.
	 *  @param whichArray Index of the internal array to return a pointer to.  Typically zero or one.
	 *  @param retLength On success, the number of items in the returned sub-array will be written here.
	 *  @return Pointer to the first item in the sub-array on success, or NULL on failure.
	 *          Note that this array is only guaranteed valid as long as no items are
	 *          added or removed from the Queue.
	 */
	ItemType* GetArrayPointer(uint32 whichArray, uint32& retLength) {
		return const_cast<ItemType*>(GetArrayPointerAux(whichArray, retLength));
	}

	/** Read-only version of the above */
	const ItemType* GetArrayPointer(uint32 whichArray, uint32& retLength) const {
		return GetArrayPointerAux(whichArray, retLength);
	}

private:
	const ItemType* GetArrayPointerAux(uint32 whichArray, uint32& retLength) const;
	void SwapContentsAux(Queue<ItemType> & that);

	inline uint32 NextIndex(uint32 idx) const {
		return (idx >= _queueSize - 1) ? 0 : idx + 1;
	}
	inline uint32 PrevIndex(uint32 idx) const {
		return (idx == 0) ? _queueSize - 1 : idx - 1;
	}

	// Translates a user-index into an index into the _queue array.
	inline uint32 InternalizeIndex(uint32 idx) const {
		return (_headIndex + idx) % _queueSize;
	}

	// Helper methods, used for sorting (stolen from http://www-ihm.lri.fr/~thomas/VisuTri/inplacestablesort.html)
	void Merge(ItemCompareFunc compareFunc, uint32 from, uint32 pivot, uint32 to, uint32 len1, uint32 len2, void* cookie);
	uint32 Lower(ItemCompareFunc compareFunc, uint32 from, uint32 to, const ItemType& val, void* cookie) const;
	uint32 Upper(ItemCompareFunc compareFunc, uint32 from, uint32 to, const ItemType& val, void* cookie) const;

	ItemType _smallQueue[SMALL_QUEUE_SIZE];  // small queues can be stored inline in this array
	ItemType* _queue;                        // points to _smallQueue, or to a dynamically alloc'd array
	uint32 _queueSize;  // number of slots in the _queue array
	uint32 _itemCount;  // number of valid items in the array
	uint32 _headIndex;  // index of the first filled slot (meaningless if _itemCount is zero)
	uint32 _tailIndex;  // index of the last filled slot (meaningless if _itemCount is zero)
	const uint32 _initialSize;  // as specified in ctor
};

template <class ItemType>
Queue<ItemType>::Queue(uint32 initialSize)
	: _queue(NULL), _queueSize(0), _itemCount(0), _initialSize(initialSize)
{
	// empty
}

template <class ItemType>
Queue<ItemType>::Queue(const Queue& rhs)
	: _queue(NULL), _queueSize(0), _itemCount(0), _initialSize(rhs._initialSize)
{
	*this = rhs;
}

template <class ItemType>
bool
Queue<ItemType>::operator ==(const Queue& rhs) const
{
	if (this == &rhs) return true;
	if (GetNumItems() != rhs.GetNumItems()) return false;

	for (int i = GetNumItems() - 1; i >= 0; i--) if (((*this)[i] == rhs[i]) == false) return false;

	return true;
}


template <class ItemType>
Queue<ItemType> &
Queue<ItemType>::operator =(const Queue& rhs)
{
	if (this != &rhs) {
		uint32 numItems = rhs.GetNumItems();
		if (EnsureSize(numItems, true) == B_NO_ERROR) for (uint32 i = 0; i < numItems; i++) (*this)[i] = rhs[i];
	}
	return *this;
}

template <class ItemType>
ItemType &
Queue<ItemType>::operator[](uint32 i)
{
	//   MASSERT(i<_itemCount, "Invalid index to Queue::[]");
	return _queue[InternalizeIndex(i)];
}

template <class ItemType>
const ItemType &
Queue<ItemType>::operator[](uint32 i) const
{
	//   MASSERT(i<_itemCount, "Invalid index to Queue::[]");
	return _queue[InternalizeIndex(i)];
}

template <class ItemType>
ItemType *
Queue<ItemType>::GetItemAt(uint32 i) const
{
	return &_queue[InternalizeIndex(i)];
}

template <class ItemType>
Queue<ItemType>::~Queue()
{
	if (_queue != _smallQueue) delete [] _queue;
}

template <class ItemType>
status_t
Queue<ItemType>::
AddTail(const ItemType& item)
{
	if (EnsureSize(_itemCount + 1, false, _itemCount + 1) == B_ERROR) return B_ERROR;
	if (_itemCount == 0) _headIndex = _tailIndex = 0;
	else _tailIndex = NextIndex(_tailIndex);
	_queue[_tailIndex] = item;
	_itemCount++;
	return B_NO_ERROR;
}

template <class ItemType>
status_t
Queue<ItemType>::
AddTail(const Queue<ItemType> &queue, uint32 startIndex, uint32 numNewItems)
{
	uint32 hisSize = queue.GetNumItems();
	numNewItems = muscleMin(numNewItems, (startIndex < hisSize) ? (hisSize - startIndex) : 0);

	uint32 mySize = GetNumItems();
	uint32 newSize = mySize + numNewItems;

	if (EnsureSize(newSize, true) != B_NO_ERROR) return B_ERROR;
	for (uint32 i = mySize; i < newSize; i++) (*this)[i] = queue[startIndex++];
	return B_NO_ERROR;
}

template <class ItemType>
status_t
Queue<ItemType>::
AddTail(const ItemType* items, uint32 numItems)
{
	uint32 mySize = GetNumItems();
	uint32 newSize = mySize + numItems;
	uint32 rhs = 0;

	if (EnsureSize(newSize, true) != B_NO_ERROR) return B_ERROR;
	for (uint32 i = mySize; i < newSize; i++) (*this)[i] = items[rhs++];
	return B_NO_ERROR;
}

template <class ItemType>
status_t
Queue<ItemType>::
AddHead(const ItemType& item)
{
	if (EnsureSize(_itemCount + 1, false, _itemCount + 1) == B_ERROR) return B_ERROR;
	if (_itemCount == 0) _headIndex = _tailIndex = 0;
	else _headIndex = PrevIndex(_headIndex);
	_queue[_headIndex] = item;
	_itemCount++;
	return B_NO_ERROR;
}

template <class ItemType>
status_t
Queue<ItemType>::
AddHead(const Queue<ItemType> &queue, uint32 startIndex, uint32 numNewItems)
{
	uint32 hisSize = queue.GetNumItems();
	numNewItems = muscleMin(numNewItems, (startIndex < hisSize) ? (hisSize - startIndex) : 0);

	if (EnsureSize(numNewItems + GetNumItems()) != B_NO_ERROR) return B_ERROR;
	for (int i = ((int)startIndex + numNewItems) - 1; i >= (int32)startIndex; i--) if (AddHead(queue[i]) == B_ERROR) return B_ERROR;
	return B_NO_ERROR;
}

template <class ItemType>
status_t
Queue<ItemType>::
AddHead(const ItemType* items, uint32 numItems)
{
	if (EnsureSize(_itemCount + numItems) != B_NO_ERROR) return B_ERROR;
	for (int i = ((int)numItems) - 1; i >= 0; i--) if (AddHead(items[i]) == B_ERROR) return B_ERROR;
	return B_NO_ERROR;
}

template <class ItemType>
status_t
Queue<ItemType>::
RemoveHead(ItemType& returnItem)
{
	if (_itemCount == 0) return B_ERROR;
	returnItem = _queue[_headIndex];
	return RemoveHead();
}

template <class ItemType>
status_t
Queue<ItemType>::
RemoveHead()
{
	if (_itemCount == 0) return B_ERROR;
	int oldHeadIndex = _headIndex;
	_headIndex = NextIndex(_headIndex);
	_itemCount--;
	_queue[oldHeadIndex] = ItemType();  // this must be done last, as queue state must be coherent when we do this
	return B_NO_ERROR;
}

template <class ItemType>
status_t
Queue<ItemType>::
RemoveTail(ItemType& returnItem)
{
	if (_itemCount == 0) return B_ERROR;
	returnItem = _queue[_tailIndex];
	return RemoveTail();
}

template <class ItemType>
status_t
Queue<ItemType>::
RemoveTail()
{
	if (_itemCount == 0) return B_ERROR;
	int removedItemIndex = _tailIndex;
	_tailIndex = PrevIndex(_tailIndex);
	_itemCount--;
	_queue[removedItemIndex] = ItemType();  // this must be done last, as queue state must be coherent when we do this
	return B_NO_ERROR;
}

template <class ItemType>
status_t
Queue<ItemType>::
GetItemAt(uint32 index, ItemType& returnItem) const
{
	if (index >= _itemCount) return B_ERROR;
	returnItem = _queue[InternalizeIndex(index)];
	return B_NO_ERROR;
}

template <class ItemType>
status_t
Queue<ItemType>::
RemoveItemAt(uint32 index, ItemType& returnItem)
{
	if (index >= _itemCount) return B_ERROR;
	returnItem = _queue[InternalizeIndex(index)];
	return RemoveItemAt(index);
}

template <class ItemType>
status_t
Queue<ItemType>::
RemoveItemAt(uint32 index)
{
	if (index >= _itemCount) return B_ERROR;

	uint32 internalizedIndex = InternalizeIndex(index);
	uint32 indexToClear;

	if (index < _itemCount / 2) {
		// item is closer to the head:  shift everything forward one, ending at the head
		while (internalizedIndex != _headIndex) {
			uint32 prev = PrevIndex(internalizedIndex);
			_queue[internalizedIndex] = _queue[prev];
			internalizedIndex = prev;
		}
		indexToClear = _headIndex;
		_headIndex = NextIndex(_headIndex);
	} else {
		// item is closer to the tail:  shift everything back one, ending at the tail
		while (internalizedIndex != _tailIndex) {
			uint32 next = NextIndex(internalizedIndex);
			_queue[internalizedIndex] = _queue[next];
			internalizedIndex = next;
		}
		indexToClear = _tailIndex;
		_tailIndex = PrevIndex(_tailIndex);
	}

	_itemCount--;
	_queue[indexToClear] = ItemType();  // this must be done last, as queue state must be coherent when we do this
	return B_NO_ERROR;
}

template <class ItemType>
status_t
Queue<ItemType>::
ReplaceItemAt(uint32 index, const ItemType& newItem)
{
	if (index >= _itemCount) return B_ERROR;
	_queue[InternalizeIndex(index)] = newItem;
	return B_NO_ERROR;
}

template <class ItemType>
status_t
Queue<ItemType>::
InsertItemAt(uint32 index, const ItemType& newItem)
{
	// Simple cases
	if (index >  _itemCount) return B_ERROR;
	if (index == _itemCount) return AddTail(newItem);
	if (index == 0)          return AddHead(newItem);

	// Harder case:  inserting into the middle of the array
	if (index < _itemCount / 2) {
		// Add a space at the front, and shift things back
		if (AddHead() != B_NO_ERROR) return B_ERROR;  // allocate an extra slot
		for (uint32 i = 0; i < index; i++) ReplaceItemAt(i, *GetItemAt(i + 1));
	} else {
		// Add a space at the rear, and shift things forward
		if (AddTail() != B_NO_ERROR) return B_ERROR;  // allocate an extra slot
		for (int32 i = ((int32)_itemCount) - 1; i > ((int32)index); i--) ReplaceItemAt(i, *GetItemAt(i - 1));
	}
	return ReplaceItemAt(index, newItem);
}

template <class ItemType>
void
Queue<ItemType>::
Clear(bool releaseCachedBuffers)
{
	if ((releaseCachedBuffers) && (_queue != _smallQueue)) {
		delete [] _queue;
		_queue = NULL;
		_queueSize = 0;
		_itemCount = 0;
	} else while (RemoveTail() == B_NO_ERROR) {
			/* empty */
		}
}

template <class ItemType>
status_t
Queue<ItemType>::
EnsureSize(uint32 size, bool setNumItems, uint32 extraPreallocs)
{
	if ((_queue == NULL) || (_queueSize < size)) {
		const uint32 sqLen = ARRAYITEMS(_smallQueue);
		uint32 temp    = size + extraPreallocs;
		uint32 newQLen = muscleMax(_initialSize, ((setNumItems) || (temp <= sqLen)) ? muscleMax(sqLen, temp) : temp);

		ItemType* newQueue = ((_queue == _smallQueue) || (newQLen > sqLen)) ? newnothrow_array(ItemType, newQLen) : _smallQueue;
		if (newQueue == NULL) {
			return B_ERROR;
		}
		if (newQueue == _smallQueue) newQLen = sqLen;

		for (uint32 i = 0; i < _itemCount; i++) (void) GetItemAt(i, newQueue[i]); // we know that (_itemCount < size)
		if (setNumItems) _itemCount = size;
		_headIndex = 0;
		_tailIndex = _itemCount - 1;

		if (_queue == _smallQueue) {
			ItemType blank = ItemType();
			for (uint32 i = 0; i < sqLen; i++) _smallQueue[i] = blank;
		} else delete [] _queue;

		_queue = newQueue;
		_queueSize = newQLen;
	}

	if (setNumItems) {
		// Force ourselves to contain exactly the required number of items
		if (_itemCount < size) {
			// We can do this quickly because the "new" items are already initialized properly
			_tailIndex = (_tailIndex + (size - _itemCount)) % _queueSize;
			_itemCount = size;
		} else while (_itemCount > size) (void) RemoveTail(); // Gotta overwrite the "removed" items, so this is a bit slower
	}

	return B_NO_ERROR;
}

template <class ItemType>
int32
Queue<ItemType>::
IndexOf(const ItemType& item) const
{
	if (_queue) for (int i = ((int)GetNumItems()) - 1; i >= 0; i--) if (*GetItemAt(i) == item) return i;
	return -1;
}


template <class ItemType>
void
Queue<ItemType>::
Swap(uint32 fromIndex, uint32 toIndex)
{
	ItemType temp = *(GetItemAt(fromIndex));
	ReplaceItemAt(fromIndex, *(GetItemAt(toIndex)));
	ReplaceItemAt(toIndex,   temp);
}

template <class ItemType>
void
Queue<ItemType>::
Sort(ItemCompareFunc compareFunc, uint32 from, uint32 to, void* cookie)
{
	uint32 size = GetNumItems();
	if (to > size) to = size;
	if (to > from) {
		if (to < from + 12) {
			// too easy, just do a bubble sort (base case)
			if (to > from + 1) {
				for (uint32 i = from + 1; i < to; i++) {
					for (uint32 j = i; j > from; j--) {
						int ret = compareFunc(*(GetItemAt(j)), *(GetItemAt(j - 1)), cookie);
						if (ret < 0) Swap(j, j - 1);
						else break;
					}
				}
			}
		} else {
			// Okay, do the real thing
			uint32 middle = (from + to) / 2;
			Sort(compareFunc, from, middle, cookie);
			Sort(compareFunc, middle, to, cookie);
			Merge(compareFunc, from, middle, to, middle - from, to - middle, cookie);
		}
	}
}

template <class ItemType>
void
Queue<ItemType>::
ReverseItemOrdering(uint32 from, uint32 to)
{
	uint32 size = GetNumItems();
	if (size > 0) {
		to--;  // make it inclusive
		if (to >= size) to = size - 1;
		while (from < to) Swap(from++, to--);
	}
}

template <class ItemType>
status_t
Queue<ItemType>::
RemoveFirstInstanceOf(const ItemType& val)
{
	uint32 ni = GetNumItems();
	for (uint32 i = 0; i < ni; i++) if ((*this)[i] == val) return RemoveItemAt(i);
	return B_ERROR;
}

template <class ItemType>
status_t
Queue<ItemType>::
RemoveLastInstanceOf(const ItemType& val)
{
	for (int32 i = ((int32)GetNumItems()) - 1; i >= 0; i--) if ((*this)[i] == val) return RemoveItemAt(i);
	return B_ERROR;
}

template <class ItemType>
uint32
Queue<ItemType>::
RemoveAllInstancesOf(const ItemType& val)
{
	// Efficiently collapse all non-matching slots up to the top of the list
	uint32 ret      = 0;
	uint32 writeTo  = 0;
	uint32 origSize = GetNumItems();
	for (uint32 readFrom = 0; readFrom < origSize; readFrom++) {
		const ItemType& nextRead = (*this)[readFrom];
		if (nextRead == val) ret++;
		else {
			if (readFrom > writeTo) (*this)[writeTo] = nextRead;
			writeTo++;
		}
	}

	// Now get rid of any now-surplus slots
	for (; writeTo < origSize; writeTo++) RemoveTail();

	return ret;
}

template <class ItemType>
void
Queue<ItemType>::
Merge(ItemCompareFunc compareFunc, uint32 from, uint32 pivot, uint32 to, uint32 len1, uint32 len2, void* cookie)
{
	if ((len1) && (len2)) {
		if (len1 + len2 == 2) {
			if (compareFunc(*(GetItemAt(pivot)), *(GetItemAt(from)), cookie) < 0) Swap(pivot, from);
		} else {
			uint32 first_cut, second_cut;
			uint32 len11, len22;
			if (len1 > len2) {
				len11      = len1 / 2;
				first_cut  = from + len11;
				second_cut = Lower(compareFunc, pivot, to, *GetItemAt(first_cut), cookie);
				len22      = second_cut - pivot;
			} else {
				len22      = len2 / 2;
				second_cut = pivot + len22;
				first_cut  = Upper(compareFunc, from, pivot, *GetItemAt(second_cut), cookie);
				len11      = first_cut - from;
			}

			// do a rotation
			if ((pivot != first_cut) && (pivot != second_cut)) {
				// find the greatest common denominator of (pivot-first_cut) and (second_cut-first_cut)
				uint32 n = pivot - first_cut;
				{
					uint32 m = second_cut - first_cut;
					while (n != 0) {
						uint32 t = m % n;
						m = n;
						n = t;
					}
					n = m;
				}

				while (n--) {
					const ItemType val = *GetItemAt(first_cut + n);
					uint32 shift = pivot - first_cut;
					uint32 p1 = first_cut + n;
					uint32 p2 = p1 + shift;
					while (p2 != first_cut + n) {
						ReplaceItemAt(p1, *GetItemAt(p2));
						p1 = p2;
						if (second_cut - p2 > shift) p2 += shift;
						else p2  = first_cut + (shift - (second_cut - p2));
					}
					ReplaceItemAt(p1, val);
				}
			}

			uint32 new_mid = first_cut + len22;
			Merge(compareFunc, from,    first_cut,  new_mid, len11,        len22,        cookie);
			Merge(compareFunc, new_mid, second_cut, to,      len1 - len11, len2 - len22, cookie);
		}
	}
}


template <class ItemType>
uint32
Queue<ItemType>::
Lower(ItemCompareFunc compareFunc, uint32 from, uint32 to, const ItemType& val, void* cookie) const
{
	if (to > from) {
		uint32 len = to - from;
		while (len > 0) {
			uint32 half = len / 2;
			uint32 mid  = from + half;
			if (compareFunc(*(GetItemAt(mid)), val, cookie) < 0) {
				from = mid + 1;
				len  = len - half - 1;
			} else len = half;
		}
	}
	return from;
}

template <class ItemType>
uint32
Queue<ItemType>::
Upper(ItemCompareFunc compareFunc, uint32 from, uint32 to, const ItemType& val, void* cookie) const
{
	if (to > from) {
		uint32 len = to - from;
		while (len > 0) {
			uint32 half = len / 2;
			uint32 mid  = from + half;
			if (compareFunc(val, *(GetItemAt(mid)), cookie) < 0) len = half;
			else {
				from = mid + 1;
				len  = len - half - 1;
			}
		}
	}
	return from;
}

template <class ItemType>
const ItemType *
Queue<ItemType> :: GetArrayPointerAux(uint32 whichArray, uint32& retLength) const
{
	if (_itemCount > 0) {
		switch (whichArray) {
			case 0:
				retLength = (_headIndex <= _tailIndex) ? (_tailIndex - _headIndex) + 1 : (_queueSize - _headIndex);
				return &_queue[_headIndex];
				break;

			case 1:
				if (_headIndex > _tailIndex) {
					retLength = _tailIndex + 1;
					return &_queue[0];
				}
				break;
		}
	}
	return NULL;
}

template <class ItemType>
void
Queue<ItemType>::SwapContents(Queue<ItemType> & that)
{
	bool thisSmall = (_queue == _smallQueue);
	bool thatSmall = (that._queue == that._smallQueue);

	if ((thisSmall) && (thatSmall)) {
		// First, move any extra items from the longer queue to the shorter one...
		uint32 commonSize = muscleMin(GetNumItems(), that.GetNumItems());
		int32 sizeDiff    = ((int32)GetNumItems()) - ((int32)that.GetNumItems());
		Queue<ItemType> & copyTo   = (sizeDiff > 0) ? that : *this;
		Queue<ItemType> & copyFrom = (sizeDiff > 0) ? *this : that;
		(void) copyTo.AddTail(copyFrom, commonSize);   // guaranteed not to fail
		(void) copyFrom.EnsureSize(commonSize, true);  // remove the copied items from (copyFrom)

		// Then just swap the elements that are present in both arrays
		for (uint32 i = 0; i < commonSize; i++) muscleSwap((*this)[i], that[i]);
	} else if (thisSmall) SwapContentsAux(that);
	else if (thatSmall) that.SwapContentsAux(*this);
	else {
		// this case is easy, we can just swizzle the pointers around
		muscleSwap(_queue,     that._queue);
		muscleSwap(_queueSize, that._queueSize);
		muscleSwap(_headIndex, that._headIndex);
		muscleSwap(_tailIndex, that._tailIndex);
		muscleSwap(_itemCount, that._itemCount);
	}
}

template <class ItemType>
void
Queue<ItemType>::SwapContentsAux(Queue<ItemType> & largeThat)
{
	// First, copy over our (small) contents to his static buffer
	uint32 ni = GetNumItems();
	for (uint32 i = 0; i < ni; i++) largeThat._smallQueue[i] = (*this)[i];

	// Now adopt his dynamic buffer
	_queue     = largeThat._queue;
	_queueSize = largeThat._queueSize;
	_headIndex = largeThat._headIndex;
	_tailIndex = largeThat._tailIndex;

	// And point him back at his static buffer
	if (ni > 0) {
		largeThat._queue     = largeThat._smallQueue;
		largeThat._queueSize = ARRAYITEMS(largeThat._smallQueue);
		largeThat._headIndex = 0;
		largeThat._tailIndex = ni - 1;
	} else {
		largeThat._queue     = NULL;
		largeThat._queueSize = 0;
		// headIndex and tailIndex are undefined in this case anyway
	}

	muscleSwap(_itemCount, largeThat._itemCount);
}

template <class ItemType>
bool
Queue<ItemType>::StartsWith(const Queue<ItemType> & prefixQueue) const
{
	if (prefixQueue.GetNumItems() > GetNumItems()) return false;
	uint32 prefixQueueLen = prefixQueue.GetNumItems();
	for (uint32 i = 0; i < prefixQueueLen; i++) if (!(prefixQueue[i] == (*this)[i])) return false;
	return true;
}

template <class ItemType>
bool
Queue<ItemType>::EndsWith(const Queue<ItemType> & suffixQueue) const
{
	if (suffixQueue.GetNumItems() > GetNumItems()) return false;
	int32 lastToCheck = GetNumItems() - suffixQueue.GetNumItems();
	for (int32 i = GetNumItems() - 1; i >= lastToCheck; i--) if (!(suffixQueue[i] == (*this)[i])) return false;
	return true;
}


#endif

