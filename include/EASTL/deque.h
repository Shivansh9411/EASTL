#include <cstdlib>
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) Electronic Arts Inc. All rights reserved.
/////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// deque design
//
// A deque (pronounced "deck") is a double-ended queue, though this is partially 
// of a misnomer. A deque does indeed let you add and remove values from both ends
// of the container, but it's not usually used for such a thing and instead is used
// as a more flexible version of a vector. It provides operator[] (random access) 
// and can insert items anywhere and not just at the front and back.
// 
// While you can implement a double-ended queue via a doubly-linked list, deque is 
// instead implemented as a list of arrays. The benefit of this is that memory usage 
// is lower and that random access can be had with decent efficiency. 
// 
// Our implementation of deque is just like every other implementation of deque,
// as the C++ standard all but dictates that you make it work this way. Below 
// we have a depiction of an array (or vector) of 48 items, with each node being 
// a '+' character and extra capacity being a '-' character. What we have is one 
// contiguous block of memory:
// 
//     ++++++++++++++++++++++++++++++++++++++++++++++++-----------------
//     0                                              47
// 
// With a deque, the same array of 48 items would be implemented as multiple smaller
// arrays of contiguous memory, each of fixed size. We will call these "sub-arrays."
// In the case here, we have six arrays of 8 nodes:
// 
//     ++++++++ ++++++++ ++++++++ ++++++++ ++++++++ ++++++++
// 
// With an vector, item [0] is the first item and item [47] is the last item. With a 
// deque, item [0] is usually not the first item and neither is item [47]. There is 
// extra capacity on both the front side and the back side of the deque. So a deque
// (of 24 items) actually looks like this:
// 
//     -------- -----+++ ++++++++ ++++++++ +++++--- --------
//                   0                         23
// 
// To insert items at the front, you move into the capacity on the left, and to insert
// items at the back, you append items on the right. As you can see, inserting an item
// at the front doesn't require allocating new memory nor does it require moving any 
// items in the container. It merely involves moving the pointer to the [0] item to
// the left by one node.
// 
// We keep track of these sub-arrays by having an array of pointers, with each array 
// entry pointing to each of the sub-arrays. We could alternatively use a linked
// list of pointers, but it turns out we can implement our deque::operator[] more 
// efficiently if we use an array of pointers instead of a list of pointers.
//
// To implement deque::iterator, we could keep a struct which is essentially this:
//     struct iterator {
//        int subArrayIndex;
//        int subArrayOffset;
//     }
//
// In practice, we implement iterators a little differently, but in reality our 
// implementation isn't much different from the above. It turns out that it's most
// simple if we also manage the location of item [0] and item [end] by using these
// same iterators.
//
// To consider: Implement the deque as a circular deque instead of a linear one.
//              This would use a similar subarray layout but iterators would
//              wrap around when they reached the end of the subarray pointer list.
//
//////////////////////////////////////////////////////////////////////////////


#ifndef EASTL_DEQUE_H
#define EASTL_DEQUE_H


#include <EASTL/internal/config.h>
#include <EASTL/allocator.h>
#include <EASTL/algorithm.h>
#include <EASTL/type_traits.h>
#include <EASTL/iterator.h>
#include <EASTL/memory.h>
#include <EASTL/initializer_list.h>

EA_DISABLE_ALL_VC_WARNINGS()
#include <new>
#include <stddef.h>
EA_RESTORE_ALL_VC_WARNINGS()

#if EASTL_EXCEPTIONS_ENABLED
	EA_DISABLE_ALL_VC_WARNINGS()
	#include <stdexcept> // std::out_of_range, std::length_error, std::logic_error.
	EA_RESTORE_ALL_VC_WARNINGS()
#endif


// 4267 - 'argument' : conversion from 'size_t' to 'const uint32_t', possible loss of data. This is a bogus warning resulting from a bug in VC++.
// 4345 - Behavior change: an object of POD type constructed with an initializer of the form () will be default-initialized
// 4480 - nonstandard extension used: specifying underlying type for enum
// 4530 - C++ exception handler used, but unwind semantics are not enabled. Specify /EHsc
// 4571 - catch(...) semantics changed since Visual C++ 7.1; structured exceptions (SEH) are no longer caught.
EA_DISABLE_VC_WARNING(4267 4345 4480 4530 4571);

#if EASTL_EXCEPTIONS_ENABLED
	// 4703 - potentially uninitialized local pointer variable used. VC++ is mistakenly analyzing the possibility of uninitialized variables, though it's not easy for it to do so.
	// 4701 - potentially uninitialized local variable used.
	EA_DISABLE_VC_WARNING(4703 4701)
#endif


#if defined(EA_PRAGMA_ONCE_SUPPORTED)
	#pragma once // Some compilers (e.g. VC++) benefit significantly from using this. We've measured 3-4% build speed improvements in apps as a result.
#endif


namespace eastl
{

	/// EASTL_DEQUE_DEFAULT_NAME
	///
	/// Defines a default container name in the absence of a user-provided name.
	///
	#ifndef EASTL_DEQUE_DEFAULT_NAME
		#define EASTL_DEQUE_DEFAULT_NAME EASTL_DEFAULT_NAME_PREFIX " deque" // Unless the user overrides something, this is "EASTL deque".
	#endif


	/// EASTL_DEQUE_DEFAULT_ALLOCATOR
	///
	#ifndef EASTL_DEQUE_DEFAULT_ALLOCATOR
		#define EASTL_DEQUE_DEFAULT_ALLOCATOR allocator_type(EASTL_DEQUE_DEFAULT_NAME)
	#endif


	/// DEQUE_DEFAULT_SUBARRAY_SIZE
	///
	/// Defines the default number of items in a subarray.
	/// Note that the user has the option of specifying the subarray size
	/// in the deque template declaration.
	///
	#if !defined(__GNUC__) || (__GNUC__ >= 3) // GCC 2.x can't handle the declaration below.
		#define DEQUE_DEFAULT_SUBARRAY_SIZE(T) ((sizeof(T) <= 4) ? 64 : ((sizeof(T) <= 8) ? 32 : ((sizeof(T) <= 16) ? 16 : ((sizeof(T) <= 32) ? 8 : 4))))
	#else
		#define DEQUE_DEFAULT_SUBARRAY_SIZE(T) 16
	#endif



	/// DequeIterator
	///
	/// The DequeIterator provides both const and non-const iterators for deque. 
	/// It also is used for the tracking of the begin and end for the deque.
	///
	template <typename T, typename Pointer, typename Reference, unsigned kDequeSubarraySize>
	struct DequeIterator
	{
		typedef DequeIterator<T, Pointer, Reference, kDequeSubarraySize>  this_type;
		typedef DequeIterator<T, T*, T&, kDequeSubarraySize>              iterator;
		typedef DequeIterator<T, const T*, const T&, kDequeSubarraySize>  const_iterator;
		typedef ptrdiff_t                                                 difference_type;
		typedef eastl::random_access_iterator_tag                  iterator_category;
		typedef T                                                         value_type;
		typedef T*                                                        pointer;
		typedef T&                                                        reference;

	public:
		DequeIterator();
		DequeIterator(const iterator& x);
		DequeIterator& operator=(const iterator& x);

		pointer   operator->() const;
		reference operator*() const;

