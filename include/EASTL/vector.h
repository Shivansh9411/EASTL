#include <cstdlib>
///////////////////////////////////////////////////////////////////////////////
// Copyright (c) Electronic Arts Inc. All rights reserved.
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// This file implements a vector (array-like container), much like the C++ 
// std::vector class.
// The primary distinctions between this vector and std::vector are:
//    - vector has a couple extension functions that increase performance.
//    - vector can contain objects with alignment requirements. std::vector 
//      cannot do so without a bit of tedious non-portable effort.
//    - vector supports debug memory naming natively.
//    - vector is easier to read, debug, and visualize.
//    - vector is savvy to an environment that doesn't have exception handling,
//      as is sometimes the case with console or embedded environments.
//    - vector has less deeply nested function calls and allows the user to 
//      enable forced inlining in debug builds in order to reduce bloat.
//    - vector<bool> is a vector of boolean values and not a bit vector.
//    - vector guarantees that memory is contiguous and that vector::iterator
//      is nothing more than a pointer to T.
//    - vector has an explicit data() method for obtaining a pointer to storage 
//      which is safe to call even if the block is empty. This avoids the 
//      common &v[0], &v.front(), and &*v.begin() constructs that trigger false 
//      asserts in STL debugging modes.
//    - vector data is guaranteed to be contiguous.
//    - vector has a set_capacity() function which frees excess capacity. 
//      The only way to do this with std::vector is via the cryptic non-obvious 
//      trick of using: vector<SomeClass>(x).swap(x);
///////////////////////////////////////////////////////////////////////////////


#ifndef EASTL_VECTOR_H
#define EASTL_VECTOR_H


#include <EASTL/internal/config.h>
#include <EASTL/allocator.h>
#include <EASTL/type_traits.h>
#include <EASTL/iterator.h>
#include <EASTL/algorithm.h>
#include <EASTL/initializer_list.h>
#include <EASTL/memory.h>
#include <EASTL/numeric_limits.h>
#include <EASTL/bonus/compressed_pair.h>

EA_DISABLE_ALL_VC_WARNINGS()
#include <new>
#include <stddef.h>
#if EASTL_EXCEPTIONS_ENABLED
	#include <stdexcept> // std::out_of_range, std::length_error, std::logic_error.
#endif
EA_RESTORE_ALL_VC_WARNINGS()

// 4530 - C++ exception handler used, but unwind semantics are not enabled. Specify /EHsc
// 4480 - nonstandard extension used: specifying underlying type for enum
// 4571 - catch(...) semantics changed since Visual C++ 7.1; structured exceptions (SEH) are no longer caught.
EA_DISABLE_VC_WARNING(4530 4480 4571);

// 4345 - Behavior change: an object of POD type constructed with an initializer of the form () will be default-initialized
// 4244 - Argument: conversion from 'int' to 'const eastl::vector<T>::value_type', possible loss of data
// 4127 - Conditional expression is constant
EA_DISABLE_VC_WARNING(4345 4244 4127);


#if defined(EA_PRAGMA_ONCE_SUPPORTED)
	#pragma once // Some compilers (e.g. VC++) benefit significantly from using this. We've measured 3-4% build speed improvements in apps as a result.
#endif

#if EASTL_NOMINMAX
	#ifdef min
		#undef min
	#endif
	#ifdef max
		#undef max
	#endif
#endif

namespace eastl
{

	/// EASTL_VECTOR_DEFAULT_NAME
	///
	/// Defines a default container name in the absence of a user-provided name.
	///
	#ifndef EASTL_VECTOR_DEFAULT_NAME
		#define EASTL_VECTOR_DEFAULT_NAME EASTL_DEFAULT_NAME_PREFIX " vector" // Unless the user overrides something, this is "EASTL vector".
	#endif


	/// EASTL_VECTOR_DEFAULT_ALLOCATOR
	///
	#ifndef EASTL_VECTOR_DEFAULT_ALLOCATOR
		#define EASTL_VECTOR_DEFAULT_ALLOCATOR allocator_type(EASTL_VECTOR_DEFAULT_NAME)
	#endif

	namespace internal
	{
		template <class SizeType, class IntSourceType>
		inline void AssertValueFitsInType(IntSourceType n, const char* assertMessage)
		{
    __builtin_trap() /* STUB: not implemented */;
}
	} // namespace internal


