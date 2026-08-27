#include <stdexcept>
#include <cstdlib>
///////////////////////////////////////////////////////////////////////////////
// Copyright (c) Electronic Arts Inc. All rights reserved.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// This file implements sorting algorithms. Some of these are equivalent to 
// std C++ sorting algorithms, while others don't have equivalents in the 
// C++ standard. We implement the following sorting algorithms:
//    is_sorted             -- 
//    sort                  -- Unstable.    The implementation of this is mapped to quick_sort by default.
//    quick_sort            -- Unstable.    This is actually an intro-sort (quick sort with switch to insertion sort).
//    tim_sort              -- Stable.
//    tim_sort_buffer       -- Stable.
//    partial_sort          -- Unstable.
//    insertion_sort        -- Stable. 
//    shell_sort            -- Unstable.
//    heap_sort             -- Unstable. 
//    stable_sort           -- Stable.      The implementation of this is simply mapped to merge_sort.
//    merge                 -- 
//    merge_sort            -- Stable. 
//    merge_sort_buffer     -- Stable. 
//    nth_element           -- Unstable.
//    radix_sort            -- Stable.      Important and useful sort for integral data, and faster than all others for this.
//    comb_sort             -- Unstable.    Possibly the best combination of small code size but fast sort.
//    bubble_sort           -- Stable.      Useful in practice for sorting tiny sets of data (<= 10 elements).
//    selection_sort*       -- Unstable.
//    shaker_sort*          -- Stable.
//    bucket_sort*          -- Stable. 
//
// * Found in sort_extra.h.
//
// Additional sorting and related algorithms we may want to implement:
//    partial_sort_copy     This would be like the std STL version.
//    paritition            This would be like the std STL version. This is not categorized as a sort routine by the language standard.
//    stable_partition      This would be like the std STL version.
//    counting_sort         Maybe we don't want to implement this.
//
//////////////////////////////////////////////////////////////////////////////


#ifndef EASTL_SORT_H
#define EASTL_SORT_H


#include <EASTL/internal/config.h>
#include <EASTL/internal/move_help.h>
#include <EASTL/bit.h>
#include <EASTL/iterator.h>
#include <EASTL/memory.h>
#include <EASTL/algorithm.h>
#include <EASTL/functional.h>
#include <EASTL/heap.h>
#include <EASTL/allocator.h>
#include <EASTL/memory.h>


#if defined(EA_PRAGMA_ONCE_SUPPORTED)
	#pragma once // Some compilers (e.g. VC++) benefit significantly from using this. We've measured 3-4% build speed improvements in apps as a result.
#endif


// EASTL_PLATFORM_PREFERRED_ALIGNMENT
//
// Allows for slightly faster buffers in some cases.
//
#if !defined(EASTL_PLATFORM_PREFERRED_ALIGNMENT)
	#if defined(EA_PROCESSOR_ARM)
		#define EASTL_PLATFORM_PREFERRED_ALIGNMENT 8
	#else
		#define EASTL_PLATFORM_PREFERRED_ALIGNMENT 16
	#endif
#endif


namespace eastl
{

	/// is_sorted
	///
	/// Returns true if the range [first, last) is sorted.
	/// An empty range is considered to be sorted.
	/// To test if a range is reverse-sorted, use 'greater' as the comparison 
	/// instead of 'less'.
	///
	/// Example usage:
	///    vector<int> intArray;
	///    bool bIsSorted        = is_sorted(intArray.begin(), intArray.end());
	///    bool bIsReverseSorted = is_sorted(intArray.begin(), intArray.end(), greater<int>());
	///
	template <typename ForwardIterator, typename StrictWeakOrdering>
	bool is_sorted(ForwardIterator first, ForwardIterator last, StrictWeakOrdering compare)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename ForwardIterator>
	inline bool is_sorted(ForwardIterator first, ForwardIterator last)
	{
    __builtin_trap() /* STUB: not implemented */;
}



	/// is_sorted_until
	///
	/// Returns an iterator to the first element in the range [first,last) which does not follow an ascending order.
	/// The range between first and the iterator returned is sorted.
	/// If the entire range is sorted, the function returns last.
	/// The elements are compared using operator< for the first version, and comp for the second.
	///
	/// Example usage:
	///     vector<int> intArray;
	///     vector<int>::iterator unsorted_element = is_sorted_until(eastl::end(intArray), eastl::end(intArray));
	///     vector<int>::iterator unsorted_element_with_user_compare = is_sorted_until(eastl::end(intArray), eastl::end(intArray), eastl::less<int>());
	///
	template<typename ForwardIterator>
	ForwardIterator is_sorted_until(ForwardIterator first, ForwardIterator last)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template<typename ForwardIterator, typename Compare>
	ForwardIterator is_sorted_until(ForwardIterator first, ForwardIterator last, Compare compare)
	{
    __builtin_trap() /* STUB: not implemented */;
}



