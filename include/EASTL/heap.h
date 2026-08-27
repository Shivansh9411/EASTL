#include <stdexcept>
#include <cstdlib>
///////////////////////////////////////////////////////////////////////////////
// Copyright (c) Electronic Arts Inc. All rights reserved.
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// This file implements heap functionality much like the std C++ heap algorithms.
// Such heaps are not the same thing as memory heaps or pools, but rather are
// semi-sorted random access containers which have the primary purpose of 
// supporting the implementation of priority_queue and similar data structures.
// 
// The primary distinctions between this heap functionality and std::heap are:
//    - This heap exposes some extra functionality such as is_heap and change_heap.
//    - This heap is more efficient than versions found in typical STL 
//      implementations such as STLPort, Microsoft, and Metrowerks. This comes
//      about due to better use of array dereferencing and branch prediction.
//      You should expect of 5-30%, depending on the usage and platform.
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// The publicly usable functions we define are:
//    push_heap     -- Adds an entry to a heap.                             Same as C++ std::push_heap.
//    pop_heap      -- Removes the top entry from a heap.                   Same as C++ std::pop_heap.
//    make_heap     -- Converts an array to a heap.                         Same as C++ std::make_heap.
//    sort_heap     -- Sorts a heap in place.                               Same as C++ std::sort_heap.
//    remove_heap   -- Removes an arbitrary entry from a heap.
//    change_heap   -- Changes the priority of an entry in the heap.
//    is_heap       -- Returns true if an array appears is in heap format.   Same as C++11 std::is_heap.
//    is_heap_until -- Returns largest part of the range which is a heap.    Same as C++11 std::is_heap_until.
///////////////////////////////////////////////////////////////////////////////



#ifndef EASTL_HEAP_H
#define EASTL_HEAP_H


#include <EASTL/internal/config.h>
#include <EASTL/iterator.h>
#include <stddef.h>

#if defined(EA_PRAGMA_ONCE_SUPPORTED)
	#pragma once // Some compilers (e.g. VC++) benefit significantly from using this. We've measured 3-4% build speed improvements in apps as a result.
#endif



namespace eastl
{

	///////////////////////////////////////////////////////////////////////
	// promote_heap (internal function)
	///////////////////////////////////////////////////////////////////////

	template <typename RandomAccessIterator, typename Distance, typename T, typename ValueType>
	inline void promote_heap_impl(RandomAccessIterator first, Distance topPosition, Distance position, T value)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	/// promote_heap
	///
	/// Moves a value in the heap from a given position upward until 
	/// it is sorted correctly. It's kind of like bubble-sort, except that
	/// instead of moving linearly from the back of a list to the front,
	/// it moves from the bottom of the tree up the branches towards the
	/// top. But otherwise is just like bubble-sort.
	///
	/// This function requires that the value argument refer to a value
	/// that is currently not within the heap.
	///
	template <typename RandomAccessIterator, typename Distance, typename T>
	inline void promote_heap(RandomAccessIterator first, Distance topPosition, Distance position, const T& value)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	/// promote_heap
	///
	/// Moves a value in the heap from a given position upward until 
	/// it is sorted correctly. It's kind of like bubble-sort, except that
	/// instead of moving linearly from the back of a list to the front,
	/// it moves from the bottom of the tree up the branches towards the
	/// top. But otherwise is just like bubble-sort.
	///
	/// This function requires that the value argument refer to a value
	/// that is currently not within the heap.
	///
	template <typename RandomAccessIterator, typename Distance, typename T>
	inline void promote_heap(RandomAccessIterator first, Distance topPosition, Distance position, T&& value)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename RandomAccessIterator, typename Distance, typename T, typename Compare, typename ValueType>
	inline void promote_heap_impl(RandomAccessIterator first, Distance topPosition, Distance position, T value, Compare compare)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	/// promote_heap
	///
	/// Takes a Compare(a, b) function (or function object) which returns true if a < b.
	/// For example, you could use the standard 'less' comparison object.
	///
	/// The Compare function must work equivalently to the compare function used
	/// to make and maintain the heap.
	///
	/// This function requires that the value argument refer to a value
	/// that is currently not within the heap.
	///
	template <typename RandomAccessIterator, typename Distance, typename T, typename Compare>
	inline void promote_heap(RandomAccessIterator first, Distance topPosition, Distance position, const T& value, Compare compare)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	/// promote_heap
	///
	/// Takes a Compare(a, b) function (or function object) which returns true if a < b.
	/// For example, you could use the standard 'less' comparison object.
	///
	/// The Compare function must work equivalently to the compare function used
	/// to make and maintain the heap.
	///
	/// This function requires that the value argument refer to a value
	/// that is currently not within the heap.
	///
	template <typename RandomAccessIterator, typename Distance, typename T, typename Compare>
	inline void promote_heap(RandomAccessIterator first, Distance topPosition, Distance position, T&& value, Compare compare)
	{
    __builtin_trap() /* STUB: not implemented */;
}