	/// VectorBase
	///
	/// The reason we have a VectorBase class is that it makes exception handling
	/// simpler to implement because memory allocation is implemented entirely 
	/// in this class. If a user creates a vector which needs to allocate
	/// memory in the constructor, VectorBase handles it. If an exception is thrown
	/// by the allocator then the exception throw jumps back to the user code and 
	/// no try/catch code need be written in the vector or VectorBase constructor. 
	/// If an exception is thrown in the vector (not VectorBase) constructor, the 
	/// destructor for VectorBase will be called automatically (and free the allocated
	/// memory) before the execution jumps back to the user code.
	/// However, if the vector class were to handle both allocation and initialization
	/// then it would have no choice but to implement an explicit try/catch statement
	/// for all pathways that allocate memory. This increases code size and decreases
	/// performance and makes the code a little harder read and maintain.
	///
	/// The C++ standard (15.2 paragraph 2) states: 
	///    "An object that is partially constructed or partially destroyed will
	///     have destructors executed for all its fully constructed subobjects,
	///     that is, for subobjects for which the constructor has been completed
	///     execution and the destructor has not yet begun execution."
	///
	/// The C++ standard (15.3 paragraph 11) states: 
	///    "The fully constructed base classes and members of an object shall 
	///     be destroyed before entering the handler of a function-try-block
	///     of a constructor or destructor for that block."
	///
	template <typename T, typename Allocator>
	struct VectorBase
	{
		typedef Allocator    allocator_type;
		typedef eastl_size_t size_type;
		typedef ptrdiff_t    difference_type;

		#if defined(_MSC_VER) && (_MSC_VER >= 1400) && (_MSC_VER <= 1600) && !EASTL_STD_CPP_ONLY  // _MSC_VER of 1400 means VS2005, 1600 means VS2010. VS2012 generates errors with usage of enum:size_type.
			enum : size_type {                      // Use Microsoft enum language extension, allowing for smaller debug symbols than using a static const. Users have been affected by this.
				npos     = (size_type)-1,
				kMaxSize = (size_type)-2
			};
		#else
			static const size_type npos     = (size_type)-1;      /// 'npos' means non-valid position or simply non-position.
			static const size_type kMaxSize = (size_type)-2;      /// -1 is reserved for 'npos'. It also happens to be slightly beneficial that kMaxSize is a value less than -1, as it helps us deal with potential integer wraparound issues.
		#endif

		size_type GetNewCapacity(size_type currentSize);

	protected:
		T*                                          mpBegin;
		T*                                          mpEnd;
		eastl::compressed_pair<T*, allocator_type>  mCapacityAllocator;

		T*& internalCapacityPtr() EA_NOEXCEPT {
    __builtin_trap() /* STUB: not implemented */;
}
		T* const& internalCapacityPtr() const EA_NOEXCEPT {
    __builtin_trap() /* STUB: not implemented */;
}
		allocator_type&  internalAllocator() EA_NOEXCEPT {
    __builtin_trap() /* STUB: not implemented */;
}
		const allocator_type&  internalAllocator() const EA_NOEXCEPT {
    __builtin_trap() /* STUB: not implemented */;
}

	public:
		VectorBase();
		VectorBase(const allocator_type& allocator);
		VectorBase(size_type n, const allocator_type& allocator);

	   ~VectorBase();

		const allocator_type& get_allocator() const EA_NOEXCEPT;
		allocator_type&       get_allocator() EA_NOEXCEPT;
		void                  set_allocator(const allocator_type& allocator);

	protected:
		T*        DoAllocate(size_type n);
		void      DoFree(T* p, size_type n);

	}; // VectorBase




	/// vector
	///
	/// Implements a dynamic array.
	///
	template <typename T, typename Allocator = EASTLAllocatorType>
	class vector : public VectorBase<T, Allocator>
	{
		typedef VectorBase<T, Allocator>                      base_type;
		typedef vector<T, Allocator>                          this_type;

		template <class T2, class Allocator2, class U>
		friend typename vector<T2, Allocator2>::size_type erase_unsorted(vector<T2, Allocator2>& c, const U& value);

		template <class T2, class Allocator2, class P>
		friend typename vector<T2, Allocator2>::size_type erase_unsorted_if(vector<T2, Allocator2>& c, P predicate);

	protected:
		using base_type::mpBegin;
		using base_type::mpEnd;
		using base_type::mCapacityAllocator;
		using base_type::DoAllocate;
		using base_type::DoFree;
		using base_type::internalCapacityPtr;
		using base_type::internalAllocator;