	/// merge
	///
	/// This function merges two sorted input sorted ranges into a result sorted range.
	/// This merge is stable in that no element from the first range will be changed
	/// in order relative to other elements from the first range.
	///
	template <typename InputIterator1, typename InputIterator2, typename OutputIterator, typename Compare>
	OutputIterator merge(InputIterator1 first1, InputIterator1 last1, InputIterator2 first2, InputIterator2 last2, OutputIterator result, Compare compare)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename InputIterator1, typename InputIterator2, typename OutputIterator>
	inline OutputIterator merge(InputIterator1 first1, InputIterator1 last1, InputIterator2 first2, InputIterator2 last2, OutputIterator result)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	//////////////////////////////////////////////////////////////////////////////
	/// insertion_sort
	///
	/// insertion_sort is an O(n^2) stable sorting algorithm that starts at the
	/// (k + 1) element and assumes the first (k) elements are sorted.
	/// Then copy_backwards from (k + 1) to the begining any elements where the
	/// (k + 1) element is less than [0, k] elements. The position of k when
	/// (k + 1) element is not less than k is the sorted position of the (k + 1) element.
	///
	/// Example With Intermediate Steps:
	/// (k + 1) == 2 : [3, 2, 1] -> [3, 3, 1] -> [2, 3, 1]
	/// (k + 1) == 1 : [2, 3, 1] -> [2, 3, 3] -> [2, 2, 3] -> [1, 2, 3]
	///              : [1, 2, 3]
	template <typename BidirectionalIterator, typename StrictWeakOrdering>
	void insertion_sort(BidirectionalIterator first, BidirectionalIterator last, StrictWeakOrdering compare)
	{
    __builtin_trap() /* STUB: not implemented */;
} // insertion_sort


	template <typename BidirectionalIterator>
	void insertion_sort(BidirectionalIterator first, BidirectionalIterator last)
	{
    __builtin_trap() /* STUB: not implemented */;
} // insertion_sort


	/// shell_sort
	///
	/// Implements the ShellSort algorithm. This algorithm is a serious algorithm for larger 
	/// data sets, as reported by Sedgewick in his discussions on QuickSort. Note that shell_sort
	/// requires a random access iterator, which usually means an array (eg. vector, deque).
	/// ShellSort has good performance with presorted sequences.
	/// The term "shell" derives from the name of the inventor, David Shell.
	///
	/// To consider: Allow the user to specify the "h-sequence" array.
	///
	template <typename RandomAccessIterator, typename StrictWeakOrdering>
	void shell_sort(RandomAccessIterator first, RandomAccessIterator last, StrictWeakOrdering compare)
	{
    __builtin_trap() /* STUB: not implemented */;
} // shell_sort

	template <typename RandomAccessIterator>
	inline void shell_sort(RandomAccessIterator first, RandomAccessIterator last)
	{
    __builtin_trap() /* STUB: not implemented */;
}



	/// heap_sort
	///
	/// Implements the HeapSort algorithm. 
	/// Note that heap_sort requires a random access iterator, which usually means 
	/// an array (eg. vector, deque).
	///
	template <typename RandomAccessIterator, typename StrictWeakOrdering>
	void heap_sort(RandomAccessIterator first, RandomAccessIterator last, StrictWeakOrdering compare)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename RandomAccessIterator>
	inline void heap_sort(RandomAccessIterator first, RandomAccessIterator last)
	{
    __builtin_trap() /* STUB: not implemented */;
}



	namespace Internal
	{
		// Sorts a range whose initial (start - first) entries are already sorted.
		// This function is a useful helper to the tim_sort function.
		// This is the same as insertion_sort except that it has a start parameter which indicates
		// where the start of the unsorted data is.
		template <typename BidirectionalIterator, typename StrictWeakOrdering>
		void insertion_sort_already_started(BidirectionalIterator first, BidirectionalIterator last, BidirectionalIterator start, StrictWeakOrdering compare)
		{
    __builtin_trap() /* STUB: not implemented */;
}
	}



