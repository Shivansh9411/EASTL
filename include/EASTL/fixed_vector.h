#include <stdexcept>
#include <cstdlib>
///////////////////////////////////////////////////////////////////////////////
// Copyright (c) Electronic Arts Inc. All rights reserved.
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// This file implements a vector which uses a fixed size memory pool. 
// The bEnableOverflow template parameter allows the container to resort to
// heap allocations if the memory pool is exhausted.
///////////////////////////////////////////////////////////////////////////////


#ifndef EASTL_FIXED_VECTOR_H
#define EASTL_FIXED_VECTOR_H


#include <EASTL/vector.h>
#include <EASTL/internal/fixed_pool.h>

#if defined(EA_PRAGMA_ONCE_SUPPORTED)
	#pragma once // Some compilers (e.g. VC++) benefit significantly from using this. We've measured 3-4% build speed improvements in apps as a result.
#endif



namespace eastl
{
	/// EASTL_FIXED_VECTOR_DEFAULT_NAME
	///
	/// Defines a default container name in the absence of a user-provided name.
	/// In the case of fixed-size containers, the allocator name always refers
	/// to overflow allocations. 
	///
	#ifndef EASTL_FIXED_VECTOR_DEFAULT_NAME
		#define EASTL_FIXED_VECTOR_DEFAULT_NAME EASTL_DEFAULT_NAME_PREFIX " fixed_vector" // Unless the user overrides something, this is "EASTL fixed_vector".
	#endif


	/// EASTL_FIXED_VECTOR_DEFAULT_ALLOCATOR
	///
	#ifndef EASTL_FIXED_VECTOR_DEFAULT_ALLOCATOR
		#define EASTL_FIXED_VECTOR_DEFAULT_ALLOCATOR overflow_allocator_type(EASTL_FIXED_VECTOR_DEFAULT_NAME)
	#endif


	/// fixed_vector
	///
	/// A fixed_vector with bEnableOverflow == true is identical to a regular 
	/// vector in terms of its behavior. All the expectations of regular vector
	/// apply to it and no additional expectations come from it. When bEnableOverflow
	/// is false, fixed_vector behaves like regular vector with the exception that 
	/// its capacity can never increase. All operations you do on such a fixed_vector
	/// which require a capacity increase will result in undefined behavior or an 
	/// C++ allocation exception, depending on the configuration of EASTL.
	///
	/// Template parameters:
	///     T                      The type of object the vector holds.
	///     nodeCount              The max number of objects to contain.
	///     bEnableOverflow        Whether or not we should use the overflow heap if our object pool is exhausted.
	///     OverflowAllocator              Overflow allocator, which is only used if bEnableOverflow == true. Defaults to the global heap.
	///
	/// Note: The nodeCount value must be at least 1.
	///
	/// Example usage:
	///    fixed_vector<Widget, 128, true> fixedVector);
	///
	///    fixedVector.push_back(Widget());
	///    fixedVector.resize(200);
	///    fixedVector.clear();
	///
	template <typename T, size_t nodeCount, bool bEnableOverflow = true, typename OverflowAllocator = typename eastl::conditional<bEnableOverflow, EASTLAllocatorType, EASTLDummyAllocatorType>::type>
	class fixed_vector : public vector<T, fixed_vector_allocator<sizeof(T), nodeCount, EASTL_ALIGN_OF(T), 0, bEnableOverflow, OverflowAllocator> >
	{
	public:
		typedef fixed_vector_allocator<sizeof(T), nodeCount, EASTL_ALIGN_OF(T), 
							0, bEnableOverflow, OverflowAllocator>              fixed_allocator_type;
		typedef OverflowAllocator                                               overflow_allocator_type;
		typedef vector<T, fixed_allocator_type>                                 base_type;
		typedef fixed_vector<T, nodeCount, bEnableOverflow, OverflowAllocator>  this_type;
		typedef typename base_type::size_type                                   size_type;
		typedef typename base_type::value_type                                  value_type;
		typedef typename base_type::reference                                   reference;
		typedef typename base_type::iterator                                    iterator;
		typedef typename base_type::const_iterator                              const_iterator;
		typedef aligned_buffer<nodeCount * sizeof(T), EASTL_ALIGN_OF(T)>        aligned_buffer_type;

