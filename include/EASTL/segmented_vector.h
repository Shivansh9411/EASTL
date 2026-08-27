#include <stdexcept>
#include <cstdlib>
///////////////////////////////////////////////////////////////////////////////
// Copyright (c) Electronic Arts Inc. All rights reserved.
///////////////////////////////////////////////////////////////////////////////

#ifndef EASTL_SEGMENTED_VECTOR_H
#define EASTL_SEGMENTED_VECTOR_H

#if defined(EA_PRAGMA_ONCE_SUPPORTED)
	#pragma once 
#endif

#include <EASTL/internal/config.h>

#include <EASTL/algorithm.h>
#include <EASTL/allocator.h>
#include <EASTL/memory.h>

namespace eastl
{
	// TODO: this really shouldn't be a public class, deprecate it and hide it.
	template<typename T, size_t Count, typename Allocator = EASTLAllocatorType>
	class segment
	{
	public:
		typedef eastl_size_t					size_type;
		typedef segment<T, Count, Allocator>	this_type;
		typedef T*								iterator;
		typedef const T*						const_iterator;

		const this_type*		next_segment() const;
		this_type*				next_segment();

		const_iterator			begin() const;
		iterator				begin();

		const_iterator			end() const;
		iterator				end();

		segment() = default;
		// TODO: should we implement copy/move ctor/assign?
		segment(const segment& other) = delete;
		segment& operator=(const segment&) = delete;
		// move ctor/assignment implicitly deleted.

		// The owning segment_vector is in charge of destroying the
		// data before the destructor of the segment runs, so we leave
		// the destructor as trivial. This class can't be copied and
		// it's public API doesn't really let you do anything, only
		// segmented_vector can really use these.
		//
		//~segment();

	private:
		// Destroys all the elements in the segment.
		void                    DestroyData();

		static const uintptr_t	kIsLastSegment = 1 << 0;
		uintptr_t				mPrev;

		union
		{
			// When the segment is not the last one in the segmented
			// vector, mNext is the link to the next segment.
			this_type*			mNext;
			// When the segment is the last one in the segmented
			// vector, mSize is the number elements in this current
			// segment. Note: if a segment is not the last one, it
			// must be completely full i.e. it must contain exactly
			// Count elements so there's no need to store anything in
			// mSize.
			size_type			mSize;
		};

		// Storage for the segment. Type-erased because we want to
		// support types which are not default constructible.
		// TODO: Should we be using std::byte here instead of
		// unsigned char?
		alignas(T) unsigned char mData[Count * sizeof(T)];

		template<typename, size_t, typename> friend class segmented_vector;
		template<typename, size_t, typename> friend struct segmented_vector_iterator;
	};


	template <typename T, size_t Count, typename Allocator = EASTLAllocatorType>
	struct segmented_vector_iterator
	{
	public:
        typedef segmented_vector_iterator<T, Count, Allocator>	this_type;
		typedef segment<T, Count, Allocator>					segment_type;

		// does not conform to any Iterator concept. could be a bidirectional iterator, but not random access iterator because segment is a double-linked list.

        T*						operator->() const;
        T&						operator*() const;

        this_type&				operator++();
        this_type				operator++(int);
		
	public:
		T*						mCurrent;
		T*						mEnd;
		segment_type*			mSegment;
	};


	template <typename T, size_t Count, typename Allocator = EASTLAllocatorType>
	class segmented_vector
	{
	public:
		typedef eastl_size_t												size_type;
		typedef segmented_vector<T, Count, Allocator>						this_type;
		typedef segment<T, Count, Allocator>								segment_type;
        typedef Allocator													allocator_type;
		typedef segmented_vector_iterator<const T, Count, Allocator>		const_iterator;
		typedef segmented_vector_iterator<T, Count, Allocator>				iterator;
		typedef T															value_type;


		segmented_vector(const Allocator& allocator = Allocator());
		segmented_vector(const segmented_vector& other);
		segmented_vector(segmented_vector&& other);
		segmented_vector& operator=(const segmented_vector& other);
		segmented_vector& operator=(segmented_vector&& other);
		~segmented_vector();