	/// merge_sort_buffer
	///
	/// Implements the MergeSort algorithm with a user-supplied buffer.
	/// The input buffer must be able to hold a number of items equal to 'last - first'.
	/// Note that merge_sort_buffer requires a random access iterator, which usually means 
	/// an array (eg. vector, deque).
	///
	/// The algorithm used for merge sort is not the standard merge sort.  It has been modified
	/// to improve performance for data that is already partially sorted.  In fact, if data
	/// is completely sorted, then performance is O(n), but even data with partially sorted
	/// regions can benefit from the modifications.
	///
	/// 'InsertionSortLimit' specifies a size limit for which the algorithm will use insertion sort.
	/// Due to the overhead of merge sort, it is often faster to use insertion sort once the size of a region
	/// is fairly small.  However, insertion sort is not as efficient (in terms of assignments orcomparisons)
	/// so choosing a value that is too large will reduce performance.  Generally a value of 16 to 32 is reasonable,
	/// but the best choose will depend on the data being sorted.
	template <typename RandomAccessIterator, typename T, typename StrictWeakOrdering, typename difference_type, int InsertionSortLimit>
	class MergeSorter
	{
	public:
		static void sort(RandomAccessIterator first, RandomAccessIterator last, T* pBuffer, StrictWeakOrdering compare)
		{
    __builtin_trap() /* STUB: not implemented */;
}

	private:
		static_assert(InsertionSortLimit > 1, "Sequences of length 1 are already sorted.  Use a larger value for InsertionSortLimit");

		enum ResultLocation
		{
			RL_SourceRange,	// i.e. result is in the range defined by [first, last)
			RL_Buffer,		// i.e. result is in pBuffer
		};

		// sort_impl
		//
		// This sort routine sorts the data in [first, last) and places the result in pBuffer or in the original range of the input.  The actual
		// location of the data is indicated by the enum returned.
		// 
		// lastSortedEnd is used to specify a that data in the range [first, first + lastSortedEnd] is already sorted.  This information is used
		// to avoid unnecessary merge sorting of already sorted data.  lastSortedEnd is a hint, and can be an under estimate of the sorted elements
		// (i.e. it is legal to pass 0).
		static ResultLocation sort_impl(RandomAccessIterator first, RandomAccessIterator last, T* pBuffer, difference_type lastSortedEnd, StrictWeakOrdering compare)
		{
    __builtin_trap() /* STUB: not implemented */;
}

		// merge_halves
		//
		// Merge two sorted regions of elements.
		// The inputs to this method effectively define two large buffers.  The variables 'firstHalfLocation' and 'secondHalfLocation' define where the data to be
		// merged is located within the two buffers.  It is entirely possible that the two areas to be merged could be entirely located in either of the larger buffers.
		// Upon returning the merged results will be in one of the two buffers (indicated by the return result).
		static ResultLocation merge_halves(RandomAccessIterator first, RandomAccessIterator last, difference_type nMid, T* pBuffer, ResultLocation firstHalfLocation, ResultLocation secondHalfLocation, StrictWeakOrdering compare)
		{
    __builtin_trap() /* STUB: not implemented */;
}

	};


	template <typename RandomAccessIterator, typename T, typename StrictWeakOrdering>
	void merge_sort_buffer(RandomAccessIterator first, RandomAccessIterator last, T* pBuffer, StrictWeakOrdering compare)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename RandomAccessIterator, typename T>
	inline void merge_sort_buffer(RandomAccessIterator first, RandomAccessIterator last, T* pBuffer)
	{
    __builtin_trap() /* STUB: not implemented */;
}



	/// merge_sort
	///
	/// Implements the MergeSort algorithm.
	/// This algorithm allocates memory via the user-supplied allocator. Use merge_sort_buffer
	/// function if you want a version which doesn't allocate memory.
	/// Note that merge_sort requires a random access iterator, which usually means 
	/// an array (eg. vector, deque).
	/// 
	template <typename RandomAccessIterator, typename Allocator, typename StrictWeakOrdering>
	void merge_sort(RandomAccessIterator first, RandomAccessIterator last, Allocator& allocator, StrictWeakOrdering compare)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename RandomAccessIterator, typename Allocator>
	inline void merge_sort(RandomAccessIterator first, RandomAccessIterator last, Allocator& allocator)
	{
    __builtin_trap() /* STUB: not implemented */;
}



	/// partition
	///
	/// Implements the partition algorithm.
	/// Rearranges the elements in the range [first, last), in such a way that all the elements 
	/// for which pred returns true precede all those for which it returns false. The iterator 
	/// returned points to the first element of the second group.
	/// The relative ordering within each group is not necessarily the same as before the call. 
	/// See function stable_partition for a function with a similar behavior and stability in 
	/// the ordering.
	/// 
	/// To do: Implement a version that uses a faster BidirectionalIterator algorithm for the 
	///        case that the iterator range is a bidirectional iterator instead of just an
	///        input iterator (one direction).
	///
	template<typename InputIterator, typename Predicate>
	InputIterator partition(InputIterator begin, InputIterator end, Predicate predicate)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	/// stable_partition
	///
	/// Performs the same function as @p partition() with the additional
	/// guarantee that the relative ordering of elements in each group is
	/// preserved.
	template <typename ForwardIterator, typename Predicate>
	ForwardIterator stable_partition(ForwardIterator first, ForwardIterator last, Predicate pred)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	/////////////////////////////////////////////////////////////////////
	// quick_sort
	//
	// We do the "introspection sort" variant of quick sort which is now
	// well-known and understood. You can read about this algorithm in
	// many articles on quick sort, but briefly what it does is a median-
	// of-three quick sort whereby the recursion depth is limited to a
	// some value (after which it gives up on quick sort and switches to
	// a heap sort) and whereby after a certain amount of sorting the 
	// algorithm stops doing quick-sort and finishes the sorting via
	// a simple insertion sort.
	/////////////////////////////////////////////////////////////////////