		enum { kMaxSize = nodeCount };

		using base_type::get_allocator;
		using base_type::resize;
		using base_type::clear;
		using base_type::size;
		using base_type::assign;
		using base_type::npos;

		static_assert(!is_const<value_type>::value, "fixed_vector<T> value_type must be non-const.");
		static_assert(!is_volatile<value_type>::value, "fixed_vector<T> value_type must be non-volatile.");

	protected:
		aligned_buffer_type mBuffer;

		using base_type::mpBegin;
		using base_type::mpEnd;
		using base_type::internalCapacityPtr;
		using base_type::DoAllocate;
		using base_type::DoFree;
		using base_type::DoAssign;

	public:
		fixed_vector();
		explicit fixed_vector(const overflow_allocator_type& overflowAllocator); // Only applicable if bEnableOverflow is true.
		explicit fixed_vector(size_type n);                                      // Currently we don't support overflowAllocator specification for other constructors, for simplicity.
		fixed_vector(size_type n, const overflow_allocator_type& overflowAllocator);
		fixed_vector(size_type n, const value_type& value);
		fixed_vector(size_type n, const value_type& value, const overflow_allocator_type& overflowAllocator);
		fixed_vector(const this_type& x);
		fixed_vector(this_type&& x) EA_NOEXCEPT;
		fixed_vector(this_type&& x, const overflow_allocator_type& overflowAllocator);
		fixed_vector(std::initializer_list<T> ilist, const overflow_allocator_type& overflowAllocator = EASTL_FIXED_VECTOR_DEFAULT_ALLOCATOR);

		template <typename InputIterator>
		fixed_vector(InputIterator first, InputIterator last);

		this_type& operator=(const this_type& x);
		this_type& operator=(std::initializer_list<T> ilist);
		this_type& operator=(this_type&& x);

		void swap(this_type& x);

		void      set_capacity(size_type n);
		void      clear(bool freeOverflow);
		void      reset_lose_memory();          // This is a unilateral reset to an initially empty state. No destructors are called, no deallocation occurs.
		size_type max_size() const;             // Returns the max fixed size, which is the user-supplied nodeCount parameter.
		bool      full() const;                 // Returns true if the fixed space has been fully allocated. Note that if overflow is enabled, the container size can be greater than nodeCount but full() could return true because the fixed space may have a recently freed slot. 
		bool      has_overflowed() const;       // Returns true if the allocations spilled over into the overflow allocator. Meaningful only if overflow is enabled.
		static constexpr bool can_overflow() { return bEnableOverflow; } // Returns the value of the bEnableOverflow template parameter.

		void*     push_back_uninitialized();
		void      push_back(const value_type& value);   // We implement push_back here because we have a specialization that's 
		reference push_back();                          // smaller for the case of overflow being disabled.
		void      push_back(value_type&& value);

		// OverflowAllocator
		const overflow_allocator_type& get_overflow_allocator() const EA_NOEXCEPT;
		overflow_allocator_type&       get_overflow_allocator() EA_NOEXCEPT;
		void                           set_overflow_allocator(const overflow_allocator_type& allocator);

	protected:
		void*     DoPushBackUninitialized(true_type);
		void*     DoPushBackUninitialized(false_type);

		void      DoPushBack(true_type, const value_type& value);
		void      DoPushBack(false_type, const value_type& value);

		void      DoPushBackMove(true_type, value_type&& value);
		void      DoPushBackMove(false_type, value_type&& value);

		reference DoPushBack(false_type);
		reference DoPushBack(true_type);

	}; // fixed_vector




	///////////////////////////////////////////////////////////////////////
	// fixed_vector
	///////////////////////////////////////////////////////////////////////