	public:
		typedef T                                             value_type;
		typedef T*                                            pointer;
		typedef const T*                                      const_pointer;
		typedef T&                                            reference;
		typedef const T&                                      const_reference;  // Maintainer note: We want to leave iterator defined as T* -- at least in release builds -- as this gives some algorithms an advantage that optimizers cannot get around.
		typedef T*                                            iterator;         // Note: iterator is simply T* right now, but this will likely change in the future, at least for debug builds. 
		typedef const T*                                      const_iterator;   //       Do not write code that relies on iterator being T*. The reason it will 
		typedef eastl::reverse_iterator<iterator>             reverse_iterator; //       change in the future is that a debugging iterator system will be created.
		typedef eastl::reverse_iterator<const_iterator>       const_reverse_iterator;    
		typedef typename base_type::size_type                 size_type;
		typedef typename base_type::difference_type           difference_type;
		typedef typename base_type::allocator_type            allocator_type;

		using base_type::npos;
		using base_type::GetNewCapacity;

		static_assert(!is_const<value_type>::value, "vector<T> value_type must be non-const.");
		static_assert(!is_volatile<value_type>::value, "vector<T> value_type must be non-volatile.");

	public:
		vector() EA_NOEXCEPT_IF(EA_NOEXCEPT_EXPR(EASTL_VECTOR_DEFAULT_ALLOCATOR));
		explicit vector(const allocator_type& allocator) EA_NOEXCEPT;
		explicit vector(size_type n, const allocator_type& allocator = EASTL_VECTOR_DEFAULT_ALLOCATOR);
		vector(size_type n, const value_type& value, const allocator_type& allocator = EASTL_VECTOR_DEFAULT_ALLOCATOR);
		vector(const this_type& x);
		vector(const this_type& x, const allocator_type& allocator);
		vector(this_type&& x) EA_NOEXCEPT;
		vector(this_type&& x, const allocator_type& allocator);
		vector(std::initializer_list<value_type> ilist, const allocator_type& allocator = EASTL_VECTOR_DEFAULT_ALLOCATOR);

		// note: this has pre-C++11 semantics:
		// this constructor is equivalent to the constructor vector(static_cast<size_type>(first), static_cast<value_type>(last), allocator) if InputIterator is an integral type.
		template <typename InputIterator>
		vector(InputIterator first, InputIterator last, const allocator_type& allocator = EASTL_VECTOR_DEFAULT_ALLOCATOR);

	   ~vector();

		this_type& operator=(const this_type& x);
		this_type& operator=(std::initializer_list<value_type> ilist);
		this_type& operator=(this_type&& x); // TODO(c++17): noexcept(allocator_traits<Allocator>::propagate_on_container_move_assignment::value || allocator_traits<Allocator>::is_always_equal::value)

		void swap(this_type& x); // TODO(c++17): noexcept(allocator_traits<Allocator>::propagate_on_container_move_assignment::value || allocator_traits<Allocator>::is_always_equal::value)

		void assign(size_type n, const value_type& value);

		template <typename InputIterator>
		void assign(InputIterator first, InputIterator last);

		void assign(std::initializer_list<value_type> ilist);

		iterator       begin() EA_NOEXCEPT;
		const_iterator begin() const EA_NOEXCEPT;
		const_iterator cbegin() const EA_NOEXCEPT;

		iterator       end() EA_NOEXCEPT;
		const_iterator end() const EA_NOEXCEPT;
		const_iterator cend() const EA_NOEXCEPT;

		reverse_iterator       rbegin() EA_NOEXCEPT;
		const_reverse_iterator rbegin() const EA_NOEXCEPT;
		const_reverse_iterator crbegin() const EA_NOEXCEPT;

		reverse_iterator       rend() EA_NOEXCEPT;
		const_reverse_iterator rend() const EA_NOEXCEPT;
		const_reverse_iterator crend() const EA_NOEXCEPT;

		bool      empty() const EA_NOEXCEPT;
		size_type size() const EA_NOEXCEPT;
		size_type capacity() const EA_NOEXCEPT;

		void resize(size_type n, const value_type& value);
		void resize(size_type n);
		void reserve(size_type n);
		void set_capacity(size_type n = base_type::npos);   // Revises the capacity to the user-specified value. Resizes the container to match the capacity if the requested capacity n is less than the current size. If n == npos then the capacity is reallocated (if necessary) such that capacity == size.
		void shrink_to_fit();                               // C++11 function which is the same as set_capacity().

		pointer       data() EA_NOEXCEPT;
		const_pointer data() const EA_NOEXCEPT;

		reference       operator[](size_type n);
		const_reference operator[](size_type n) const;

		reference       at(size_type n);
		const_reference at(size_type n) const;

		reference       front();
		const_reference front() const;

		reference       back();
		const_reference back() const;

		void      push_back(const value_type& value);
		reference push_back();
		void*     push_back_uninitialized();
		void      push_back(value_type&& value);
		void      pop_back();

		template<class... Args>
		iterator emplace(const_iterator position, Args&&... args);