	#if (defined(EA_PROCESSOR_X86) || defined(EA_PROCESSOR_X86_64))
		static const int kQuickSortLimit = 28; // For sorts of random arrays over 100 items, 28 - 32 have been found to be good numbers on x86.
	#else
		static const int kQuickSortLimit = 16; // It seems that on other processors lower limits are more beneficial, as they result in fewer compares.
	#endif

	namespace Internal
	{
		template <typename Size>
		inline Size Log2(Size n)
		{
    __builtin_trap() /* STUB: not implemented */;
}

		// To do: Investigate the speed of this bit-trick version of Log2.
		//        It may work better on some platforms but not others.
		//
		// union FloatUnion {
		//     float    f;
		//     uint32_t i;
		// };
		// 
		// inline uint32_t Log2(uint32_t x)
		// {
		//     const FloatInt32Union u = { x };
		//     return (u.i >> 23) - 127;
		// }
	}

	template <typename RandomAccessIterator, typename T>
	inline RandomAccessIterator get_partition_impl(RandomAccessIterator first, RandomAccessIterator last, T&& pivotValue)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	/// get_partition
	///
	/// This function takes const T& instead of T because T may have special alignment
	/// requirements and some compilers (e.g. VC++) are don't respect alignment requirements
	/// for function arguments.
	///
	template <typename RandomAccessIterator, typename T>
	inline RandomAccessIterator get_partition(RandomAccessIterator first, RandomAccessIterator last, const T& pivotValue)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename RandomAccessIterator, typename T>
	inline RandomAccessIterator get_partition(RandomAccessIterator first, RandomAccessIterator last, T&& pivotValue)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename RandomAccessIterator, typename T, typename Compare>
	inline RandomAccessIterator get_partition_impl(RandomAccessIterator first, RandomAccessIterator last, T&& pivotValue, Compare compare)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename RandomAccessIterator, typename T, typename Compare> 
	inline RandomAccessIterator get_partition(RandomAccessIterator first, RandomAccessIterator last, const T& pivotValue, Compare compare)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename RandomAccessIterator, typename T, typename Compare>
	inline RandomAccessIterator get_partition(RandomAccessIterator first, RandomAccessIterator last, T&& pivotValue, Compare compare)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	namespace Internal
	{
		// This function is used by quick_sort and is not intended to be used by itself. 
		// This is because the implementation below makes an assumption about the input
		// data that quick_sort satisfies but arbitrary data may not.
		// There is a standalone insertion_sort function. 
		template <typename RandomAccessIterator>
		inline void insertion_sort_simple(RandomAccessIterator first, RandomAccessIterator last)
		{
    __builtin_trap() /* STUB: not implemented */;
}


		// This function is used by quick_sort and is not intended to be used by itself. 
		// This is because the implementation below makes an assumption about the input
		// data that quick_sort satisfies but arbitrary data may not.
		// There is a standalone insertion_sort function. 
		template <typename RandomAccessIterator, typename Compare>
		inline void insertion_sort_simple(RandomAccessIterator first, RandomAccessIterator last, Compare compare)
		{
    __builtin_trap() /* STUB: not implemented */;
}
	} // namespace Internal