		this_type& operator++();
		this_type  operator++(int);

		this_type& operator--();
		this_type  operator--(int);

		this_type& operator+=(difference_type n);
		this_type& operator-=(difference_type n);

		this_type operator+(difference_type n) const;
		this_type operator-(difference_type n) const;

	protected:
		template <typename, typename, typename, unsigned>
		friend struct DequeIterator;

		template <typename, typename, unsigned>
		friend struct DequeBase;

		template <typename, typename, unsigned>
		friend class deque;

		template <typename U, typename PointerA, typename ReferenceA, typename PointerB, typename ReferenceB, unsigned kDequeSubarraySizeU>
		friend bool operator==(const DequeIterator<U, PointerA, ReferenceA, kDequeSubarraySizeU>&, 
							   const DequeIterator<U, PointerB, ReferenceB, kDequeSubarraySizeU>&);

		template <typename U, typename PointerA, typename ReferenceA, typename PointerB, typename ReferenceB, unsigned kDequeSubarraySizeU>
		friend bool operator!=(const DequeIterator<U, PointerA, ReferenceA, kDequeSubarraySizeU>&, 
							   const DequeIterator<U, PointerB, ReferenceB, kDequeSubarraySizeU>&);

		template <typename U, typename PointerU, typename ReferenceU, unsigned kDequeSubarraySizeU>
		friend bool operator!=(const DequeIterator<U, PointerU, ReferenceU, kDequeSubarraySizeU>& a, 
							   const DequeIterator<U, PointerU, ReferenceU, kDequeSubarraySizeU>& b);

		template <typename U, typename PointerA, typename ReferenceA, typename PointerB, typename ReferenceB, unsigned kDequeSubarraySizeU>
		friend bool operator< (const DequeIterator<U, PointerA, ReferenceA, kDequeSubarraySizeU>&, 
							   const DequeIterator<U, PointerB, ReferenceB, kDequeSubarraySizeU>&);

		template <typename U, typename PointerA, typename ReferenceA, typename PointerB, typename ReferenceB, unsigned kDequeSubarraySizeU>
		friend bool operator> (const DequeIterator<U, PointerA, ReferenceA, kDequeSubarraySizeU>&, 
							   const DequeIterator<U, PointerB, ReferenceB, kDequeSubarraySizeU>&);

		template <typename U, typename PointerA, typename ReferenceA, typename PointerB, typename ReferenceB, unsigned kDequeSubarraySizeU>
		friend bool operator<=(const DequeIterator<U, PointerA, ReferenceA, kDequeSubarraySizeU>&, 
							   const DequeIterator<U, PointerB, ReferenceB, kDequeSubarraySizeU>&);

		template <typename U, typename PointerA, typename ReferenceA, typename PointerB, typename ReferenceB, unsigned kDequeSubarraySizeU>
		friend bool operator>=(const DequeIterator<U, PointerA, ReferenceA, kDequeSubarraySizeU>&, 
							   const DequeIterator<U, PointerB, ReferenceB, kDequeSubarraySizeU>&);

		template <typename U, typename PointerA, typename ReferenceA, typename PointerB, typename ReferenceB, unsigned kDequeSubarraySizeU>
		friend typename DequeIterator<U, PointerA, ReferenceA, kDequeSubarraySizeU>::difference_type
		operator-(const DequeIterator<U, PointerA, ReferenceA, kDequeSubarraySizeU>& a,
				  const DequeIterator<U, PointerB, ReferenceB, kDequeSubarraySizeU>& b);

	protected:
		T*  mpCurrent;          // Where we currently point. Declared first because it's used most often.
		T*  mpBegin;            // The beginning of the current subarray.
		T*  mpEnd;              // The end of the current subarray. To consider: remove this member, as it is always equal to 'mpBegin + kDequeSubarraySize'. Given that deque subarrays usually consist of hundreds of bytes, this isn't a massive win. Also, now that we are implementing a zero-allocation new deque policy, mpEnd may in fact not be equal to 'mpBegin + kDequeSubarraySize'.
		T** mpCurrentArrayPtr;  // Pointer to current subarray. We could alternatively implement this as a list node iterator if the deque used a linked list.

		struct Increment {};
		struct Decrement {};
		struct FromConst {};

		DequeIterator(T** pCurrentArrayPtr, T* pCurrent);
		DequeIterator(const const_iterator& x, FromConst) : mpCurrent(x.mpCurrent), mpBegin(x.mpBegin), mpEnd(x.mpEnd), mpCurrentArrayPtr(x.mpCurrentArrayPtr){
    __builtin_trap() /* STUB: not implemented */;
}
		DequeIterator(const iterator&       x, Increment);
		DequeIterator(const iterator&       x, Decrement);

		this_type move(const iterator& first, const iterator& last, true_type);  // true means that value_type has the type_trait is_trivially_copyable,
		this_type move(const iterator& first, const iterator& last, false_type); // false means it does not. 

		void move_backward(const iterator& first, const iterator& last, true_type);  // true means that value_type has the type_trait is_trivially_copyable,
		void move_backward(const iterator& first, const iterator& last, false_type); // false means it does not.

		void SetSubarray(T** pCurrentArrayPtr);
	};




	/// DequeBase
	///
	/// The DequeBase implements memory allocation for deque.
	/// See VectorBase (class vector) for an explanation of why we 
	/// create this separate base class.
	///
	template <typename T, typename Allocator, unsigned kDequeSubarraySize>
	struct DequeBase
	{
		typedef T                                                        value_type;
		typedef Allocator                                                allocator_type;
		typedef eastl_size_t                                             size_type;     // See config.h for the definition of eastl_size_t, which defaults to size_t.
		typedef ptrdiff_t                                                difference_type;
		typedef DequeIterator<T, T*, T&, kDequeSubarraySize>             iterator;
		typedef DequeIterator<T, const T*, const T&, kDequeSubarraySize> const_iterator;

		static const size_type npos     = (size_type)-1;      /// 'npos' means non-valid position or simply non-position.
		static const size_type kMaxSize = (size_type)-2;      /// -1 is reserved for 'npos'. It also happens to be slightly beneficial that kMaxSize is a value less than -1, as it helps us deal with potential integer wraparound issues.

		enum
		{
			kMinPtrArraySize = 8,                               /// A new empty deque has a ptrArraySize of 0, but any allocated ptrArrays use this min size.
			kSubarraySize    = kDequeSubarraySize               /// 
		  //kNodeSize        = kDequeSubarraySize * sizeof(T)   /// Disabled because it prevents the ability to do this: struct X{ eastl::deque<X, EASTLAllocatorType, 16> mDequeOfSelf; };
		};

	protected:
		enum Side       /// Defines the side of the deque: front or back.
		{
			kSideFront, /// Identifies the front side of the deque.
			kSideBack   /// Identifies the back side of the deque.
		};

		T**             mpPtrArray;         // Array of pointers to subarrays.
		size_type       mnPtrArraySize;     // Possibly we should store this as T** mpArrayEnd.
		iterator        mItBegin;           // Where within the subarrays is our beginning.
		iterator        mItEnd;             // Where within the subarrays is our end.
		allocator_type  mAllocator;         // To do: Use base class optimization to make this go away.

	public:
		DequeBase(const allocator_type& allocator);
		DequeBase(size_type n);
		DequeBase(size_type n, const allocator_type& allocator);
	   ~DequeBase();