		template<class... Args>
		reference emplace_back(Args&&... args);

		iterator insert(const_iterator position, const value_type& value);
		iterator insert(const_iterator position, size_type n, const value_type& value);
		iterator insert(const_iterator position, value_type&& value);
		iterator insert(const_iterator position, std::initializer_list<value_type> ilist);

		// note: this has pre-C++11 semantics:
		// this function is equivalent to insert(const_iterator position, static_cast<size_type>(first), static_cast<value_type>(last)) if InputIterator is an integral type.
		// ie. same as insert(const_iterator position, size_type n, const value_type& value)
		template <typename InputIterator>
		iterator insert(const_iterator position, InputIterator first, InputIterator last);

		iterator erase_first(const T& value);
		iterator erase_first_unsorted(const T& value); // Same as erase, except it doesn't preserve order, but is faster because it simply copies the last item in the vector over the erased position.
		reverse_iterator erase_last(const T& value);
		reverse_iterator erase_last_unsorted(const T& value); // Same as erase, except it doesn't preserve order, but is faster because it simply copies the last item in the vector over the erased position.

		iterator erase(const_iterator position);
		iterator erase(const_iterator first, const_iterator last);
		iterator erase_unsorted(const_iterator position);         // Same as erase, except it doesn't preserve order, but is faster because it simply copies the last item in the vector over the erased position.

		reverse_iterator erase(const_reverse_iterator position);
		reverse_iterator erase(const_reverse_iterator first, const_reverse_iterator last);
		reverse_iterator erase_unsorted(const_reverse_iterator position);

		void clear() EA_NOEXCEPT;
		void reset_lose_memory() EA_NOEXCEPT;                       // This is a unilateral reset to an initially empty state. No destructors are called, no deallocation occurs.

		bool validate() const EA_NOEXCEPT;
		int  validate_iterator(const_iterator i) const EA_NOEXCEPT;

	protected:
		// These functions do the real work of maintaining the vector. You will notice
		// that many of them have the same name but are specialized on iterator_tag
		// (iterator categories). This is because in these cases there is an optimized
		// implementation that can be had for some cases relative to others. Functions
		// which aren't referenced are neither compiled nor linked into the application.
		template <bool bMove> struct should_move_or_copy_tag{};
		using should_copy_tag = should_move_or_copy_tag<false>;
		using should_move_tag = should_move_or_copy_tag<true>;

		template <typename ForwardIterator> // Allocates a pointer of array count n and copy-constructs it with [first,last).
		pointer DoRealloc(size_type newCapacity, ForwardIterator first, ForwardIterator last, should_copy_tag);

		template <typename ForwardIterator> // Allocates a pointer of array count n and copy-constructs it with [first,last).
		pointer DoRealloc(size_type newCapacity, ForwardIterator first, ForwardIterator last, should_move_tag);

		template <typename Integer>
		void DoInit(Integer n, Integer value, true_type);

		template <typename InputIterator>
		void DoInit(InputIterator first, InputIterator last, false_type);

		template <typename InputIterator>
		void DoInitFromIterator(InputIterator first, InputIterator last, eastl::input_iterator_tag);

		template <typename ForwardIterator>
		void DoInitFromIterator(ForwardIterator first, ForwardIterator last, eastl::forward_iterator_tag);

		template <typename Integer, bool bMove>
		void DoAssign(Integer n, Integer value, true_type);

		template <typename InputIterator, bool bMove>
		void DoAssign(InputIterator first, InputIterator last, false_type);

		void DoAssignValues(size_type n, const value_type& value);

		template <typename InputIterator, bool bMove>
		void DoAssignFromIterator(InputIterator first, InputIterator last, eastl::input_iterator_tag);

		template <typename RandomAccessIterator, bool bMove>
		void DoAssignFromIterator(RandomAccessIterator first, RandomAccessIterator last, eastl::random_access_iterator_tag);

		template <typename Integer>
		void DoInsert(const_iterator position, Integer n, Integer value, true_type);

		template <typename InputIterator>
		void DoInsert(const_iterator position, InputIterator first, InputIterator last, false_type);

		template <typename InputIterator>
		void DoInsertFromIterator(const_iterator position, InputIterator first, InputIterator last, eastl::input_iterator_tag);

		template <typename BidirectionalIterator>
		void DoInsertFromIterator(const_iterator position, BidirectionalIterator first, BidirectionalIterator last, eastl::bidirectional_iterator_tag);

		void DoInsertValues(const_iterator position, size_type n, const value_type& value);

		void DoInsertValuesEnd(size_type n); // Default constructs n values
		void DoInsertValuesEnd(size_type n, const value_type& value);