		// constructors that initialize from a range, etc. missing

		segmented_vector(std::initializer_list<value_type> ilist, const Allocator& allocator = Allocator());

		const allocator_type& get_allocator() const noexcept;
		allocator_type& get_allocator() noexcept;

		// TODO: deprecate these? what's the point of having them in
		// the API? We don't want people messing around with the
		// segments directly.
		const segment_type* first_segment() const noexcept;
		segment_type* first_segment() noexcept;

		const_iterator begin() const noexcept;
		iterator begin() noexcept;

		const_iterator end() const noexcept;
		iterator end() noexcept;

		// The number of elements in the container.
		size_type size() const noexcept;

		// The total capacity of the container.
		size_type capacity() const noexcept;

		// TODO: Deprecate this?
		// The number of "active" segments (does not include segments
		// in the free list).
		size_type segment_count() const noexcept;

		// These are UB if the container is empty.
		T& front() noexcept;
		T& back() noexcept;

		// Return true if the container has no elements and false
		// otherwise.
		bool empty() const noexcept;

		// Destroys the elements in the container, but does not
		// relinquish any memory (i.e. capacity() is unchanged)
		void clear();

		// Increase the capacity so it fits at least `n` elements.
		// This is less useful than in normal vectors since this will
		// allocate multiple segments, resulting in the same number of
		// allocations as if you'd pushed the elements one by one
		// without reserving.
		void reserve(size_type n);

		// Resizes the container to contain exactly `n` elements.
		//   - If `size() > n` it destroys the last `size() - n` elements.
		//   - If `size() < n` it inserts `n - size()` copies of `v`.
		void resize(size_type n, const value_type& v);

		// Equivalent to resize(n, value_type());
		void resize(size_type n);

		// Frees all the segments not currently in use, i.e makes it
		// so capacity() is the lowest multiple of Count greater than
		// or equal to size().
		void shrink_to_fit() noexcept;

		// missing (could be implemented):
		//		assign()
		//		set_allocator()
		//		set_capacity()
		//		validate()
		//		validate_iterator()

		// segmented_vector is almost a deque, but cannot provide:
		//		operator[]
		//		at()
		//		insert()
		//		push_front()
		//		emplace()
		//		emplace_front()
		//		erase()
		// because it is not a random access container and can only push/pop elements from the back.

		T& push_back();
		T& push_back(const T& value);
		T& push_back(T&& value);
		void* push_back_uninitialized();

		template <class... Args>
		T& emplace_back(Args&&... args);

		void pop_back();

		void erase_unsorted(segment_type& segment, typename segment_type::iterator it);
		iterator erase_unsorted(const iterator& i);

		void swap(this_type& other);

	protected:
		// This does not initialize the entry, it just makes returns a
		// pointer to an address where one could initialize one.
		void* DoPushBack();

		// Moves the segment into the free list
		void AddToFreeList(segment_type* segment);

		// Get a segment from the free list, or allocate a segment if
		// there are none.  Sets up the segment's "last segment" flag.
		segment_type* GetUnusedSegmentForLastSegment(segment_type* prevSegment);

		// Allocate a new segment.
		segment_type* AllocateNewSegment();

		// Destroys the elements in the container. Optionally also
		// frees all the memory.
		template <bool bFreeMemory>
		void Clear();

		// If the last segment is empty, this function will move it to
		// the free list and update mLastSegment with the previous
		// segment.
		void UpdateLastSegment();

		// Remove the last `n` elements in the container.
		void EraseFromBack(size_type n);

		// Pushes `n` copies of `v`
		void PushBack(size_type n, const value_type& v);

		template<bool bDoMove, typename ForwardIt>
		void InsertRange(ForwardIt begin, ForwardIt end);

		allocator_type mAllocator;
		segment_type* mFirstSegment{};
		segment_type* mLastSegment{};
		segment_type* mFreeList{};
		size_type mInUseSegmentCount{};

		// TODO: consider storing this in the first free segment (if
		// there is one), it'd save the memory for this member but
		// would make things like `capacity()` less cache coherent and
		// branchy (we need to check if there's a free segment)
		size_type mFreeListSegmentCount{};
	};