		const allocator_type& get_allocator() const EA_NOEXCEPT;
		allocator_type&       get_allocator() EA_NOEXCEPT;
		void                  set_allocator(const allocator_type& allocator);

	protected:
		T*       DoAllocateSubarray();
		void     DoFreeSubarray(T* p);
		void     DoFreeSubarrays(T** pBegin, T** pEnd);

		T**      DoAllocatePtrArray(size_type n);
		void     DoFreePtrArray(T** p, size_t n);

		iterator DoReallocSubarray(size_type nAdditionalCapacity, Side allocationSide);
		void     DoReallocPtrArray(size_type nAdditionalCapacity, Side allocationSide);

		void     DoInit(size_type n);

	}; // DequeBase




	/// deque
	///
	/// Implements a conventional C++ double-ended queue. The implementation used here
	/// is very much like any other deque implementations you may have seen, as it 
	/// follows the standard algorithm for deque design. 
	///
	/// Note:
	/// As of this writing, deque does not support zero-allocation initial emptiness.
	/// A newly created deque with zero elements will still allocate a subarray
	/// pointer set. We are looking for efficient and clean ways to get around this,
	/// but current efforts have resulted in less efficient and more fragile code.
	/// The logic of this class doesn't lend itself to a clean implementation. 
	/// It turns out that deques are one of the least likely classes you'd want this
	/// behaviour in, so until this functionality becomes very important to somebody,
	/// we will leave it as-is. It can probably be solved by adding some extra code to
	/// the Do* functions and adding good comments explaining the situation.
	/// 
	template <typename T, typename Allocator = EASTLAllocatorType, unsigned kDequeSubarraySize = DEQUE_DEFAULT_SUBARRAY_SIZE(T)>
	class deque : public DequeBase<T, Allocator, kDequeSubarraySize>
	{
	public:
		typedef DequeBase<T, Allocator, kDequeSubarraySize>              base_type;
		typedef deque<T, Allocator, kDequeSubarraySize>                  this_type;
		typedef T                                                        value_type;
		typedef T*                                                       pointer;
		typedef const T*                                                 const_pointer;
		typedef T&                                                       reference;
		typedef const T&                                                 const_reference;
		typedef DequeIterator<T, T*, T&, kDequeSubarraySize>             iterator;
		typedef DequeIterator<T, const T*, const T&, kDequeSubarraySize> const_iterator;
		typedef eastl::reverse_iterator<iterator>                        reverse_iterator;
		typedef eastl::reverse_iterator<const_iterator>                  const_reverse_iterator;
		typedef typename base_type::size_type                            size_type;
		typedef typename base_type::difference_type                      difference_type;
		typedef typename base_type::allocator_type                       allocator_type;

		using base_type::npos;

		static_assert(!is_const<value_type>::value, "deque<T>::value_type must be non-const.");
		static_assert(!is_volatile<value_type>::value, "deque<T>::value_type must be non-volatile.");

	protected:
		using base_type::kSideFront;
		using base_type::kSideBack;
		using base_type::mpPtrArray;
		using base_type::mnPtrArraySize;
		using base_type::mItBegin;
		using base_type::mItEnd;
		using base_type::mAllocator;
		using base_type::DoAllocateSubarray;
		using base_type::DoFreeSubarray;
		using base_type::DoFreeSubarrays;
		using base_type::DoAllocatePtrArray;
		using base_type::DoFreePtrArray;
		using base_type::DoReallocSubarray;
		using base_type::DoReallocPtrArray;

	public:
		deque();
		explicit deque(const allocator_type& allocator);
		explicit deque(size_type n, const allocator_type& allocator = EASTL_DEQUE_DEFAULT_ALLOCATOR);
		deque(size_type n, const value_type& value, const allocator_type& allocator = EASTL_DEQUE_DEFAULT_ALLOCATOR);
		deque(const this_type& x);
		deque(this_type&& x);
		deque(this_type&& x, const allocator_type& allocator);
		deque(std::initializer_list<value_type> ilist, const allocator_type& allocator = EASTL_DEQUE_DEFAULT_ALLOCATOR);

		// note: this has pre-C++11 semantics:
		// this constructor is equivalent to the constructor deque(static_cast<size_type>(first), static_cast<value_type>(last)) if InputIterator is an integral type.
		template <typename InputIterator>
		deque(InputIterator first, InputIterator last); // allocator arg removed because VC7.1 fails on the default arg. To do: Make a second version of this function without a default arg.

	   ~deque();

		this_type& operator=(const this_type& x);
		this_type& operator=(std::initializer_list<value_type> ilist);
		this_type& operator=(this_type&& x);

		void swap(this_type& x);

		void assign(size_type n, const value_type& value);
		void assign(std::initializer_list<value_type> ilist);

		template <typename InputIterator>                       // It turns out that the C++ std::deque<int, int> specifies a two argument
		void assign(InputIterator first, InputIterator last);   // version of assign that takes (int size, int value). These are not 
																// iterators, so we need to do a template compiler trick to do the right thing.

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

		void resize(size_type n, const value_type& value);
		void resize(size_type n);

		void shrink_to_fit();
		void set_capacity(size_type n = base_type::npos);

		reference       operator[](size_type n);
		const_reference operator[](size_type n) const;

		reference       at(size_type n);
		const_reference at(size_type n) const;

		reference       front();
		const_reference front() const;

		reference       back();
		const_reference back() const;

		void      push_front(const value_type& value);
		reference push_front();
		void      push_front(value_type&& value);

		void      push_back(const value_type& value);
		reference push_back();
		void      push_back(value_type&& value);

		void pop_front();
		void pop_back();

		template<class... Args>
		iterator emplace(const_iterator position, Args&&... args);

		template<class... Args>
		reference emplace_front(Args&&... args);

		template<class... Args>
		reference emplace_back(Args&&... args);

		iterator insert(const_iterator position, const value_type& value);
		iterator insert(const_iterator position, value_type&& value);
		iterator insert(const_iterator position, size_type n, const value_type& value);
		iterator insert(const_iterator position, std::initializer_list<value_type> ilist);

		// note: this has pre-C++11 semantics:
		// this function is equivalent to insert(const_iterator position, static_cast<size_type>(first), static_cast<value_type>(last)) if InputIterator is an integral type.
		// ie. same as insert(const_iterator position, size_type n, const value_type& value)
		template <typename InputIterator>
		iterator insert(const_iterator position, InputIterator first, InputIterator last);

		iterator         erase(const_iterator position);
		iterator         erase(const_iterator first, const_iterator last);
		reverse_iterator erase(reverse_iterator position);
		reverse_iterator erase(reverse_iterator first, reverse_iterator last);

		void clear();
		//void reset_lose_memory(); // Disabled until it can be implemented efficiently and cleanly.  // This is a unilateral reset to an initially empty state. No destructors are called, no deallocation occurs.

		bool validate() const;
		int  validate_iterator(const_iterator i) const;

	protected:
		template <typename Integer>
		void DoInit(Integer n, Integer value, true_type);

		template <typename InputIterator>
		void DoInit(InputIterator first, InputIterator last, false_type);

		template <typename InputIterator>
		void DoInitFromIterator(InputIterator first, InputIterator last, eastl::input_iterator_tag);