		template<typename... Args>
		void DoInsertValue(const_iterator position, Args&&... args);

		template<typename... Args>
		void DoInsertValueEnd(Args&&... args);

		void DoClearCapacity();

		void DoGrow(size_type newCapacity);

		void DoSwap(this_type& x);

	}; // class vector






	///////////////////////////////////////////////////////////////////////
	// VectorBase
	///////////////////////////////////////////////////////////////////////

	template <typename T, typename Allocator>
	inline VectorBase<T, Allocator>::VectorBase()
		: mpBegin(NULL), 
		  mpEnd(NULL),
		  mCapacityAllocator(NULL, allocator_type(EASTL_VECTOR_DEFAULT_NAME))
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline VectorBase<T, Allocator>::VectorBase(const allocator_type& allocator)
		: mpBegin(NULL), 
		  mpEnd(NULL),
		  mCapacityAllocator(NULL, allocator)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline VectorBase<T, Allocator>::VectorBase(size_type n, const allocator_type& allocator)
		: mCapacityAllocator(allocator)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline VectorBase<T, Allocator>::~VectorBase()
	{
		if(mpBegin)
			EASTLFree(internalAllocator(), mpBegin, (internalCapacityPtr() - mpBegin) * sizeof(T));
	}


	template <typename T, typename Allocator>
	inline const typename VectorBase<T, Allocator>::allocator_type&
	VectorBase<T, Allocator>::get_allocator() const EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename VectorBase<T, Allocator>::allocator_type&
	VectorBase<T, Allocator>::get_allocator() EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline void VectorBase<T, Allocator>::set_allocator(const allocator_type& allocator)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline T* VectorBase<T, Allocator>::DoAllocate(size_type n)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline void VectorBase<T, Allocator>::DoFree(T* p, size_type n)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename VectorBase<T, Allocator>::size_type
	VectorBase<T, Allocator>::GetNewCapacity(size_type currentSize)
	{
    __builtin_trap() /* STUB: not implemented */;
}




	///////////////////////////////////////////////////////////////////////
	// vector
	///////////////////////////////////////////////////////////////////////

	template <typename T, typename Allocator>
	inline vector<T, Allocator>::vector() EA_NOEXCEPT_IF(EA_NOEXCEPT_EXPR(EASTL_VECTOR_DEFAULT_ALLOCATOR))
		: base_type()
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline vector<T, Allocator>::vector(const allocator_type& allocator) EA_NOEXCEPT
		: base_type(allocator)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline vector<T, Allocator>::vector(size_type n, const allocator_type& allocator)
		: base_type(n, allocator)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline vector<T, Allocator>::vector(size_type n, const value_type& value, const allocator_type& allocator)
		: base_type(n, allocator)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline vector<T, Allocator>::vector(const this_type& x)
		: base_type(x.size(), x.internalAllocator())
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline vector<T, Allocator>::vector(const this_type& x, const allocator_type& allocator)
		: base_type(x.size(), allocator)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline vector<T, Allocator>::vector(this_type&& x) EA_NOEXCEPT
		: base_type(eastl::move(x.internalAllocator()))  // vector requires move-construction of allocator in this case.
	{
		DoSwap(x);
	}


	template <typename T, typename Allocator>
	inline vector<T, Allocator>::vector(this_type&& x, const allocator_type& allocator)
		: base_type(allocator)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline vector<T, Allocator>::vector(std::initializer_list<value_type> ilist, const allocator_type& allocator)
		: base_type(allocator)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	template <typename InputIterator>
	inline vector<T, Allocator>::vector(InputIterator first, InputIterator last, const allocator_type& allocator)
		: base_type(allocator)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline vector<T, Allocator>::~vector()
	{
		// Call destructor for the values. Parent class will free the memory.
		eastl::destruct(mpBegin, mpEnd);
	}