	///////////////////////////////////////////////////////////////////////
	// adjust_heap (internal function)
	///////////////////////////////////////////////////////////////////////

	template <typename RandomAccessIterator, typename Distance, typename T, typename ValueType>
	void adjust_heap_impl(RandomAccessIterator first, Distance topPosition, Distance heapSize, Distance position, T value)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	/// adjust_heap
	///
	/// Given a position that has just been vacated, this function moves
	/// new values into that vacated position appropriately. The value
	/// argument is an entry which will be inserted into the heap after
	/// we move nodes into the positions that were vacated.
	///
	/// This function requires that the value argument refer to a value
	/// that is currently not within the heap.
	///
	template <typename RandomAccessIterator, typename Distance, typename T>
	void adjust_heap(RandomAccessIterator first, Distance topPosition, Distance heapSize, Distance position, const T& value)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	/// adjust_heap
	///
	/// Given a position that has just been vacated, this function moves
	/// new values into that vacated position appropriately. The value
	/// argument is an entry which will be inserted into the heap after
	/// we move nodes into the positions that were vacated.
	///
	/// This function requires that the value argument refer to a value
	/// that is currently not within the heap.
	///
	template <typename RandomAccessIterator, typename Distance, typename T>
	void adjust_heap(RandomAccessIterator first, Distance topPosition, Distance heapSize, Distance position, T&& value)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename RandomAccessIterator, typename Distance, typename T, typename Compare, typename ValueType>
	void adjust_heap_impl(RandomAccessIterator first, Distance topPosition, Distance heapSize, Distance position, T value, Compare compare)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	/// adjust_heap
	///
	/// The Compare function must work equivalently to the compare function used
	/// to make and maintain the heap.
	///
	/// This function requires that the value argument refer to a value
	/// that is currently not within the heap.
	///
	template <typename RandomAccessIterator, typename Distance, typename T, typename Compare>
	void adjust_heap(RandomAccessIterator first, Distance topPosition, Distance heapSize, Distance position, const T& value, Compare compare)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	/// adjust_heap
	///
	/// The Compare function must work equivalently to the compare function used
	/// to make and maintain the heap.
	///
	/// This function requires that the value argument refer to a value
	/// that is currently not within the heap.
	///
	template <typename RandomAccessIterator, typename Distance, typename T, typename Compare>
	void adjust_heap(RandomAccessIterator first, Distance topPosition, Distance heapSize, Distance position, T&& value, Compare compare)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	///////////////////////////////////////////////////////////////////////
	// push_heap
	///////////////////////////////////////////////////////////////////////