		template <typename ForwardIterator>
		void DoInitFromIterator(ForwardIterator first, ForwardIterator last, eastl::forward_iterator_tag);

		void DoFillInit(const value_type& value);

		template <typename Integer>
		void DoAssign(Integer n, Integer value, true_type);

		template <typename InputIterator>
		void DoAssign(InputIterator first, InputIterator last, false_type);

		void DoAssignValues(size_type n, const value_type& value);

		template <typename Integer>
		iterator DoInsert(const const_iterator& position, Integer n, Integer value, true_type);

		template <typename InputIterator>
		iterator DoInsert(const const_iterator& position, const InputIterator& first, const InputIterator& last, false_type);

		template <typename InputIterator>
		iterator DoInsertFromIterator(const_iterator position, const InputIterator& first, const InputIterator& last, eastl::input_iterator_tag);

		template <typename ForwardIterator>
		iterator DoInsertFromIterator(const_iterator position, const ForwardIterator& first, const ForwardIterator& last, eastl::forward_iterator_tag);

		iterator DoInsertValues(const_iterator position, size_type n, const value_type& value);

		void DoSwap(this_type& x);
	}; // class deque




	///////////////////////////////////////////////////////////////////////
	// DequeBase
	///////////////////////////////////////////////////////////////////////

	template <typename T, typename Allocator, unsigned kDequeSubarraySize>
	DequeBase<T, Allocator, kDequeSubarraySize>::DequeBase(const allocator_type& allocator)
		: mpPtrArray(NULL),
		  mnPtrArraySize(0),
		  mItBegin(),
		  mItEnd(),
		  mAllocator(allocator)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator, unsigned kDequeSubarraySize>
	DequeBase<T, Allocator, kDequeSubarraySize>::DequeBase(size_type n)
		: mpPtrArray(NULL),
		  mnPtrArraySize(0),
		  mItBegin(),
		  mItEnd(),
		  mAllocator(EASTL_DEQUE_DEFAULT_NAME)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator, unsigned kDequeSubarraySize>
	DequeBase<T, Allocator, kDequeSubarraySize>::DequeBase(size_type n, const allocator_type& allocator)
		: mpPtrArray(NULL),
		  mnPtrArraySize(0),
		  mItBegin(),
		  mItEnd(),
		  mAllocator(allocator)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator, unsigned kDequeSubarraySize>
	DequeBase<T, Allocator, kDequeSubarraySize>::~DequeBase()
	{
		if(mpPtrArray)
		{
			DoFreeSubarrays(mItBegin.mpCurrentArrayPtr, mItEnd.mpCurrentArrayPtr + 1);
			DoFreePtrArray(mpPtrArray, mnPtrArraySize);
			mpPtrArray = nullptr;
		}
	}


	template <typename T, typename Allocator, unsigned kDequeSubarraySize>
	const typename DequeBase<T, Allocator, kDequeSubarraySize>::allocator_type&
	DequeBase<T, Allocator, kDequeSubarraySize>::get_allocator() const EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator, unsigned kDequeSubarraySize>
	typename DequeBase<T, Allocator, kDequeSubarraySize>::allocator_type&
	DequeBase<T, Allocator, kDequeSubarraySize>::get_allocator() EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator, unsigned kDequeSubarraySize>
	void DequeBase<T, Allocator, kDequeSubarraySize>::set_allocator(const allocator_type& allocator)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator, unsigned kDequeSubarraySize>
	T* DequeBase<T, Allocator, kDequeSubarraySize>::DoAllocateSubarray()
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator, unsigned kDequeSubarraySize>
	void DequeBase<T, Allocator, kDequeSubarraySize>::DoFreeSubarray(T* p)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T, typename Allocator, unsigned kDequeSubarraySize>
	void DequeBase<T, Allocator, kDequeSubarraySize>::DoFreeSubarrays(T** pBegin, T** pEnd)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T, typename Allocator, unsigned kDequeSubarraySize>
	T** DequeBase<T, Allocator, kDequeSubarraySize>::DoAllocatePtrArray(size_type n)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator, unsigned kDequeSubarraySize>
	void DequeBase<T, Allocator, kDequeSubarraySize>::DoFreePtrArray(T** pp, size_t n)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator, unsigned kDequeSubarraySize>
	typename DequeBase<T, Allocator, kDequeSubarraySize>::iterator
	DequeBase<T, Allocator, kDequeSubarraySize>::DoReallocSubarray(size_type nAdditionalCapacity, Side allocationSide)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator, unsigned kDequeSubarraySize>
	void DequeBase<T, Allocator, kDequeSubarraySize>::DoReallocPtrArray(size_type nAdditionalCapacity, Side allocationSide)
	{
    __builtin_trap() /* STUB: not implemented */;
}
	

	template <typename T, typename Allocator, unsigned kDequeSubarraySize>
	void DequeBase<T, Allocator, kDequeSubarraySize>::DoInit(size_type n)
	{
    __builtin_trap() /* STUB: not implemented */;
}



	///////////////////////////////////////////////////////////////////////
	// DequeIterator
	///////////////////////////////////////////////////////////////////////