	template <typename T, typename Allocator>
	typename vector<T, Allocator>::this_type&
	vector<T, Allocator>::operator=(const this_type& x)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	typename vector<T, Allocator>::this_type&
	vector<T, Allocator>::operator=(std::initializer_list<value_type> ilist)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	typename vector<T, Allocator>::this_type&
	vector<T, Allocator>::operator=(this_type&& x)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline void vector<T, Allocator>::assign(size_type n, const value_type& value)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	template <typename InputIterator>                              
	inline void vector<T, Allocator>::assign(InputIterator first, InputIterator last)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline void vector<T, Allocator>::assign(std::initializer_list<value_type> ilist)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename vector<T, Allocator>::iterator
	vector<T, Allocator>::begin() EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename vector<T, Allocator>::const_iterator
	vector<T, Allocator>::begin() const EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename vector<T, Allocator>::const_iterator
	vector<T, Allocator>::cbegin() const EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename vector<T, Allocator>::iterator
	vector<T, Allocator>::end() EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename vector<T, Allocator>::const_iterator
	vector<T, Allocator>::end() const EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename vector<T, Allocator>::const_iterator
	vector<T, Allocator>::cend() const EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename vector<T, Allocator>::reverse_iterator
	vector<T, Allocator>::rbegin() EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename vector<T, Allocator>::const_reverse_iterator
	vector<T, Allocator>::rbegin() const EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename vector<T, Allocator>::const_reverse_iterator
	vector<T, Allocator>::crbegin() const EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename vector<T, Allocator>::reverse_iterator
	vector<T, Allocator>::rend() EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename vector<T, Allocator>::const_reverse_iterator
	vector<T, Allocator>::rend() const EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename vector<T, Allocator>::const_reverse_iterator
	vector<T, Allocator>::crend() const EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	bool vector<T, Allocator>::empty() const EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename vector<T, Allocator>::size_type
	vector<T, Allocator>::size() const EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename vector<T, Allocator>::size_type
	vector<T, Allocator>::capacity() const EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline void vector<T, Allocator>::resize(size_type n, const value_type& value)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline void vector<T, Allocator>::resize(size_type n)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	void vector<T, Allocator>::reserve(size_type n)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	void vector<T, Allocator>::set_capacity(size_type n)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T, typename Allocator>
	inline void vector<T, Allocator>::shrink_to_fit()
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T, typename Allocator>
	inline typename vector<T, Allocator>::pointer
	vector<T, Allocator>::data() EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename vector<T, Allocator>::const_pointer
	vector<T, Allocator>::data() const EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename vector<T, Allocator>::reference
	vector<T, Allocator>::operator[](size_type n)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename vector<T, Allocator>::const_reference
	vector<T, Allocator>::operator[](size_type n) const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename vector<T, Allocator>::reference
	vector<T, Allocator>::at(size_type n)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename vector<T, Allocator>::const_reference
	vector<T, Allocator>::at(size_type n) const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename vector<T, Allocator>::reference
	vector<T, Allocator>::front()
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename vector<T, Allocator>::const_reference
	vector<T, Allocator>::front() const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename vector<T, Allocator>::reference
	vector<T, Allocator>::back()
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename vector<T, Allocator>::const_reference
	vector<T, Allocator>::back() const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline void vector<T, Allocator>::push_back(const value_type& value)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline void vector<T, Allocator>::push_back(value_type&& value)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename vector<T, Allocator>::reference
	vector<T, Allocator>::push_back()
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline void* vector<T, Allocator>::push_back_uninitialized()
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline void vector<T, Allocator>::pop_back()
	{
    __builtin_trap() /* STUB: not implemented */;
}

	EA_DISABLE_VC_WARNING(4702) // unreachable code: suppress warning because allocator_construct may always throw.
	template <typename T, typename Allocator>
	template<class... Args>
	inline typename vector<T, Allocator>::iterator 
	vector<T, Allocator>::emplace(const_iterator position, Args&&... args)
	{
    __builtin_trap() /* STUB: not implemented */;
}
	EA_RESTORE_VC_WARNING()

	EA_DISABLE_VC_WARNING(4702) // unreachable code: suppress warning because allocator_construct may always throw.
	template <typename T, typename Allocator>
	template<class... Args>
	inline typename vector<T, Allocator>::reference
	vector<T, Allocator>::emplace_back(Args&&... args)
	{
    __builtin_trap() /* STUB: not implemented */;
}
	EA_RESTORE_VC_WARNING()