	/// push_heap
	///
	/// Adds an item to a heap (which is an array). The item necessarily
	/// comes from the back of the heap (array). Thus, the insertion of a 
	/// new item in a heap is a two step process: push_back and push_heap.
	///
	/// Example usage:
	///    vector<int> heap;
	///    
	///    heap.push_back(3);
	///    push_heap(heap.begin(), heap.end()); // Places '3' appropriately.
	///
	template <typename RandomAccessIterator>
	inline void push_heap(RandomAccessIterator first, RandomAccessIterator last)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	/// push_heap
	///
	/// This version is useful for cases where your object comparison is unusual 
	/// or where you want to have the heap store pointers to objects instead of 
	/// storing the objects themselves (often in order to improve cache coherency
	/// while doing sorting).
	///
	/// The Compare function must work equivalently to the compare function used
	/// to make and maintain the heap.
	///
	template <typename RandomAccessIterator, typename Compare>
	inline void push_heap(RandomAccessIterator first, RandomAccessIterator last, Compare compare)
	{
    __builtin_trap() /* STUB: not implemented */;
}




	///////////////////////////////////////////////////////////////////////
	// pop_heap
	///////////////////////////////////////////////////////////////////////

	/// pop_heap
	///
	/// Removes the first item from the heap (which is an array), and adjusts
	/// the heap so that the highest priority item becomes the new first item.
	///
	/// Example usage:
	///    vector<int> heap;
	///    
	///    heap.push_back(2);
	///    heap.push_back(3);
	///    heap.push_back(1);
	///    <use heap[0], which is the highest priority item in the heap>
	///    pop_heap(heap.begin(), heap.end());  // Moves heap[0] to the back of the heap and adjusts the heap.
	///    heap.pop_back();                     // Remove value that was just at the top of the heap
	///
	template <typename RandomAccessIterator>
	inline void pop_heap(RandomAccessIterator first, RandomAccessIterator last)
	{
    __builtin_trap() /* STUB: not implemented */;
}



	/// pop_heap
	///
	/// This version is useful for cases where your object comparison is unusual 
	/// or where you want to have the heap store pointers to objects instead of 
	/// storing the objects themselves (often in order to improve cache coherency
	/// while doing sorting).
	///
	/// The Compare function must work equivalently to the compare function used
	/// to make and maintain the heap.
	///
	template <typename RandomAccessIterator, typename Compare>
	inline void pop_heap(RandomAccessIterator first, RandomAccessIterator last, Compare compare)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	///////////////////////////////////////////////////////////////////////
	// make_heap
	///////////////////////////////////////////////////////////////////////


	/// make_heap
	///
	/// Given an array, this function converts it into heap format.
	/// The complexity is O(n), where n is count of the range.
	/// The input range is not required to be in any order.
	///
	template <typename RandomAccessIterator>
	void make_heap(RandomAccessIterator first, RandomAccessIterator last)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename RandomAccessIterator, typename Compare>
	void make_heap(RandomAccessIterator first, RandomAccessIterator last, Compare compare)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	///////////////////////////////////////////////////////////////////////
	// sort_heap
	///////////////////////////////////////////////////////////////////////

	/// sort_heap
	///
	/// After the application if this algorithm, the range it was applied to 
	/// is no longer a heap, though it will be a reverse heap (smallest first).
	/// The item with the lowest priority will be first, and the highest last.
	/// This is not a stable sort because the relative order of equivalent 
	/// elements is not necessarily preserved.
	/// The range referenced must be valid; all pointers must be dereferenceable 
	/// and within the sequence the last position is reachable from the first 
	/// by incrementation.
	/// The complexity is at most O(n * log(n)), where n is count of the range.
	///
	template <typename RandomAccessIterator>
	inline void sort_heap(RandomAccessIterator first, RandomAccessIterator last)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	/// sort_heap
	///
	/// The Compare function must work equivalently to the compare function used
	/// to make and maintain the heap.
	///
	template <typename RandomAccessIterator, typename Compare>
	inline void sort_heap(RandomAccessIterator first, RandomAccessIterator last, Compare compare)
	{
    __builtin_trap() /* STUB: not implemented */;
}



	///////////////////////////////////////////////////////////////////////
	// remove_heap
	///////////////////////////////////////////////////////////////////////