	template <typename RandomAccessIterator>
	inline void partial_sort(RandomAccessIterator first, RandomAccessIterator middle, RandomAccessIterator last)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename RandomAccessIterator, typename Compare>
	inline void partial_sort(RandomAccessIterator first, RandomAccessIterator middle, RandomAccessIterator last, Compare compare)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template<typename RandomAccessIterator>
	inline void nth_element(RandomAccessIterator first, RandomAccessIterator nth, RandomAccessIterator last)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template<typename RandomAccessIterator, typename Compare>
	inline void nth_element(RandomAccessIterator first, RandomAccessIterator nth, RandomAccessIterator last, Compare compare)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	namespace Internal
	{
		EA_DISABLE_VC_WARNING(4702) // unreachable code
		template <typename RandomAccessIterator, typename Size, typename PivotValueType>
		inline void quick_sort_impl_helper(RandomAccessIterator first, RandomAccessIterator last, Size kRecursionCount)
		{
    __builtin_trap() /* STUB: not implemented */;
}

		template <typename RandomAccessIterator, typename Size, typename Compare, typename PivotValueType>
		inline void quick_sort_impl_helper(RandomAccessIterator first, RandomAccessIterator last, Size kRecursionCount, Compare compare)
		{
    __builtin_trap() /* STUB: not implemented */;
}
		EA_RESTORE_VC_WARNING()

		template <typename RandomAccessIterator, typename Size>
		inline void quick_sort_impl(RandomAccessIterator first, RandomAccessIterator last, Size kRecursionCount,
			typename eastl::enable_if<eastl::is_copy_constructible<typename iterator_traits<RandomAccessIterator>::value_type>::value>::type* = 0)
		{
    __builtin_trap() /* STUB: not implemented */;
}

		template <typename RandomAccessIterator, typename Size>
		inline void quick_sort_impl(RandomAccessIterator first, RandomAccessIterator last, Size kRecursionCount,
			typename eastl::enable_if<eastl::is_move_constructible<typename iterator_traits<RandomAccessIterator>::value_type>::value
			&& !eastl::is_copy_constructible<typename iterator_traits<RandomAccessIterator>::value_type>::value>::type* = 0)
		{
    __builtin_trap() /* STUB: not implemented */;
}

		template <typename RandomAccessIterator, typename Size, typename Compare>
		inline void quick_sort_impl(RandomAccessIterator first, RandomAccessIterator last, Size kRecursionCount, Compare compare,
			typename eastl::enable_if<eastl::is_copy_constructible<typename iterator_traits<RandomAccessIterator>::value_type>::value>::type* = 0)
		{
    __builtin_trap() /* STUB: not implemented */;
}

		template <typename RandomAccessIterator, typename Size, typename Compare>
		inline void quick_sort_impl(RandomAccessIterator first, RandomAccessIterator last, Size kRecursionCount, Compare compare,
			typename eastl::enable_if<eastl::is_move_constructible<typename iterator_traits<RandomAccessIterator>::value_type>::value
			&& !eastl::is_copy_constructible<typename iterator_traits<RandomAccessIterator>::value_type>::value>::type* = 0)
		{
    __builtin_trap() /* STUB: not implemented */;
}
	}


	/// quick_sort
	///
	/// This is an unstable sort.
	/// quick_sort sorts the elements in [first, last) into ascending order, 
	/// meaning that if i and j are any two valid iterators in [first, last) 
	/// such that i precedes j, then *j is not less than *i. quick_sort is not 
	/// guaranteed to be stable. That is, suppose that *i and *j are equivalent: 
	/// neither one is less than the other. It is not guaranteed that the 
	/// relative order of these two elements will be preserved by sort.
	///
	/// We implement the "introspective" variation of quick-sort. This is 
	/// considered to be the best general-purpose variant, as it avoids 
	/// worst-case behaviour and optimizes the final sorting stage by 
	/// switching to an insertion sort.
	///
	template <typename RandomAccessIterator>
	void quick_sort(RandomAccessIterator first, RandomAccessIterator last)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename RandomAccessIterator, typename Compare>
	void quick_sort(RandomAccessIterator first, RandomAccessIterator last, Compare compare)
	{
    __builtin_trap() /* STUB: not implemented */;
}




	namespace Internal
	{
		// Portions of the tim_sort code were originally written by Christopher Swenson.
		// https://github.com/swenson/sort
		// All code in this repository, unless otherwise specified, is hereby licensed under the 
		// MIT Public License: Copyright (c) 2010 Christopher Swenson

		const intptr_t kTimSortStackSize = 64; // Question: What's the upper-limit size requirement for this?

		struct tim_sort_run
		{
			intptr_t start;
			intptr_t length;
		};


		// reverse_elements
		//
		// Reverses the range [first + start, first + start + size)
		// To consider: Use void eastl::reverse(BidirectionalIterator first, BidirectionalIterator last);
		//
		template <typename RandomAccessIterator>
		void reverse_elements(RandomAccessIterator first, intptr_t start, intptr_t end)
		{
    __builtin_trap() /* STUB: not implemented */;
}


		// tim_sort_count_run
		//
		// Finds the length of a run which is already sorted (either up or down).
		// If the run is in reverse order, this function puts it in regular order.
		//
		template <typename RandomAccessIterator, typename StrictWeakOrdering>
		intptr_t tim_sort_count_run(const RandomAccessIterator first, const intptr_t start, const intptr_t size, StrictWeakOrdering compare)
		{
    __builtin_trap() /* STUB: not implemented */;
}