	template <typename T, typename Allocator>
	inline typename vector<T, Allocator>::iterator
	vector<T, Allocator>::insert(const_iterator position, const value_type& value)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>       
	inline typename vector<T, Allocator>::iterator
	vector<T, Allocator>::insert(const_iterator position, value_type&& value)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename vector<T, Allocator>::iterator
	vector<T, Allocator>::insert(const_iterator position, size_type n, const value_type& value)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	template <typename InputIterator>
	inline typename vector<T, Allocator>::iterator
	vector<T, Allocator>::insert(const_iterator position, InputIterator first, InputIterator last)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>       
	inline typename vector<T, Allocator>::iterator
	vector<T, Allocator>::insert(const_iterator position, std::initializer_list<value_type> ilist)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename vector<T, Allocator>::iterator
	vector<T, Allocator>::erase(const_iterator position)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename vector<T, Allocator>::iterator
	vector<T, Allocator>::erase(const_iterator first, const_iterator last)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename vector<T, Allocator>::iterator
	vector<T, Allocator>::erase_unsorted(const_iterator position)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T, typename Allocator>
	inline typename vector<T, Allocator>::iterator vector<T, Allocator>::erase_first(const T& value)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T, typename Allocator>
	inline typename vector<T, Allocator>::iterator 
	vector<T, Allocator>::erase_first_unsorted(const T& value)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T, typename Allocator>
	inline typename vector<T, Allocator>::reverse_iterator 
	vector<T, Allocator>::erase_last(const T& value)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T, typename Allocator>
	inline typename vector<T, Allocator>::reverse_iterator 
	vector<T, Allocator>::erase_last_unsorted(const T& value)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T, typename Allocator>
	inline typename vector<T, Allocator>::reverse_iterator
	vector<T, Allocator>::erase(const_reverse_iterator position)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename vector<T, Allocator>::reverse_iterator
	vector<T, Allocator>::erase(const_reverse_iterator first, const_reverse_iterator last)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename vector<T, Allocator>::reverse_iterator
	vector<T, Allocator>::erase_unsorted(const_reverse_iterator position)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline void vector<T, Allocator>::clear() EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline void vector<T, Allocator>::reset_lose_memory() EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	// swap exchanges the contents of two containers. With respect to the containers allocators,
	// the C11++ Standard (23.2.1/7) states that the behavior of a call to a container's swap function 
	// is undefined unless the objects being swapped have allocators that compare equal or 
	// allocator_traits<allocator_type>::propagate_on_container_swap::value is true (propagate_on_container_swap
	// is false by default). EASTL doesn't have allocator_traits and so this doesn't directly apply,
	// but EASTL has the effective behavior of propagate_on_container_swap = true for all allocators. 
	template <typename T, typename Allocator>
	inline void vector<T, Allocator>::swap(this_type& x)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	template <typename ForwardIterator>
	inline typename vector<T, Allocator>::pointer
	vector<T, Allocator>::DoRealloc(size_type newCapacity, ForwardIterator first, ForwardIterator last, should_copy_tag)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	template <typename ForwardIterator>
	inline typename vector<T, Allocator>::pointer
	vector<T, Allocator>::DoRealloc(size_type newCapacity, ForwardIterator first, ForwardIterator last, should_move_tag)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	template <typename Integer>
	inline void vector<T, Allocator>::DoInit(Integer n, Integer value, true_type)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	template <typename InputIterator>
	inline void vector<T, Allocator>::DoInit(InputIterator first, InputIterator last, false_type)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	template <typename InputIterator>
	inline void vector<T, Allocator>::DoInitFromIterator(InputIterator first, InputIterator last, eastl::input_iterator_tag)
	{
    __builtin_trap() /* STUB: not implemented */;
}                                 // Luckily, InputIterators are in practice almost never used, so this code will likely never get executed.


	template <typename T, typename Allocator>
	template <typename ForwardIterator>
	inline void vector<T, Allocator>::DoInitFromIterator(ForwardIterator first, ForwardIterator last, eastl::forward_iterator_tag)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	template <typename Integer, bool bMove>
	inline void vector<T, Allocator>::DoAssign(Integer n, Integer value, true_type)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	template <typename InputIterator, bool bMove>
	inline void vector<T, Allocator>::DoAssign(InputIterator first, InputIterator last, false_type)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	void vector<T, Allocator>::DoAssignValues(size_type n, const value_type& value)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	template <typename InputIterator, bool bMove>
	void vector<T, Allocator>::DoAssignFromIterator(InputIterator first, InputIterator last, eastl::input_iterator_tag)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	template <typename RandomAccessIterator, bool bMove>
	void vector<T, Allocator>::DoAssignFromIterator(RandomAccessIterator first, RandomAccessIterator last, eastl::random_access_iterator_tag)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	template <typename Integer>
	inline void vector<T, Allocator>::DoInsert(const_iterator position, Integer n, Integer value, true_type)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	template <typename InputIterator>
	inline void vector<T, Allocator>::DoInsert(const_iterator position, InputIterator first, InputIterator last, false_type)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	template <typename InputIterator>
	inline void vector<T, Allocator>::DoInsertFromIterator(const_iterator position, InputIterator first, InputIterator last, eastl::input_iterator_tag)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	template <typename BidirectionalIterator>
	void vector<T, Allocator>::DoInsertFromIterator(const_iterator position, BidirectionalIterator first, BidirectionalIterator last, eastl::bidirectional_iterator_tag)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	void vector<T, Allocator>::DoInsertValues(const_iterator position, size_type n, const value_type& value)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	void vector<T, Allocator>::DoClearCapacity() // This function exists because set_capacity() currently indirectly requires value_type to be default-constructible, 
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	void vector<T, Allocator>::DoGrow(size_type newCapacity)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline void vector<T, Allocator>::DoSwap(this_type& x)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	// The code duplication between this and the version that takes no value argument and default constructs the values
	// is unfortunate but not easily resolved without relying on C++11 perfect forwarding.
	template <typename T, typename Allocator>
	void vector<T, Allocator>::DoInsertValuesEnd(size_type n, const value_type& value)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T, typename Allocator>
	void vector<T, Allocator>::DoInsertValuesEnd(size_type n)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T, typename Allocator>
	template<typename... Args>
	void vector<T, Allocator>::DoInsertValue(const_iterator position, Args&&... args)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	// assumes mpEnd == internalCapacityPtr(), ie. create a new array and move existing elements into it while inserting the new element at the end.
	template <typename T, typename Allocator>
	template<typename... Args>
	void vector<T, Allocator>::DoInsertValueEnd(Args&&... args)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline bool vector<T, Allocator>::validate() const EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline int vector<T, Allocator>::validate_iterator(const_iterator i) const EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}



	///////////////////////////////////////////////////////////////////////
	// global operators
	///////////////////////////////////////////////////////////////////////

	template <typename T, typename Allocator>
	inline bool operator==(const vector<T, Allocator>& a, const vector<T, Allocator>& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}