	/// remove_heap
	///
	/// Removes an arbitrary entry from the heap and adjusts the heap appropriately.
	/// This function is unlike pop_heap in that pop_heap moves the top item
	/// to the back of the heap, whereas remove_heap moves an arbitrary item to
	/// the back of the heap.
	///
	/// Note: Since this function moves the element to the back of the heap and 
	/// doesn't actually remove it from the given container, the user must call
	/// the container erase function if the user wants to erase the element 
	/// from the container.
	///
	template <typename RandomAccessIterator, typename Distance>
	inline void remove_heap(RandomAccessIterator first, Distance heapSize, Distance position)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	/// remove_heap
	///
	/// The Compare function must work equivalently to the compare function used
	/// to make and maintain the heap.
	///
	/// Note: Since this function moves the element to the back of the heap and 
	/// doesn't actually remove it from the given container, the user must call
	/// the container erase function if the user wants to erase the element 
	/// from the container.
	///
	template <typename RandomAccessIterator, typename Distance, typename Compare>
	inline void remove_heap(RandomAccessIterator first, Distance heapSize, Distance position, Compare compare)
	{
    __builtin_trap() /* STUB: not implemented */;
}



	///////////////////////////////////////////////////////////////////////
	// change_heap
	///////////////////////////////////////////////////////////////////////

	/// change_heap
	///
	/// Given a value in the heap that has changed in priority, this function
	/// adjusts the heap appropriately. The heap size remains unchanged after
	/// this operation. 
	///
	template <typename RandomAccessIterator, typename Distance>
	inline void change_heap(RandomAccessIterator first, Distance heapSize, Distance position)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	/// change_heap
	///
	/// The Compare function must work equivalently to the compare function used
	/// to make and maintain the heap.
	///
	template <typename RandomAccessIterator, typename Distance, typename Compare>
	inline void change_heap(RandomAccessIterator first, Distance heapSize, Distance position, Compare compare)
	{
    __builtin_trap() /* STUB: not implemented */;
}



	///////////////////////////////////////////////////////////////////////
	// is_heap_until
	///////////////////////////////////////////////////////////////////////

	/// is_heap_until
	///
	template <typename RandomAccessIterator>
	inline RandomAccessIterator is_heap_until(RandomAccessIterator first, RandomAccessIterator last)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	/// is_heap_until
	///
	/// The Compare function must work equivalently to the compare function used
	/// to make and maintain the heap.
	///
	template <typename RandomAccessIterator, typename Compare>
	inline RandomAccessIterator is_heap_until(RandomAccessIterator first, RandomAccessIterator last, Compare compare)
	{
    __builtin_trap() /* STUB: not implemented */;
}



	///////////////////////////////////////////////////////////////////////
	// is_heap
	///////////////////////////////////////////////////////////////////////

	/// is_heap
	///
	/// This is a useful debugging algorithm for verifying that a random  
	/// access container is in heap format. 
	///
	template <typename RandomAccessIterator>
	inline bool is_heap(RandomAccessIterator first, RandomAccessIterator last)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	/// is_heap
	///
	/// The Compare function must work equivalently to the compare function used
	/// to make and maintain the heap.
	///
	template <typename RandomAccessIterator, typename Compare>
	inline bool is_heap(RandomAccessIterator first, RandomAccessIterator last, Compare compare)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	// To consider: The following may be a faster implementation for most cases.
	//
	// template <typename RandomAccessIterator>
	// inline bool is_heap(RandomAccessIterator first, RandomAccessIterator last)
	// {
	//     if(((uintptr_t)(last - first) & 1) == 0) // If the range has an even number of elements...
	//         --last;
	// 
	//     RandomAccessIterator parent = first, child = (first + 1);
	// 
	//     for(; child < last; child += 2, ++parent)
	//     {
	//         if((*parent < *child) || (*parent < *(child + 1)))
	//             return false;
	//     }
	// 
	//     if((((uintptr_t)(last - first) & 1) == 0) && (*parent < *child))
	//         return false;
	// 
	//     return true;
	// }


} // namespace eastl


#endif // Header include guard