	template <typename T, typename Pointer, typename Reference, unsigned kDequeSubarraySize>
	DequeIterator<T, Pointer, Reference, kDequeSubarraySize>::DequeIterator()
		: mpCurrent(NULL), mpBegin(NULL), mpEnd(NULL), mpCurrentArrayPtr(NULL)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Pointer, typename Reference, unsigned kDequeSubarraySize>
	DequeIterator<T, Pointer, Reference, kDequeSubarraySize>::DequeIterator(T** pCurrentArrayPtr, T* pCurrent)
		: mpCurrent(pCurrent), mpBegin(*pCurrentArrayPtr), mpEnd(pCurrent + kDequeSubarraySize), mpCurrentArrayPtr(pCurrentArrayPtr)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Pointer, typename Reference, unsigned kDequeSubarraySize>
	DequeIterator<T, Pointer, Reference, kDequeSubarraySize>::DequeIterator(const iterator& x)
		: mpCurrent(x.mpCurrent), mpBegin(x.mpBegin), mpEnd(x.mpEnd), mpCurrentArrayPtr(x.mpCurrentArrayPtr)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T, typename Pointer, typename Reference, unsigned kDequeSubarraySize>
	DequeIterator<T, Pointer, Reference, kDequeSubarraySize>& DequeIterator<T, Pointer, Reference, kDequeSubarraySize>::operator=(const iterator& x)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Pointer, typename Reference, unsigned kDequeSubarraySize>
	DequeIterator<T, Pointer, Reference, kDequeSubarraySize>::DequeIterator(const iterator& x, Increment)
		: mpCurrent(x.mpCurrent), mpBegin(x.mpBegin), mpEnd(x.mpEnd), mpCurrentArrayPtr(x.mpCurrentArrayPtr)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Pointer, typename Reference, unsigned kDequeSubarraySize>
	DequeIterator<T, Pointer, Reference, kDequeSubarraySize>::DequeIterator(const iterator& x, Decrement)
		: mpCurrent(x.mpCurrent), mpBegin(x.mpBegin), mpEnd(x.mpEnd), mpCurrentArrayPtr(x.mpCurrentArrayPtr)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Pointer, typename Reference, unsigned kDequeSubarraySize>
	typename DequeIterator<T, Pointer, Reference, kDequeSubarraySize>::pointer
	DequeIterator<T, Pointer, Reference, kDequeSubarraySize>::operator->() const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Pointer, typename Reference, unsigned kDequeSubarraySize>
	typename DequeIterator<T, Pointer, Reference, kDequeSubarraySize>::reference
	DequeIterator<T, Pointer, Reference, kDequeSubarraySize>::operator*() const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Pointer, typename Reference, unsigned kDequeSubarraySize>
	typename DequeIterator<T, Pointer, Reference, kDequeSubarraySize>::this_type&
	DequeIterator<T, Pointer, Reference, kDequeSubarraySize>::operator++()
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Pointer, typename Reference, unsigned kDequeSubarraySize>
	typename DequeIterator<T, Pointer, Reference, kDequeSubarraySize>::this_type
	DequeIterator<T, Pointer, Reference, kDequeSubarraySize>::operator++(int)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Pointer, typename Reference, unsigned kDequeSubarraySize>
	typename DequeIterator<T, Pointer, Reference, kDequeSubarraySize>::this_type&
	DequeIterator<T, Pointer, Reference, kDequeSubarraySize>::operator--()
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Pointer, typename Reference, unsigned kDequeSubarraySize>
	typename DequeIterator<T, Pointer, Reference, kDequeSubarraySize>::this_type
	DequeIterator<T, Pointer, Reference, kDequeSubarraySize>::operator--(int)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Pointer, typename Reference, unsigned kDequeSubarraySize>
	typename DequeIterator<T, Pointer, Reference, kDequeSubarraySize>::this_type&
	DequeIterator<T, Pointer, Reference, kDequeSubarraySize>::operator+=(difference_type n)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Pointer, typename Reference, unsigned kDequeSubarraySize>
	typename DequeIterator<T, Pointer, Reference, kDequeSubarraySize>::this_type&
	DequeIterator<T, Pointer, Reference, kDequeSubarraySize>::operator-=(difference_type n)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Pointer, typename Reference, unsigned kDequeSubarraySize>
	typename DequeIterator<T, Pointer, Reference, kDequeSubarraySize>::this_type
	DequeIterator<T, Pointer, Reference, kDequeSubarraySize>::operator+(difference_type n) const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Pointer, typename Reference, unsigned kDequeSubarraySize>
	typename DequeIterator<T, Pointer, Reference, kDequeSubarraySize>::this_type
	DequeIterator<T, Pointer, Reference, kDequeSubarraySize>::operator-(difference_type n) const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Pointer, typename Reference, unsigned kDequeSubarraySize>
	typename DequeIterator<T, Pointer, Reference, kDequeSubarraySize>::this_type
	DequeIterator<T, Pointer, Reference, kDequeSubarraySize>::move(const iterator& first, const iterator& last, true_type)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Pointer, typename Reference, unsigned kDequeSubarraySize>
	typename DequeIterator<T, Pointer, Reference, kDequeSubarraySize>::this_type
	DequeIterator<T, Pointer, Reference, kDequeSubarraySize>::move(const iterator& first, const iterator& last, false_type)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Pointer, typename Reference, unsigned kDequeSubarraySize>
	void DequeIterator<T, Pointer, Reference, kDequeSubarraySize>::move_backward(const iterator& first, const iterator& last, true_type)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Pointer, typename Reference, unsigned kDequeSubarraySize>
	void DequeIterator<T, Pointer, Reference, kDequeSubarraySize>::move_backward(const iterator& first, const iterator& last, false_type)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Pointer, typename Reference, unsigned kDequeSubarraySize>
	void DequeIterator<T, Pointer, Reference, kDequeSubarraySize>::SetSubarray(T** pCurrentArrayPtr)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	// The C++ defect report #179 requires that we support comparisons between const and non-const iterators.
	// Thus we provide additional template paremeters here to support this. The defect report does not
	// require us to support comparisons between reverse_iterators and const_reverse_iterators.
	template <typename T, typename PointerA, typename ReferenceA, typename PointerB, typename ReferenceB, unsigned kDequeSubarraySize>
	inline bool operator==(const DequeIterator<T, PointerA, ReferenceA, kDequeSubarraySize>& a, 
						   const DequeIterator<T, PointerB, ReferenceB, kDequeSubarraySize>& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename PointerA, typename ReferenceA, typename PointerB, typename ReferenceB, unsigned kDequeSubarraySize>
	inline bool operator!=(const DequeIterator<T, PointerA, ReferenceA, kDequeSubarraySize>& a, 
						   const DequeIterator<T, PointerB, ReferenceB, kDequeSubarraySize>& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	// We provide a version of operator!= for the case where the iterators are of the 
	// same type. This helps prevent ambiguity errors in the presence of rel_ops.
	template <typename T, typename Pointer, typename Reference, unsigned kDequeSubarraySize>
	inline bool operator!=(const DequeIterator<T, Pointer, Reference, kDequeSubarraySize>& a, 
						   const DequeIterator<T, Pointer, Reference, kDequeSubarraySize>& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename PointerA, typename ReferenceA, typename PointerB, typename ReferenceB, unsigned kDequeSubarraySize>
	inline bool operator<(const DequeIterator<T, PointerA, ReferenceA, kDequeSubarraySize>& a, 
						  const DequeIterator<T, PointerB, ReferenceB, kDequeSubarraySize>& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename PointerA, typename ReferenceA, typename PointerB, typename ReferenceB, unsigned kDequeSubarraySize>
	inline bool operator>(const DequeIterator<T, PointerA, ReferenceA, kDequeSubarraySize>& a, 
						  const DequeIterator<T, PointerB, ReferenceB, kDequeSubarraySize>& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename PointerA, typename ReferenceA, typename PointerB, typename ReferenceB, unsigned kDequeSubarraySize>
	inline bool operator<=(const DequeIterator<T, PointerA, ReferenceA, kDequeSubarraySize>& a, 
						   const DequeIterator<T, PointerB, ReferenceB, kDequeSubarraySize>& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename PointerA, typename ReferenceA, typename PointerB, typename ReferenceB, unsigned kDequeSubarraySize>
	inline bool operator>=(const DequeIterator<T, PointerA, ReferenceA, kDequeSubarraySize>& a, 
						   const DequeIterator<T, PointerB, ReferenceB, kDequeSubarraySize>& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	// Random access iterators must support operator + and operator -.
	// You can only add an integer to an iterator, and you cannot add two iterators.
	template <typename T, typename Pointer, typename Reference, unsigned kDequeSubarraySize>
	inline DequeIterator<T, Pointer, Reference, kDequeSubarraySize>
	operator+(ptrdiff_t n, const DequeIterator<T, Pointer, Reference, kDequeSubarraySize>& x)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	// You can only add an integer to an iterator, but you can subtract two iterators.
	// The C++ defect report #179 mentioned above specifically refers to 
	// operator - and states that we support the subtraction of const and non-const iterators.
	template <typename T, typename PointerA, typename ReferenceA, typename PointerB, typename ReferenceB, unsigned kDequeSubarraySize>
	inline typename DequeIterator<T, PointerA, ReferenceA, kDequeSubarraySize>::difference_type
	operator-(const DequeIterator<T, PointerA, ReferenceA, kDequeSubarraySize>& a,
			  const DequeIterator<T, PointerB, ReferenceB, kDequeSubarraySize>& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}




	///////////////////////////////////////////////////////////////////////
	// deque
	///////////////////////////////////////////////////////////////////////

	template <typename T, typename Allocator, unsigned kDequeSubarraySize>
	inline deque<T, Allocator, kDequeSubarraySize>::deque()
		: base_type((size_type)0)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator, unsigned kDequeSubarraySize>
	inline deque<T, Allocator, kDequeSubarraySize>::deque(const allocator_type& allocator)
		: base_type((size_type)0, allocator)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator, unsigned kDequeSubarraySize>
	inline deque<T, Allocator, kDequeSubarraySize>::deque(size_type n, const allocator_type& allocator)
		: base_type(n, allocator)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator, unsigned kDequeSubarraySize>
	inline deque<T, Allocator, kDequeSubarraySize>::deque(size_type n, const value_type& value, const allocator_type& allocator)
		: base_type(n, allocator)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator, unsigned kDequeSubarraySize>
	inline deque<T, Allocator, kDequeSubarraySize>::deque(const this_type& x)
		: base_type(x.size(), x.mAllocator)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator, unsigned kDequeSubarraySize>
	inline deque<T, Allocator, kDequeSubarraySize>::deque(this_type&& x)
	  : base_type((size_type)0, x.mAllocator)
	{
		swap(x);
	}


	template <typename T, typename Allocator, unsigned kDequeSubarraySize>
	inline deque<T, Allocator, kDequeSubarraySize>::deque(this_type&& x, const allocator_type& allocator)
	  : base_type((size_type)0, allocator)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator, unsigned kDequeSubarraySize>
	inline deque<T, Allocator, kDequeSubarraySize>::deque(std::initializer_list<value_type> ilist, const allocator_type& allocator)
		: base_type(allocator)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator, unsigned kDequeSubarraySize>
	template <typename InputIterator>
	inline deque<T, Allocator, kDequeSubarraySize>::deque(InputIterator first, InputIterator last)
		: base_type(EASTL_DEQUE_DEFAULT_ALLOCATOR) // Call the empty base constructor, which does nothing. We need to do all the work in our own DoInit.
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator, unsigned kDequeSubarraySize>
	inline deque<T, Allocator, kDequeSubarraySize>::~deque()
	{
		// Call destructors. Parent class will free the memory.
		for(iterator itCurrent(mItBegin); itCurrent != mItEnd; ++itCurrent)
			itCurrent.mpCurrent->~value_type();
	} 


	template <typename T, typename Allocator, unsigned kDequeSubarraySize>
	typename deque<T, Allocator, kDequeSubarraySize>::this_type& 
	deque<T, Allocator, kDequeSubarraySize>::operator=(const this_type& x)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator, unsigned kDequeSubarraySize>
	inline typename deque<T, Allocator, kDequeSubarraySize>::this_type& 
	deque<T, Allocator, kDequeSubarraySize>::operator=(this_type&& x)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator, unsigned kDequeSubarraySize>
	inline typename deque<T, Allocator, kDequeSubarraySize>::this_type& 
	deque<T, Allocator, kDequeSubarraySize>::operator=(std::initializer_list<value_type> ilist)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator, unsigned kDequeSubarraySize>
	inline void deque<T, Allocator, kDequeSubarraySize>::assign(size_type n, const value_type& value)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator, unsigned kDequeSubarraySize>
	inline void deque<T, Allocator, kDequeSubarraySize>::assign(std::initializer_list<value_type> ilist)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	// It turns out that the C++ std::deque specifies a two argument
	// version of assign that takes (int size, int value). These are not
	// iterators, so we need to do a template compiler trick to do the right thing. 
	template <typename T, typename Allocator, unsigned kDequeSubarraySize>
	template <typename InputIterator>
	inline void deque<T, Allocator, kDequeSubarraySize>::assign(InputIterator first, InputIterator last)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator, unsigned kDequeSubarraySize>
	inline typename deque<T, Allocator, kDequeSubarraySize>::iterator 
	deque<T, Allocator, kDequeSubarraySize>::begin() EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator, unsigned kDequeSubarraySize>
	inline typename deque<T, Allocator, kDequeSubarraySize>::const_iterator 
	deque<T, Allocator, kDequeSubarraySize>::begin() const EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator, unsigned kDequeSubarraySize>
	inline typename deque<T, Allocator, kDequeSubarraySize>::const_iterator 
	deque<T, Allocator, kDequeSubarraySize>::cbegin() const EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator, unsigned kDequeSubarraySize>
	inline typename deque<T, Allocator, kDequeSubarraySize>::iterator 
	deque<T, Allocator, kDequeSubarraySize>::end() EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator, unsigned kDequeSubarraySize>
	typename deque<T, Allocator, kDequeSubarraySize>::const_iterator
	deque<T, Allocator, kDequeSubarraySize>::end() const EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator, unsigned kDequeSubarraySize>
	inline typename deque<T, Allocator, kDequeSubarraySize>::const_iterator
	deque<T, Allocator, kDequeSubarraySize>::cend() const EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator, unsigned kDequeSubarraySize>
	inline typename deque<T, Allocator, kDequeSubarraySize>::reverse_iterator
	deque<T, Allocator, kDequeSubarraySize>::rbegin() EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator, unsigned kDequeSubarraySize>
	inline typename deque<T, Allocator, kDequeSubarraySize>::const_reverse_iterator
	deque<T, Allocator, kDequeSubarraySize>::rbegin() const EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator, unsigned kDequeSubarraySize>
	inline typename deque<T, Allocator, kDequeSubarraySize>::const_reverse_iterator
	deque<T, Allocator, kDequeSubarraySize>::crbegin() const EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator, unsigned kDequeSubarraySize>
	inline typename deque<T, Allocator, kDequeSubarraySize>::reverse_iterator
	deque<T, Allocator, kDequeSubarraySize>::rend() EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator, unsigned kDequeSubarraySize>
	inline typename deque<T, Allocator, kDequeSubarraySize>::const_reverse_iterator
	deque<T, Allocator, kDequeSubarraySize>::rend() const EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator, unsigned kDequeSubarraySize>
	inline typename deque<T, Allocator, kDequeSubarraySize>::const_reverse_iterator
	deque<T, Allocator, kDequeSubarraySize>::crend() const EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator, unsigned kDequeSubarraySize>
	inline bool deque<T, Allocator, kDequeSubarraySize>::empty() const EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator, unsigned kDequeSubarraySize>
	typename deque<T, Allocator, kDequeSubarraySize>::size_type
	inline deque<T, Allocator, kDequeSubarraySize>::size() const EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator, unsigned kDequeSubarraySize>
	inline void deque<T, Allocator, kDequeSubarraySize>::resize(size_type n, const value_type& value)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator, unsigned kDequeSubarraySize>
	inline void deque<T, Allocator, kDequeSubarraySize>::resize(size_type n)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator, unsigned kDequeSubarraySize>
	inline void deque<T, Allocator, kDequeSubarraySize>::shrink_to_fit()
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator, unsigned kDequeSubarraySize>
	inline void deque<T, Allocator, kDequeSubarraySize>::set_capacity(size_type n)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator, unsigned kDequeSubarraySize>
	typename deque<T, Allocator, kDequeSubarraySize>::reference
	deque<T, Allocator, kDequeSubarraySize>::operator[](size_type n)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator, unsigned kDequeSubarraySize>
	typename deque<T, Allocator, kDequeSubarraySize>::const_reference
	deque<T, Allocator, kDequeSubarraySize>::operator[](size_type n) const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator, unsigned kDequeSubarraySize>
	typename deque<T, Allocator, kDequeSubarraySize>::reference
	deque<T, Allocator, kDequeSubarraySize>::at(size_type n)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator, unsigned kDequeSubarraySize>
	typename deque<T, Allocator, kDequeSubarraySize>::const_reference
	deque<T, Allocator, kDequeSubarraySize>::at(size_type n) const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator, unsigned kDequeSubarraySize>
	typename deque<T, Allocator, kDequeSubarraySize>::reference
	deque<T, Allocator, kDequeSubarraySize>::front()
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator, unsigned kDequeSubarraySize>
	typename deque<T, Allocator, kDequeSubarraySize>::const_reference
	deque<T, Allocator, kDequeSubarraySize>::front() const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator, unsigned kDequeSubarraySize>
	typename deque<T, Allocator, kDequeSubarraySize>::reference
	deque<T, Allocator, kDequeSubarraySize>::back()
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator, unsigned kDequeSubarraySize>
	typename deque<T, Allocator, kDequeSubarraySize>::const_reference
	deque<T, Allocator, kDequeSubarraySize>::back() const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator, unsigned kDequeSubarraySize>
	void deque<T, Allocator, kDequeSubarraySize>::push_front(const value_type& value)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator, unsigned kDequeSubarraySize>
	void deque<T, Allocator, kDequeSubarraySize>::push_front(value_type&& value)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator, unsigned kDequeSubarraySize>
	typename deque<T, Allocator, kDequeSubarraySize>::reference
	deque<T, Allocator, kDequeSubarraySize>::push_front()
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator, unsigned kDequeSubarraySize>
	void deque<T, Allocator, kDequeSubarraySize>::push_back(const value_type& value)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator, unsigned kDequeSubarraySize>
	void deque<T, Allocator, kDequeSubarraySize>::push_back(value_type&& value)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator, unsigned kDequeSubarraySize>
	typename deque<T, Allocator, kDequeSubarraySize>::reference
	deque<T, Allocator, kDequeSubarraySize>::push_back()
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator, unsigned kDequeSubarraySize>
	void deque<T, Allocator, kDequeSubarraySize>::pop_front()
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator, unsigned kDequeSubarraySize>
	void deque<T, Allocator, kDequeSubarraySize>::pop_back()
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator, unsigned kDequeSubarraySize>
	template<class... Args>
	typename deque<T, Allocator, kDequeSubarraySize>::iterator
	deque<T, Allocator, kDequeSubarraySize>::emplace(const_iterator position, Args&&... args)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T, typename Allocator, unsigned kDequeSubarraySize>
	template<class... Args>
	typename deque<T, Allocator, kDequeSubarraySize>::reference deque<T, Allocator, kDequeSubarraySize>::emplace_front(Args&&... args)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T, typename Allocator, unsigned kDequeSubarraySize>
	template<class... Args>
	typename deque<T, Allocator, kDequeSubarraySize>::reference deque<T, Allocator, kDequeSubarraySize>::emplace_back(Args&&... args)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator, unsigned kDequeSubarraySize>
	typename deque<T, Allocator, kDequeSubarraySize>::iterator
	deque<T, Allocator, kDequeSubarraySize>::insert(const_iterator position, const value_type& value)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator, unsigned kDequeSubarraySize>
	typename deque<T, Allocator, kDequeSubarraySize>::iterator
	deque<T, Allocator, kDequeSubarraySize>::insert(const_iterator position, value_type&& value)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator, unsigned kDequeSubarraySize>
	typename deque<T, Allocator, kDequeSubarraySize>::iterator
	deque<T, Allocator, kDequeSubarraySize>::insert(const_iterator position, size_type n, const value_type& value)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator, unsigned kDequeSubarraySize>
	template <typename InputIterator>
	typename deque<T, Allocator, kDequeSubarraySize>::iterator
	deque<T, Allocator, kDequeSubarraySize>::insert(const_iterator position, InputIterator first, InputIterator last)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator, unsigned kDequeSubarraySize>
	typename deque<T, Allocator, kDequeSubarraySize>::iterator
	deque<T, Allocator, kDequeSubarraySize>::insert(const_iterator position, std::initializer_list<value_type> ilist)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator, unsigned kDequeSubarraySize>
	typename deque<T, Allocator, kDequeSubarraySize>::iterator
	deque<T, Allocator, kDequeSubarraySize>::erase(const_iterator position)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator, unsigned kDequeSubarraySize>
	typename deque<T, Allocator, kDequeSubarraySize>::iterator
	deque<T, Allocator, kDequeSubarraySize>::erase(const_iterator first, const_iterator last)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator, unsigned kDequeSubarraySize>
	typename deque<T, Allocator, kDequeSubarraySize>::reverse_iterator
	deque<T, Allocator, kDequeSubarraySize>::erase(reverse_iterator position)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator, unsigned kDequeSubarraySize>
	typename deque<T, Allocator, kDequeSubarraySize>::reverse_iterator
	deque<T, Allocator, kDequeSubarraySize>::erase(reverse_iterator first, reverse_iterator last)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator, unsigned kDequeSubarraySize>
	void deque<T, Allocator, kDequeSubarraySize>::clear()
	{
    __builtin_trap() /* STUB: not implemented */;
}


	//template <typename T, typename Allocator, unsigned kDequeSubarraySize>
	//void deque<T, Allocator, kDequeSubarraySize>::reset_lose_memory()
	//{
	//    // The reset_lose_memory function is a special extension function which unilaterally 
	//    // resets the container to an empty state without freeing the memory of 
	//    // the contained objects. This is useful for very quickly tearing down a 
	//    // container built into scratch memory.
	//
	//    // Currently we are unable to get this reset_lose_memory operation to work correctly 
	//    // as we haven't been able to find a good way to have a deque initialize
	//    // without allocating memory. We can lose the old memory, but DoInit 
	//    // would necessarily do a ptrArray allocation. And this is not within
	//    // our definition of how reset_lose_memory works.
	//    base_type::DoInit(0);
	//
	//}


	template <typename T, typename Allocator, unsigned kDequeSubarraySize>
	void deque<T, Allocator, kDequeSubarraySize>::swap(deque& x)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator, unsigned kDequeSubarraySize>
	template <typename Integer>
	void deque<T, Allocator, kDequeSubarraySize>::DoInit(Integer n, Integer value, true_type)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator, unsigned kDequeSubarraySize>
	template <typename InputIterator>
	void deque<T, Allocator, kDequeSubarraySize>::DoInit(InputIterator first, InputIterator last, false_type)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator, unsigned kDequeSubarraySize>
	template <typename InputIterator>
	void deque<T, Allocator, kDequeSubarraySize>::DoInitFromIterator(InputIterator first, InputIterator last, eastl::input_iterator_tag)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator, unsigned kDequeSubarraySize>
	template <typename ForwardIterator>
	void deque<T, Allocator, kDequeSubarraySize>::DoInitFromIterator(ForwardIterator first, ForwardIterator last, eastl::forward_iterator_tag)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator, unsigned kDequeSubarraySize>
	void deque<T, Allocator, kDequeSubarraySize>::DoFillInit(const value_type& value)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator, unsigned kDequeSubarraySize>
	template <typename Integer>
	void deque<T, Allocator, kDequeSubarraySize>::DoAssign(Integer n, Integer value, true_type) // false_type means this is the integer version instead of iterator version.
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator, unsigned kDequeSubarraySize>
	template <typename InputIterator>
	void deque<T, Allocator, kDequeSubarraySize>::DoAssign(InputIterator first, InputIterator last, false_type) // false_type means this is the iterator version instead of integer version.
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator, unsigned kDequeSubarraySize>
	void deque<T, Allocator, kDequeSubarraySize>::DoAssignValues(size_type n, const value_type& value)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator, unsigned kDequeSubarraySize>
	template <typename Integer>
	typename deque<T, Allocator, kDequeSubarraySize>::iterator
	deque<T, Allocator, kDequeSubarraySize>::DoInsert(const const_iterator& position, Integer n, Integer value, true_type)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator, unsigned kDequeSubarraySize>
	template <typename InputIterator>
	typename deque<T, Allocator, kDequeSubarraySize>::iterator
	deque<T, Allocator, kDequeSubarraySize>::DoInsert(const const_iterator& position, const InputIterator& first, const InputIterator& last, false_type)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T, typename Allocator, unsigned kDequeSubarraySize>
	template <typename InputIterator>
	typename deque<T, Allocator, kDequeSubarraySize>::iterator
	deque<T, Allocator, kDequeSubarraySize>::DoInsertFromIterator(const_iterator position, const InputIterator& first, const InputIterator& last, eastl::input_iterator_tag)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T, typename Allocator, unsigned kDequeSubarraySize>
	template <typename ForwardIterator>
	typename deque<T, Allocator, kDequeSubarraySize>::iterator
	deque<T, Allocator, kDequeSubarraySize>::DoInsertFromIterator(const_iterator position, const ForwardIterator& first, const ForwardIterator& last, eastl::forward_iterator_tag)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator, unsigned kDequeSubarraySize>
	typename deque<T, Allocator, kDequeSubarraySize>::iterator
	deque<T, Allocator, kDequeSubarraySize>::DoInsertValues(const_iterator position, size_type n, const value_type& value)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator, unsigned kDequeSubarraySize>
	inline void deque<T, Allocator, kDequeSubarraySize>::DoSwap(this_type& x)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator, unsigned kDequeSubarraySize>
	inline bool deque<T, Allocator, kDequeSubarraySize>::validate() const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator, unsigned kDequeSubarraySize>
	inline int deque<T, Allocator, kDequeSubarraySize>::validate_iterator(const_iterator i) const
	{
    __builtin_trap() /* STUB: not implemented */;
}



	///////////////////////////////////////////////////////////////////////
	// global operators
	///////////////////////////////////////////////////////////////////////

	template <typename T, typename Allocator, unsigned kDequeSubarraySize>
	inline bool operator==(const deque<T, Allocator, kDequeSubarraySize>& a, const deque<T, Allocator, kDequeSubarraySize>& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}

#if defined(EA_COMPILER_HAS_THREE_WAY_COMPARISON)
	template <typename T, typename Allocator, unsigned kDequeSubarraySize>
	inline synth_three_way_result<T> operator<=>(const deque<T, Allocator, kDequeSubarraySize>& a, const deque<T, Allocator, kDequeSubarraySize>& b)
	{
	    return eastl::lexicographical_compare_three_way(a.begin(), a.end(), b.begin(), b.end(), synth_three_way{});
	}

#else
	template <typename T, typename Allocator, unsigned kDequeSubarraySize>
	inline bool operator!=(const deque<T, Allocator, kDequeSubarraySize>& a, const deque<T, Allocator, kDequeSubarraySize>& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T, typename Allocator, unsigned kDequeSubarraySize>
	inline bool operator<(const deque<T, Allocator, kDequeSubarraySize>& a, const deque<T, Allocator, kDequeSubarraySize>& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T, typename Allocator, unsigned kDequeSubarraySize>
	inline bool operator>(const deque<T, Allocator, kDequeSubarraySize>& a, const deque<T, Allocator, kDequeSubarraySize>& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T, typename Allocator, unsigned kDequeSubarraySize>
	inline bool operator<=(const deque<T, Allocator, kDequeSubarraySize>& a, const deque<T, Allocator, kDequeSubarraySize>& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T, typename Allocator, unsigned kDequeSubarraySize>
	inline bool operator>=(const deque<T, Allocator, kDequeSubarraySize>& a, const deque<T, Allocator, kDequeSubarraySize>& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}
#endif

	template <typename T, typename Allocator, unsigned kDequeSubarraySize>
	inline void swap(deque<T, Allocator, kDequeSubarraySize>& a, deque<T, Allocator, kDequeSubarraySize>& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	///////////////////////////////////////////////////////////////////////
	// erase / erase_if
	//
	// https://en.cppreference.com/w/cpp/container/deque/erase2
	///////////////////////////////////////////////////////////////////////
	template <class T, class Allocator, class U>
	typename deque<T, Allocator>::size_type erase(deque<T, Allocator>& c, const U& value)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <class T, class Allocator, class Predicate>
	typename deque<T, Allocator>::size_type erase_if(deque<T, Allocator>& c, Predicate predicate)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	///////////////////////////////////////////////////////////////////////
	// erase_unsorted
	//
	// This serves a similar purpose as erase above but with the difference
	// that it doesn't preserve the relative order of what is left in the
	// deque.
	//
	// Effects: Removes all elements equal to value from the deque while
	// optimizing for speed with the potential reordering of elements as a
	// side effect.
	//
	// Complexity: Linear
	//
	///////////////////////////////////////////////////////////////////////
	template <class T, class Allocator, unsigned SubArraySize, class U>
	typename deque<T, Allocator, SubArraySize>::size_type erase_unsorted(deque<T, Allocator, SubArraySize>& c, const U& value)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	///////////////////////////////////////////////////////////////////////
	// erase_unsorted_if
	//
	// This serves a similar purpose as erase_if above but with the
	// difference that it doesn't preserve the relative order of what is
	// left in the deque.
	//
	// Effects: Removes all elements that return true for the predicate
	// while optimizing for speed with the potential reordering of elements
	// as a side effect.
	//
	// Complexity: Linear
	//
	///////////////////////////////////////////////////////////////////////
	template <class T, class Allocator, class Predicate, unsigned SubArraySize>
	typename deque<T, Allocator, SubArraySize>::size_type erase_unsorted_if(deque<T, Allocator, SubArraySize>& c, Predicate predicate)
	{
    __builtin_trap() /* STUB: not implemented */;
}

} // namespace eastl


EA_RESTORE_VC_WARNING();
#if EASTL_EXCEPTIONS_ENABLED
	EA_RESTORE_VC_WARNING();
#endif


#endif // Header include guard