		// Input   Return
		// --------------
		//  64      32
		//  65      33
		//  66      33
		//  67      34
		//  68      34
		// ...
		// 125      63
		// 126      63
		// 127      64
		// 128      32
		// 129      33
		// 130      33
		// 131      33
		// 132      33
		// 133      34
		// 134      34
		// 135      34
		// 136      34
		// 137      35
		// ...
		//
		// This function will return a value that is always in the range of [32, 64].
		//
		static inline intptr_t timsort_compute_minrun(intptr_t size)
		{
    __builtin_trap() /* STUB: not implemented */;
}


		template <typename RandomAccessIterator, typename T, typename StrictWeakOrdering>
		void tim_sort_merge(RandomAccessIterator first, const tim_sort_run* run_stack, const intptr_t stack_curr, 
							T* pBuffer, StrictWeakOrdering compare)
		{
    __builtin_trap() /* STUB: not implemented */;
}


		// See the timsort.txt file for an explanation of this function.
		//
		// ------------------------------------------------------------------------
		// What turned out to be a good compromise maintains two invariants on the
		// stack entries, where A, B and C are the lengths of the three righmost 
		// not-yet merged slices:
		//    1.  A > B+C
		//    2.  B > C
		// ------------------------------------------------------------------------
		//
		static inline bool timsort_check_invariant(tim_sort_run* run_stack, const intptr_t stack_curr)
		{
    __builtin_trap() /* STUB: not implemented */;
}


		template <typename RandomAccessIterator, typename T, typename StrictWeakOrdering>
		intptr_t tim_sort_collapse(RandomAccessIterator first, tim_sort_run* run_stack, intptr_t stack_curr, 
								   T* pBuffer, const intptr_t size, StrictWeakOrdering compare)
		{
    __builtin_trap() /* STUB: not implemented */;
}


		// tim_sort_add_run
		//
		// Return true if the sort is done.
		//
		template <typename RandomAccessIterator, typename T, typename StrictWeakOrdering>
		bool tim_sort_add_run(tim_sort_run* run_stack, RandomAccessIterator first, T* pBuffer, const intptr_t size, const intptr_t minrun, 
							  intptr_t& len, intptr_t& run, intptr_t& curr, intptr_t& stack_curr, StrictWeakOrdering compare)
		{
    __builtin_trap() /* STUB: not implemented */;
}

	} // namespace Internal


	// tim_sort_buffer
	//
	/// This is a stable sort.
	// Implements the tim-sort sorting algorithm with a user-provided scratch buffer.
	// http://en.wikipedia.org/wiki/Timsort
	// This sort is the fastest sort when sort stability (maintaining order of equal values) is required and
	// data sets are non-trivial (size >= 15). It's also the fastest sort (e.g. faster than quick_sort) for 
	// the case that at at least half your data is already sorted. Otherwise, eastl::quick_sort is about 10% 
	// faster than tim_sort_buffer but is not a stable sort. There are some reports that tim_sort outperforms
	// quick_sort but most of these aren't taking into account that optimal quick_sort implementations use
	// a hybrid approach called "introsort" (http://en.wikipedia.org/wiki/Introsort) which improves quick_sort
	// considerably in practice.
	//
	// Strengths:
	//     - Fastest stable sort for most sizes of data.
	//     - Fastest sort for containers of data already mostly sorted.
	//     - Simpler to understand than quick_sort.
	//
	// Weaknesses:
	//     - User must provide a scratch buffer, otherwise the buffer is dynamically allocated during runtime.
	//     - Not as fast as quick_sort for the general case of randomized data.
	//     - Requires a RandomAccessIterator; thus must be on an array container type and not a list container type.
	//     - Uses a lot of code to implement; thus it's not great when there is little room for more code.
	//
	// The pBuffer parameter must hold at least ((last-first)/2) elements (i.e. half the elements of the container).
	// This minimum size is a worst-case size requirement, but handles all possible cases. pBuffer is just a scratch
	// buffer and is not needed after the return of this function, and doesn't need to be seeded with any particular
	// values upon entering this function.
	//
	// Example usage:
	//     int intArray[64];
	//     int buffer[32];
	//     ...
	//     tim_sort_buffer(intArray, intArray + 64, buffer);
	//
	template <typename RandomAccessIterator, typename T, typename StrictWeakOrdering>
	void tim_sort_buffer(RandomAccessIterator first, RandomAccessIterator last, T* pBuffer, StrictWeakOrdering compare)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename RandomAccessIterator, typename T>
	inline void tim_sort_buffer(RandomAccessIterator first, RandomAccessIterator last, T* pBuffer)
	{
    __builtin_trap() /* STUB: not implemented */;
}