	template <typename T, size_t nodeCount, bool bEnableOverflow, typename OverflowAllocator>
	inline fixed_vector<T, nodeCount, bEnableOverflow, OverflowAllocator>::fixed_vector()
		: base_type(fixed_allocator_type(mBuffer.buffer))
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T, size_t nodeCount, bool bEnableOverflow, typename OverflowAllocator>
	inline fixed_vector<T, nodeCount, bEnableOverflow, OverflowAllocator>::fixed_vector(const overflow_allocator_type& overflowAllocator)
		: base_type(fixed_allocator_type(mBuffer.buffer, overflowAllocator))
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T, size_t nodeCount, bool bEnableOverflow, typename OverflowAllocator>
	inline fixed_vector<T, nodeCount, bEnableOverflow, OverflowAllocator>::fixed_vector(size_type n)
		: base_type(fixed_allocator_type(mBuffer.buffer))
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T, size_t nodeCount, bool bEnableOverflow, typename OverflowAllocator>
	inline fixed_vector<T, nodeCount, bEnableOverflow, OverflowAllocator>::fixed_vector(size_type n, const overflow_allocator_type& overflowAllocator)
		: base_type(fixed_allocator_type(mBuffer.buffer, overflowAllocator))
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, size_t nodeCount, bool bEnableOverflow, typename OverflowAllocator>
	inline fixed_vector<T, nodeCount, bEnableOverflow, OverflowAllocator>::fixed_vector(size_type n, const value_type& value)
		: base_type(fixed_allocator_type(mBuffer.buffer))
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T, size_t nodeCount, bool bEnableOverflow, typename OverflowAllocator>
	inline fixed_vector<T, nodeCount, bEnableOverflow, OverflowAllocator>::fixed_vector(size_type n, const value_type& value, const overflow_allocator_type& overflowAllocator)
		: base_type(fixed_allocator_type(mBuffer.buffer, overflowAllocator))
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, size_t nodeCount, bool bEnableOverflow, typename OverflowAllocator>
	inline fixed_vector<T, nodeCount, bEnableOverflow, OverflowAllocator>::fixed_vector(const this_type& x)
		: base_type(fixed_allocator_type(mBuffer.buffer))
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, size_t nodeCount, bool bEnableOverflow, typename OverflowAllocator>
	inline fixed_vector<T, nodeCount, bEnableOverflow, OverflowAllocator>::fixed_vector(this_type&& x) EA_NOEXCEPT
		: base_type(fixed_allocator_type(mBuffer.buffer))
	{
		// Since we are a fixed_vector, we can't swap pointers. We can possibly do something like fixed_swap or
		// we can just do an assignment from x. If we want to do the former then we need to have some complicated
		// code to deal with overflow or no overflow, and whether the memory is in the fixed-size buffer or in 
		// the overflow allocator. 90% of the time the memory should be in the fixed buffer, in which case
		// a simple assignment is no worse than the fancy pathway.

		// Since we are a fixed_vector, we can't normally swap pointers unless both this and 
		// x are using using overflow and the overflow allocators are equal. To do:
		//if(has_overflowed() && x.has_overflowed() && (get_overflow_allocator() == x.get_overflow_allocator()))
		//{
		//    We can swap contents and may need to swap the allocators as well.
		//}
		get_allocator().copy_overflow_allocator(x.get_allocator());

		#if EASTL_NAME_ENABLED
			get_allocator().set_name(x.get_allocator().get_name());
		#endif

		mpBegin = mpEnd = (value_type*)&mBuffer.buffer[0];
		internalCapacityPtr() = mpBegin + nodeCount;
		base_type::template DoAssign<move_iterator<iterator>, true>(eastl::make_move_iterator(x.begin()), eastl::make_move_iterator(x.end()), false_type());
	}