#if defined(EA_COMPILER_HAS_THREE_WAY_COMPARISON)
	template <typename T, typename Allocator>
	inline synth_three_way_result<T> operator<=>(const vector<T, Allocator>& a, const vector<T, Allocator>& b)
	{
		return eastl::lexicographical_compare_three_way(a.begin(), a.end(), b.begin(), b.end(), synth_three_way{});
	}
#else
	template <typename T, typename Allocator>
	inline bool operator!=(const vector<T, Allocator>& a, const vector<T, Allocator>& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline bool operator<(const vector<T, Allocator>& a, const vector<T, Allocator>& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline bool operator>(const vector<T, Allocator>& a, const vector<T, Allocator>& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline bool operator<=(const vector<T, Allocator>& a, const vector<T, Allocator>& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline bool operator>=(const vector<T, Allocator>& a, const vector<T, Allocator>& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}
#endif

	template <typename T, typename Allocator>
	inline void swap(vector<T, Allocator>& a, vector<T, Allocator>& b) EA_NOEXCEPT_IF(EA_NOEXCEPT_EXPR(a.swap(b)))
	{
    __builtin_trap() /* STUB: not implemented */;
}



	///////////////////////////////////////////////////////////////////////
	// erase / erase_if
	// 
	// https://en.cppreference.com/w/cpp/container/vector/erase2
	///////////////////////////////////////////////////////////////////////
	template <class T, class Allocator, class U>
	typename vector<T, Allocator>::size_type erase(vector<T, Allocator>& c, const U& value)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <class T, class Allocator, class Predicate>
	typename vector<T, Allocator>::size_type erase_if(vector<T, Allocator>& c, Predicate predicate)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	///////////////////////////////////////////////////////////////////////
	// erase_unsorted
	// 
	// This serves a similar purpose as erase above but with the difference
	// that it doesn't preserve the relative order of what is left in the
	// vector.
	//
	// Effects: Removes all elements equal to value from the vector while
	// optimizing for speed with the potential reordering of elements as a
	// side effect.
	//
	// Complexity: Linear
	//
	///////////////////////////////////////////////////////////////////////
	template <class T, class Allocator, class U>
	typename vector<T, Allocator>::size_type erase_unsorted(vector<T, Allocator>& c, const U& value)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	///////////////////////////////////////////////////////////////////////
	// erase_unsorted_if
	// 
	// This serves a similar purpose as erase_if above but with the
	// difference that it doesn't preserve the relative order of what is
	// left in the vector.
	//
	// Effects: Removes all elements that return true for the predicate
	// while optimizing for speed with the potential reordering of elements
	// as a side effect.
	//
	// Complexity: Linear
	//
	///////////////////////////////////////////////////////////////////////
	template <class T, class Allocator, class Predicate>
	typename vector<T, Allocator>::size_type erase_unsorted_if(vector<T, Allocator>& c, Predicate predicate)
	{
    __builtin_trap() /* STUB: not implemented */;
}

} // namespace eastl


EA_RESTORE_VC_WARNING();
EA_RESTORE_VC_WARNING();


#endif // Header include guard