	/// radix_sort
	///
	/// Implements a classic LSD (least significant digit) radix sort.
	/// See http://en.wikipedia.org/wiki/Radix_sort.
	/// This sort requires that the sorted data be of a type that has a member
	/// radix_type typedef and an mKey member of that type. The type must be
	/// an integral type. This limits what can be sorted, but radix_sort is 
	/// very fast -- typically faster than any other sort.
	/// For example:
	///     struct Sortable {
	///         typedef int radix_type;
	///         radix_type mKey;
	///         // User data goes here, or the user can inherit from Sortable.
	///     };
	/// or, more generally:
	///     template <typname Integer>
	///     struct Sortable {
	///         typedef Integer radix_type;
	///         Integer mKey;
	///     };
	/// 
	/// Example usage:
	///     struct Element {
	///         typedef uint16_t radix_type;
	///         uint16_t mKey;
	///         uint16_t mUserData;
	///     };
	///
	///     Element elementArray[100];
	///     Element buffer[100];
	///
	///     radix_sort<Element*, extract_radix_key<Element> >(elementArray, elementArray + 100, buffer);
	///
	/// To consider: A static linked-list implementation may be faster than the version here.

	namespace Internal
	{
		/// extract_radix_key
		///
		/// Default radix sort integer value reader. It expects the sorted elements 
		/// to have an integer member of type radix_type and of name "mKey". 
		///
		template <typename Node>
		struct extract_radix_key
		{
			typedef typename Node::radix_type radix_type;

			const radix_type operator()(const Node& x) const
				{
    __builtin_trap() /* STUB: not implemented */;
}
		};

		// The radix_sort implementation uses two optimizations that are not part of a typical radix sort implementation.
		// 1. Computing a histogram (i.e. finding the number of elements per bucket) for the next pass is done in parallel with the loop that "scatters"
		//    elements in the current pass.  The advantage is that it avoids the memory traffic / cache pressure of reading keys in a separate operation.
		//    Note: It would also be possible to compute all histograms in a single pass.  However, that would increase the amount of stack space used and
		//    also increase cache pressure slightly.  However, it could still be faster under some situations.
		// 2. If all elements are mapped to a single bucket, then there is no need to perform a scatter operation.  Instead the elements are left in place
		//    and only copied if they need to be copied to the final output buffer.
		template <typename RandomAccessIterator, typename ExtractKey, int DigitBits, typename IntegerType>
		void radix_sort_impl(RandomAccessIterator first,
			RandomAccessIterator last,
			RandomAccessIterator buffer,
			ExtractKey extractKey,
			IntegerType)
		{
    __builtin_trap() /* STUB: not implemented */;
}
	} // namespace Internal

	template <typename RandomAccessIterator, typename ExtractKey, int DigitBits = 8>
	void radix_sort(RandomAccessIterator first, RandomAccessIterator last, RandomAccessIterator buffer)
	{
    __builtin_trap() /* STUB: not implemented */;
}



	/// comb_sort
	///
	/// This is an unstable sort.
	/// Implements the CombSort algorithm; in particular, implements the CombSort11 variation 
	/// of the CombSort algorithm, based on the reference to '11' in the implementation.
	///
	/// To consider: Use a comb sort table instead of the '((nSpace * 10) + 3) / 13' expression.
	///              Ideal tables can be found on the Internet by looking up "comb sort table".
	///
	template <typename ForwardIterator, typename StrictWeakOrdering>
	void comb_sort(ForwardIterator first, ForwardIterator last, StrictWeakOrdering compare)
	{
    __builtin_trap() /* STUB: not implemented */;
} // comb_sort

	template <typename ForwardIterator>
	inline void comb_sort(ForwardIterator first, ForwardIterator last)
	{
    __builtin_trap() /* STUB: not implemented */;
}




	/// bubble_sort
	///
	/// This is a stable sort.
	/// Implements the BubbleSort algorithm. This algorithm is only useful for 
	/// small range sizes, such as 10 or less items. You may be better off using
	/// insertion_sort for cases where bubble_sort works.
	///
	namespace Internal
	{
		template <typename ForwardIterator, typename StrictWeakOrdering>
		void bubble_sort_impl(ForwardIterator first, ForwardIterator last, StrictWeakOrdering compare, eastl::forward_iterator_tag)
		{
    __builtin_trap() /* STUB: not implemented */;
}

		template <typename BidirectionalIterator, typename StrictWeakOrdering>
		void bubble_sort_impl(BidirectionalIterator first, BidirectionalIterator last, StrictWeakOrdering compare, eastl::bidirectional_iterator_tag)
		{
    __builtin_trap() /* STUB: not implemented */;
}
	} // namespace Internal

	template <typename ForwardIterator, typename StrictWeakOrdering>
	inline void bubble_sort(ForwardIterator first, ForwardIterator last, StrictWeakOrdering compare)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename ForwardIterator>
	inline void bubble_sort(ForwardIterator first, ForwardIterator last)
	{
    __builtin_trap() /* STUB: not implemented */;
}