	template <typename T, size_t nodeCount, bool bEnableOverflow, typename OverflowAllocator>
	inline fixed_vector<T, nodeCount, bEnableOverflow, OverflowAllocator>::fixed_vector(this_type&& x, const overflow_allocator_type& overflowAllocator)
		: base_type(fixed_allocator_type(mBuffer.buffer, overflowAllocator))
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, size_t nodeCount, bool bEnableOverflow, typename OverflowAllocator>
	inline fixed_vector<T, nodeCount, bEnableOverflow, OverflowAllocator>::fixed_vector(std::initializer_list<T> ilist, const overflow_allocator_type& overflowAllocator)
		: base_type(fixed_allocator_type(mBuffer.buffer, overflowAllocator))
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, size_t nodeCount, bool bEnableOverflow, typename OverflowAllocator>
	template <typename InputIterator>
	fixed_vector<T, nodeCount, bEnableOverflow, OverflowAllocator>::fixed_vector(InputIterator first, InputIterator last)
		: base_type(fixed_allocator_type(mBuffer.buffer))
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, size_t nodeCount, bool bEnableOverflow, typename OverflowAllocator>
	inline typename fixed_vector<T, nodeCount, bEnableOverflow, OverflowAllocator>::this_type& 
	fixed_vector<T, nodeCount, bEnableOverflow, OverflowAllocator>::operator=(const this_type& x)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, size_t nodeCount, bool bEnableOverflow, typename OverflowAllocator>
	inline typename fixed_vector<T, nodeCount, bEnableOverflow, OverflowAllocator>::this_type& 
	fixed_vector<T, nodeCount, bEnableOverflow, OverflowAllocator>::operator=(std::initializer_list<T> ilist)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, size_t nodeCount, bool bEnableOverflow, typename OverflowAllocator>
	inline typename fixed_vector<T, nodeCount, bEnableOverflow, OverflowAllocator>::this_type& 
	fixed_vector<T, nodeCount, bEnableOverflow, OverflowAllocator>::operator=(this_type&& x)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, size_t nodeCount, bool bEnableOverflow, typename OverflowAllocator>
	inline void fixed_vector<T, nodeCount, bEnableOverflow, OverflowAllocator>::swap(this_type& x)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, size_t nodeCount, bool bEnableOverflow, typename OverflowAllocator>
	inline void fixed_vector<T, nodeCount, bEnableOverflow, OverflowAllocator>::set_capacity(size_type n)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, size_t nodeCount, bool bEnableOverflow, typename Allocator>
	inline void fixed_vector<T, nodeCount, bEnableOverflow, Allocator>::clear(bool freeOverflow)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, size_t nodeCount, bool bEnableOverflow, typename OverflowAllocator>
	inline void fixed_vector<T, nodeCount, bEnableOverflow, OverflowAllocator>::reset_lose_memory()
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, size_t nodeCount, bool bEnableOverflow, typename OverflowAllocator>
	inline typename fixed_vector<T, nodeCount, bEnableOverflow, OverflowAllocator>::size_type
	fixed_vector<T, nodeCount, bEnableOverflow, OverflowAllocator>::max_size() const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, size_t nodeCount, bool bEnableOverflow, typename OverflowAllocator>
	inline bool fixed_vector<T, nodeCount, bEnableOverflow, OverflowAllocator>::full() const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, size_t nodeCount, bool bEnableOverflow, typename OverflowAllocator>
	inline bool fixed_vector<T, nodeCount, bEnableOverflow, OverflowAllocator>::has_overflowed() const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, size_t nodeCount, bool bEnableOverflow, typename OverflowAllocator>
	inline void* fixed_vector<T, nodeCount, bEnableOverflow, OverflowAllocator>::push_back_uninitialized()
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, size_t nodeCount, bool bEnableOverflow, typename OverflowAllocator>
	inline void* fixed_vector<T, nodeCount, bEnableOverflow, OverflowAllocator>::DoPushBackUninitialized(true_type)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, size_t nodeCount, bool bEnableOverflow, typename OverflowAllocator>
	inline void* fixed_vector<T, nodeCount, bEnableOverflow, OverflowAllocator>::DoPushBackUninitialized(false_type)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, size_t nodeCount, bool bEnableOverflow, typename OverflowAllocator>
	inline void fixed_vector<T, nodeCount, bEnableOverflow, OverflowAllocator>::push_back(const value_type& value)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, size_t nodeCount, bool bEnableOverflow, typename OverflowAllocator>
	inline void fixed_vector<T, nodeCount, bEnableOverflow, OverflowAllocator>::DoPushBack(true_type, const value_type& value)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	// This template specializes for overflow NOT enabled.
	// In this configuration, there is no need for the heavy weight push_back() which tests to see if the container should grow (it never will)
	template <typename T, size_t nodeCount, bool bEnableOverflow, typename OverflowAllocator>
	inline void fixed_vector<T, nodeCount, bEnableOverflow, OverflowAllocator>::DoPushBack(false_type, const value_type& value)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, size_t nodeCount, bool bEnableOverflow, typename OverflowAllocator>
	inline typename fixed_vector<T, nodeCount, bEnableOverflow, OverflowAllocator>::reference fixed_vector<T, nodeCount, bEnableOverflow, OverflowAllocator>::push_back()
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, size_t nodeCount, bool bEnableOverflow, typename OverflowAllocator>
	inline typename fixed_vector<T, nodeCount, bEnableOverflow, OverflowAllocator>::reference fixed_vector<T, nodeCount, bEnableOverflow, OverflowAllocator>::DoPushBack(true_type)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	// This template specializes for overflow NOT enabled.
	// In this configuration, there is no need for the heavy weight push_back() which tests to see if the container should grow (it never will)
	template <typename T, size_t nodeCount, bool bEnableOverflow, typename OverflowAllocator>
	inline typename fixed_vector<T, nodeCount, bEnableOverflow, OverflowAllocator>::reference fixed_vector<T, nodeCount, bEnableOverflow, OverflowAllocator>::DoPushBack(false_type)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, size_t nodeCount, bool bEnableOverflow, typename OverflowAllocator>
	inline void fixed_vector<T, nodeCount, bEnableOverflow, OverflowAllocator>::push_back(value_type&& value)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, size_t nodeCount, bool bEnableOverflow, typename OverflowAllocator>
	inline void fixed_vector<T, nodeCount, bEnableOverflow, OverflowAllocator>::DoPushBackMove(true_type, value_type&& value)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	// This template specializes for overflow NOT enabled.
	// In this configuration, there is no need for the heavy weight push_back() which tests to see if the container should grow (it never will)
	template <typename T, size_t nodeCount, bool bEnableOverflow, typename OverflowAllocator>
	inline void fixed_vector<T, nodeCount, bEnableOverflow, OverflowAllocator>::DoPushBackMove(false_type, value_type&& value)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, size_t nodeCount, bool bEnableOverflow, typename OverflowAllocator>
	inline const typename fixed_vector<T, nodeCount, bEnableOverflow, OverflowAllocator>::overflow_allocator_type& 
	fixed_vector<T, nodeCount, bEnableOverflow, OverflowAllocator>::get_overflow_allocator() const EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, size_t nodeCount, bool bEnableOverflow, typename OverflowAllocator>
	inline typename fixed_vector<T, nodeCount, bEnableOverflow, OverflowAllocator>::overflow_allocator_type& 
	fixed_vector<T, nodeCount, bEnableOverflow, OverflowAllocator>::get_overflow_allocator() EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, size_t nodeCount, bool bEnableOverflow, typename OverflowAllocator>
	inline void 
	fixed_vector<T, nodeCount, bEnableOverflow, OverflowAllocator>::set_overflow_allocator(const overflow_allocator_type& allocator)
	{
    __builtin_trap() /* STUB: not implemented */;
}



	///////////////////////////////////////////////////////////////////////
	// global operators
	///////////////////////////////////////////////////////////////////////

	// operator ==, !=, <, >, <=, >= come from the vector implementations.

	template <typename T, size_t nodeCount, bool bEnableOverflow, typename OverflowAllocator>
	inline void swap(fixed_vector<T, nodeCount, bEnableOverflow, OverflowAllocator>& a, 
					 fixed_vector<T, nodeCount, bEnableOverflow, OverflowAllocator>& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}



} // namespace eastl



#endif // Header include guard