	/////////////////////////////////////
	//   segment
	/////////////////////////////////////

	template<typename T, size_t Count, typename Allocator>
	inline const segment<T, Count, Allocator>*
	segment<T, Count, Allocator>::next_segment() const
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template<typename T, size_t Count, typename Allocator>
	inline segment<T, Count, Allocator>*
	segment<T, Count, Allocator>::next_segment()
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template<typename T, size_t Count, typename Allocator>
	inline typename segment<T, Count, Allocator>::const_iterator
	segment<T, Count, Allocator>::begin() const
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template<typename T, size_t Count, typename Allocator>
	inline typename segment<T, Count, Allocator>::iterator
	segment<T, Count, Allocator>::begin()
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template<typename T, size_t Count, typename Allocator>
	inline typename segment<T, Count, Allocator>::const_iterator
	segment<T, Count, Allocator>::end() const
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template<typename T, size_t Count, typename Allocator>
	inline typename segment<T, Count, Allocator>::iterator
	segment<T, Count, Allocator>::end()
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T, size_t Count, typename Allocator>
	inline void segment<T, Count, Allocator>::DestroyData()
	{
    __builtin_trap() /* STUB: not implemented */;
}


	/////////////////////////////////////
	//   segmented_vector_iterator
	/////////////////////////////////////

	template<typename T, size_t Count, typename Allocator>
    T*
	segmented_vector_iterator<T, Count, Allocator>::operator->() const
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template<typename T, size_t Count, typename Allocator>
    T&
	segmented_vector_iterator<T, Count, Allocator>::operator*() const
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template<typename T, size_t Count, typename Allocator>
    segmented_vector_iterator<T, Count, Allocator>&
	segmented_vector_iterator<T, Count, Allocator>::operator++()
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template<typename T, size_t Count, typename Allocator>
    segmented_vector_iterator<T, Count, Allocator>
	segmented_vector_iterator<T, Count, Allocator>::operator++(int)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	/////////////////////////////////////
	//   segmented_vector
	/////////////////////////////////////

	template <typename T, size_t Count, typename Allocator>
	inline segmented_vector<T, Count, Allocator>::segmented_vector(const Allocator& allocator)
	:	mAllocator(allocator)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T, size_t Count, typename Allocator>
	inline segmented_vector<T, Count, Allocator>::segmented_vector(const segmented_vector& other)
	    : mAllocator(other.mAllocator)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T, size_t Count, typename Allocator>
	inline segmented_vector<T, Count, Allocator>::segmented_vector(segmented_vector&& other)
	    : mAllocator(other.mAllocator)
	{
		swap(other);
	}

	template <typename T, size_t Count, typename Allocator>
	inline segmented_vector<T, Count, Allocator>::segmented_vector(std::initializer_list<value_type> ilist, const Allocator& allocator)
		: mAllocator(allocator)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T, size_t Count, typename Allocator>
	inline segmented_vector<T, Count, Allocator>& segmented_vector<T, Count, Allocator>::operator=(
	    const segmented_vector& other)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T, size_t Count, typename Allocator>
	inline segmented_vector<T, Count, Allocator>& segmented_vector<T, Count, Allocator>::operator=(
	    segmented_vector&& other)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T, size_t Count, typename Allocator>
	inline segmented_vector<T, Count, Allocator>::~segmented_vector()
	{
		Clear<true>();
	}