	/// sort
	/// 
	/// We use quick_sort by default. See quick_sort for details.
	///
	/// EASTL_DEFAULT_SORT_FUNCTION
	/// If a default sort function is specified then call it, otherwise use EASTL's default quick_sort.
	/// EASTL_DEFAULT_SORT_FUNCTION must be namespace-qualified and include any necessary template
	/// parameters (e.g. eastl::comb_sort instead of just comb_sort), and it must be visible to this code. 
	/// The EASTL_DEFAULT_SORT_FUNCTION must be provided in two versions: 
	///     template <typename RandomAccessIterator>
	///     void EASTL_DEFAULT_SORT_FUNCTION(RandomAccessIterator first, RandomAccessIterator last);
	///
	///     template <typename RandomAccessIterator, typename Compare>
	///     void EASTL_DEFAULT_SORT_FUNCTION(RandomAccessIterator first, RandomAccessIterator last, Compare compare)
	///
	template <typename RandomAccessIterator>
	inline void sort(RandomAccessIterator first, RandomAccessIterator last)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename RandomAccessIterator, typename Compare>
	inline void sort(RandomAccessIterator first, RandomAccessIterator last, Compare compare)
	{
    __builtin_trap() /* STUB: not implemented */;
}



	/// stable_sort
	/// 
	/// We use merge_sort by default. See merge_sort for details.
	/// Beware that the used merge_sort -- and thus stable_sort -- allocates 
	/// memory during execution. Try using merge_sort_buffer if you want
	/// to avoid memory allocation.
	///
	/// EASTL_DEFAULT_STABLE_SORT_FUNCTION
	/// If a default sort function is specified then call it, otherwise use EASTL's default merge_sort.
	/// EASTL_DEFAULT_STABLE_SORT_FUNCTION must be namespace-qualified and include any necessary template
	/// parameters (e.g. eastl::tim_sort instead of just tim_sort), and it must be visible to this code. 
	/// The EASTL_DEFAULT_STABLE_SORT_FUNCTION must be provided in three versions, though the third
	/// allocation implementation may choose to ignore the allocator parameter: 
	///     template <typename RandomAccessIterator, typename StrictWeakOrdering>
	///     void EASTL_DEFAULT_STABLE_SORT_FUNCTION(RandomAccessIterator first, RandomAccessIterator last, StrictWeakOrdering compare);
	///     
	///     template <typename RandomAccessIterator>
	///     void EASTL_DEFAULT_STABLE_SORT_FUNCTION(RandomAccessIterator first, RandomAccessIterator last);
	///
	///     template <typename RandomAccessIterator, typename Allocator, typename StrictWeakOrdering>
	///     void EASTL_DEFAULT_STABLE_SORT_FUNCTION(RandomAccessIterator first, RandomAccessIterator last, Allocator& allocator, StrictWeakOrdering compare);
	///
	template <typename RandomAccessIterator, typename StrictWeakOrdering>
	void stable_sort(RandomAccessIterator first, RandomAccessIterator last, StrictWeakOrdering compare)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename RandomAccessIterator>
	void stable_sort(RandomAccessIterator first, RandomAccessIterator last)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename RandomAccessIterator, typename Allocator, typename StrictWeakOrdering>
	void stable_sort(RandomAccessIterator first, RandomAccessIterator last, Allocator& allocator, StrictWeakOrdering compare)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	// This is not defined because it would cause compiler errors due to conflicts with a version above. 
	//template <typename RandomAccessIterator, typename Allocator>
	//void stable_sort(RandomAccessIterator first, RandomAccessIterator last, Allocator& allocator)
	//{
	//    #if defined(EASTL_DEFAULT_STABLE_SORT_FUNCTION)
	//        EASTL_DEFAULT_STABLE_SORT_FUNCTION<RandomAccessIterator, Allocator>(first, last, allocator);
	//    #else
	//        eastl::merge_sort<RandomAccessIterator, Allocator>(first, last, allocator);
	//    #endif
	//}




	/* 
	// Something to consider adding: An eastl sort which uses qsort underneath. 
	// The primary purpose of this is to have an eastl interface for sorting which
	// results in very little code generation, since all instances map to the 
	// C qsort function.

	template <typename T>
	int small_footprint_sort_func(const void* a, const void* b)
	{
		if(*(const T*)a < *(const T*)b)
			return -1;
		if(*(const T*)a > *(const T*)b)
			return +1;
		return 0;
	}

	template <typename ContiguousIterator>
	void small_footprint_sort(ContiguousIterator first, ContiguousIterator last)
	{
		typedef typename eastl::iterator_traits<ContiguousIterator>::value_type value_type;

		qsort(first, (size_t)eastl::distance(first, last), sizeof(value_type), small_footprint_sort_func<value_type>);
	}
	*/

} // namespace eastl


#endif // Header include guard