	template <typename T, size_t Count, typename Allocator>
	inline const typename segmented_vector<T, Count, Allocator>::allocator_type&
	segmented_vector<T, Count, Allocator>::get_allocator() const noexcept
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T, size_t Count, typename Allocator>
	inline typename segmented_vector<T, Count, Allocator>::allocator_type&
	segmented_vector<T, Count, Allocator>::get_allocator() noexcept
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T, size_t Count, typename Allocator>
	inline const typename segmented_vector<T, Count, Allocator>::segment_type*
	segmented_vector<T, Count, Allocator>::first_segment() const noexcept
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T, size_t Count, typename Allocator>
	inline typename segmented_vector<T, Count, Allocator>::segment_type*
	segmented_vector<T, Count, Allocator>::first_segment() noexcept
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T, size_t Count, typename Allocator>
	inline typename segmented_vector<T, Count, Allocator>::const_iterator
	segmented_vector<T, Count, Allocator>::begin() const noexcept
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T, size_t Count, typename Allocator>
	inline typename segmented_vector<T, Count, Allocator>::iterator
	segmented_vector<T, Count, Allocator>::begin() noexcept
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T, size_t Count, typename Allocator>
	inline typename segmented_vector<T, Count, Allocator>::const_iterator
	segmented_vector<T, Count, Allocator>::end() const noexcept
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T, size_t Count, typename Allocator>
	inline typename segmented_vector<T, Count, Allocator>::iterator
	segmented_vector<T, Count, Allocator>::end() noexcept
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T, size_t Count, typename Allocator>
	inline typename segmented_vector<T, Count, Allocator>::size_type
	segmented_vector<T, Count, Allocator>::size() const noexcept
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T, size_t Count, typename Allocator>
	inline typename segmented_vector<T, Count, Allocator>::size_type
	segmented_vector<T, Count, Allocator>::capacity() const noexcept
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T, size_t Count, typename Allocator>
	inline typename segmented_vector<T, Count, Allocator>::size_type
	segmented_vector<T, Count, Allocator>::segment_count() const noexcept
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T, size_t Count, typename Allocator>
	inline T&
	segmented_vector<T, Count, Allocator>::front() noexcept
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T, size_t Count, typename Allocator>
	inline T&
	segmented_vector<T, Count, Allocator>::back() noexcept
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T, size_t Count, typename Allocator>
	inline bool
	segmented_vector<T, Count, Allocator>::empty() const noexcept
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T, size_t Count, typename Allocator>
	inline void
	segmented_vector<T, Count, Allocator>::clear()
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T, size_t Count, typename Allocator>
	inline void segmented_vector<T, Count, Allocator>::reserve(size_type n)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T, size_t Count, typename Allocator>
	inline void segmented_vector<T, Count, Allocator>::resize(size_type n, const value_type& v)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T, size_t Count, typename Allocator>
	inline void
	segmented_vector<T, Count, Allocator>::resize(size_type n)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T, size_t Count, typename Allocator>
	inline void segmented_vector<T, Count, Allocator>::shrink_to_fit() noexcept
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T, size_t Count, typename Allocator>
	inline T&
	segmented_vector<T, Count, Allocator>::push_back()
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T, size_t Count, typename Allocator>
	inline T&
	segmented_vector<T, Count, Allocator>::push_back(const T& value)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T, size_t Count, typename Allocator>
	inline T&
	segmented_vector<T, Count, Allocator>::push_back(T&& value)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T, size_t Count, typename Allocator>
	inline void*
	segmented_vector<T, Count, Allocator>::push_back_uninitialized()
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T, size_t Count, typename Allocator>
	template <class... Args>
	T& segmented_vector<T, Count, Allocator>::emplace_back(Args&&... args)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T, size_t Count, typename Allocator>
	inline void
	segmented_vector<T, Count, Allocator>::pop_back()
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T, size_t Count, typename Allocator>
	inline void
	segmented_vector<T, Count, Allocator>::erase_unsorted(segment_type& segment, typename segment_type::iterator it)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T, size_t Count, typename Allocator>
	inline typename segmented_vector<T, Count, Allocator>::iterator
	segmented_vector<T, Count, Allocator>::erase_unsorted(const iterator& i)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T, size_t Count, typename Allocator>
	void
	segmented_vector<T, Count, Allocator>::swap(this_type& other)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T, size_t Count, typename Allocator>
	inline void*
	segmented_vector<T, Count, Allocator>::DoPushBack()
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T, size_t Count, typename Allocator>
	inline void
	segmented_vector<T, Count, Allocator>::AddToFreeList(segment_type* segment)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T, size_t Count, typename Allocator>
	inline typename segmented_vector<T, Count, Allocator>::segment_type*
	segmented_vector<T, Count, Allocator>::GetUnusedSegmentForLastSegment(segment_type* prevSegment)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, size_t Count, typename Allocator>
	inline typename segmented_vector<T, Count, Allocator>::segment_type*
	segmented_vector<T, Count, Allocator>::AllocateNewSegment()
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T, size_t Count, typename Allocator>
	template <bool bFreeMemory>
	inline void segmented_vector<T, Count, Allocator>::Clear()
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T, size_t Count, typename Allocator>
	inline void segmented_vector<T, Count, Allocator>::UpdateLastSegment()
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T, size_t Count, typename Allocator>
	inline void segmented_vector<T, Count, Allocator>::EraseFromBack(size_type toRemoveCount)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T, size_t Count, typename Allocator>
	inline void segmented_vector<T, Count, Allocator>::PushBack(size_type toAddCount, const value_type& v)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T, size_t Count, typename Allocator>
	template<bool bDoMove, typename ForwardIt>
	inline void segmented_vector<T, Count, Allocator>::InsertRange(ForwardIt begin, ForwardIt end)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template<typename T, size_t Count, typename Allocator>
    inline bool operator==(const segmented_vector_iterator<const T, Count, Allocator>& a, const segmented_vector_iterator<const T, Count, Allocator>& b)
    {
    __builtin_trap() /* STUB: not implemented */;
}


	template<typename T, size_t Count, typename Allocator>
    inline bool operator!=(const segmented_vector_iterator<const T, Count, Allocator>& a, const segmented_vector_iterator<const T, Count, Allocator>& b)
    {
    __builtin_trap() /* STUB: not implemented */;
}

	template<typename T, size_t Count, typename Allocator>
    inline bool operator==(const segmented_vector_iterator<T, Count, Allocator>& a, const segmented_vector_iterator<T, Count, Allocator>& b)
    {
    __builtin_trap() /* STUB: not implemented */;
}


	template<typename T, size_t Count, typename Allocator>
    inline bool operator!=(const segmented_vector_iterator<T, Count, Allocator>& a, const segmented_vector_iterator<T, Count, Allocator>& b)
    {
    __builtin_trap() /* STUB: not implemented */;
}

	///////////////////////////////////////////////////////////////////////
	// global operators
	///////////////////////////////////////////////////////////////////////

	template<typename T, size_t Count, typename Allocator>
	inline bool operator==(const segmented_vector<T, Count, Allocator>& a, const segmented_vector<T, Count, Allocator>& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}

#if defined(EA_COMPILER_HAS_THREE_WAY_COMPARISON)
	template<typename T, size_t Count, typename Allocator>
	inline synth_three_way_result<T> operator<=>(const segmented_vector<T, Count, Allocator>& a, const segmented_vector<T, Count, Allocator>& b)
	{
		return eastl::lexicographical_compare_three_way(a.begin(), a.end(), b.begin(), b.end(), synth_three_way{});
	}
#endif

	template<typename T, size_t Count, typename Allocator>
	inline bool operator!=(const segmented_vector<T, Count, Allocator>& a, const segmented_vector<T, Count, Allocator>& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template<typename T, size_t Count, typename Allocator>
	inline bool operator<(const segmented_vector<T, Count, Allocator>& a, const segmented_vector<T, Count, Allocator>& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template<typename T, size_t Count, typename Allocator>
	inline bool operator>(const segmented_vector<T, Count, Allocator>& a, const segmented_vector<T, Count, Allocator>& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template<typename T, size_t Count, typename Allocator>
	inline bool operator<=(const segmented_vector<T, Count, Allocator>& a, const segmented_vector<T, Count, Allocator>& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template<typename T, size_t Count, typename Allocator>
	inline bool operator>=(const segmented_vector<T, Count, Allocator>& a, const segmented_vector<T, Count, Allocator>& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template<typename T, size_t Count, typename Allocator>
	inline void swap(segmented_vector<T, Count, Allocator>& a, segmented_vector<T, Count, Allocator>& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}
}

#endif
