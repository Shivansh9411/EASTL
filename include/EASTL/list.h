#include <cstdlib>
///////////////////////////////////////////////////////////////////////////////
// Copyright (c) Electronic Arts Inc. All rights reserved.
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// This file implements a doubly-linked list, much like the C++ std::list class.
// The primary distinctions between this list and std::list are:
//    - list doesn't implement some of the less-frequently used functions 
//      of std::list. Any required functions can be added at a later time.
//    - list has a couple extension functions that increase performance.
//    - list can contain objects with alignment requirements. std::list cannot
//      do so without a bit of tedious non-portable effort.
//    - list has optimizations that don't exist in the STL implementations 
//      supplied by library vendors for our targeted platforms.
//    - list supports debug memory naming natively.
//    - list::size() by default is not a constant time function, like the list::size 
//      in some std implementations such as STLPort and SGI STL but unlike the 
//      list in Dinkumware and Metrowerks. The EASTL_LIST_SIZE_CACHE option can change this.
//    - list provides a guaranteed portable node definition that allows users
//      to write custom fixed size node allocators that are portable.
//    - list is easier to read, debug, and visualize.
//    - list is savvy to an environment that doesn't have exception handling,
//      as is sometimes the case with console or embedded environments.
//    - list has less deeply nested function calls and allows the user to 
//      enable forced inlining in debug builds in order to reduce bloat.
//    - list doesn't keep a member size variable. This means that list is 
//      smaller than std::list (depends on std::list) and that for most operations
//      it is faster than std::list. However, the list::size function is slower.
//    - list::size_type is defined as eastl_size_t instead of size_t in order to 
//      save memory and run faster on 64 bit systems.
///////////////////////////////////////////////////////////////////////////////


#ifndef EASTL_LIST_H
#define EASTL_LIST_H


#include <EASTL/internal/config.h>
#include <EASTL/allocator.h>
#include <EASTL/type_traits.h>
#include <EASTL/iterator.h>
#include <EASTL/algorithm.h>
#include <EASTL/initializer_list.h>
#include <EASTL/memory.h>
#include <EASTL/bonus/compressed_pair.h>
#if EASTL_EXCEPTIONS_ENABLED
#include <stdexcept>
#endif

EA_DISABLE_ALL_VC_WARNINGS()
#include <new>
#include <stddef.h>
EA_RESTORE_ALL_VC_WARNINGS()


// 4530 - C++ exception handler used, but unwind semantics are not enabled. Specify /EHsc
// 4345 - Behavior change: an object of POD type constructed with an initializer of the form () will be default-initialized
// 4571 - catch(...) semantics changed since Visual C++ 7.1; structured exceptions (SEH) are no longer caught.
// 4623 - default constructor was implicitly defined as deleted
EA_DISABLE_VC_WARNING(4530 4345 4571 4623);


#if defined(EA_PRAGMA_ONCE_SUPPORTED)
	#pragma once // Some compilers (e.g. VC++) benefit significantly from using this. We've measured 3-4% build speed improvements in apps as a result.
#endif



namespace eastl
{

	/// EASTL_LIST_DEFAULT_NAME
	///
	/// Defines a default container name in the absence of a user-provided name.
	///
	#ifndef EASTL_LIST_DEFAULT_NAME
		#define EASTL_LIST_DEFAULT_NAME EASTL_DEFAULT_NAME_PREFIX " list" // Unless the user overrides something, this is "EASTL list".
	#endif


	/// EASTL_LIST_DEFAULT_ALLOCATOR
	///
	#ifndef EASTL_LIST_DEFAULT_ALLOCATOR
		#define EASTL_LIST_DEFAULT_ALLOCATOR allocator_type(EASTL_LIST_DEFAULT_NAME)
	#endif



	/// ListNodeBase
	///
	/// We define a ListNodeBase separately from ListNode (below), because it allows
	/// us to have non-templated operations such as insert, remove (below), and it 
	/// makes it so that the list anchor node doesn't carry a T with it, which would
	/// waste space and possibly lead to surprising the user due to extra Ts existing
	/// that the user didn't explicitly create. The downside to all of this is that 
	/// it makes debug viewing of a list harder, given that the node pointers are of 
	/// type ListNodeBase and not ListNode. However, see ListNodeBaseProxy below.
	///
	struct ListNodeBase
	{
		ListNodeBase* mpNext;
		ListNodeBase* mpPrev;

		void        insert(ListNodeBase* pNext) EA_NOEXCEPT;                                // Inserts this standalone node before the node pNext in pNext's list. 
		void        remove() EA_NOEXCEPT;                                                   // Removes this node from the list it's in. Leaves this node's mpNext/mpPrev invalid.
		void        splice(ListNodeBase* pFirst, ListNodeBase* pLast) EA_NOEXCEPT;          // Removes [pFirst,pLast) from the list it's in and inserts it before this in this node's list.
		void        reverse() EA_NOEXCEPT;                                                  // Reverses the order of nodes in the circular list this node is a part of.
		static void swap(ListNodeBase& a, ListNodeBase& b) EA_NOEXCEPT;                     // Swaps the nodes a and b in the lists to which they belong.

		void        insert_range(ListNodeBase* pFirst, ListNodeBase* pFinal) EA_NOEXCEPT;   // Differs from splice in that first/final aren't in another list.
		static void remove_range(ListNodeBase* pFirst, ListNodeBase* pFinal) EA_NOEXCEPT;   // 
	};

	EA_DISABLE_VC_WARNING(4625 4626)
	template <typename T>
	struct ListNode : public ListNodeBase
	{
			T mValue;
		};
	EA_RESTORE_VC_WARNING()

	/// ListIterator
	///
	template <typename T, typename Pointer, typename Reference>
	struct ListIterator
	{
		typedef ListIterator<T, Pointer, Reference>         this_type;
		typedef ListIterator<T, T*, T&>                     iterator;
		typedef ListIterator<T, const T*, const T&>         const_iterator;
		typedef eastl_size_t                                size_type;     // See config.h for the definition of eastl_size_t, which defaults to size_t.
		typedef ptrdiff_t                                   difference_type;
		typedef T                                           value_type;
		typedef ListNodeBase                                base_node_type;
		typedef ListNode<T>                                 node_type;
		typedef Pointer                                     pointer;
		typedef Reference                                   reference;
		typedef eastl::bidirectional_iterator_tag    iterator_category;

	public:
		base_node_type* mpNode;

	public:
		ListIterator() EA_NOEXCEPT;
		ListIterator(const ListNodeBase* pNode) EA_NOEXCEPT;

		template <typename This = this_type, enable_if_t<!is_same_v<This, iterator>, bool> = true>
		inline ListIterator(const iterator& x) EA_NOEXCEPT
			: mpNode(x.mpNode)
		{
    __builtin_trap() /* STUB: not implemented */;
}

		this_type next() const EA_NOEXCEPT;
		this_type prev() const EA_NOEXCEPT;

		reference operator*() const EA_NOEXCEPT;
		pointer   operator->() const EA_NOEXCEPT;

		this_type& operator++() EA_NOEXCEPT;
		this_type  operator++(int) EA_NOEXCEPT;

		this_type& operator--() EA_NOEXCEPT;
		this_type  operator--(int) EA_NOEXCEPT;

	}; // ListIterator




	/// ListBase
	///
	/// See VectorBase (class vector) for an explanation of why we 
	/// create this separate base class.
	///
	template <typename T, typename Allocator>
	class ListBase
	{
	public:
		typedef T                                    value_type;
		typedef Allocator                            allocator_type;
		typedef ListNode<T>                          node_type;
		typedef eastl_size_t                         size_type;     // See config.h for the definition of eastl_size_t, which defaults to size_t.
		typedef ptrdiff_t                            difference_type;
		typedef ListNodeBase                         base_node_type; // We use ListNodeBase instead of ListNode<T> because we don't want to create a T.

	protected:
		eastl::compressed_pair<base_node_type, allocator_type>  mNodeAllocator;
		#if EASTL_LIST_SIZE_CACHE
			size_type  mSize;
		#endif

		base_node_type& internalNode() EA_NOEXCEPT {
    __builtin_trap() /* STUB: not implemented */;
}
		base_node_type const& internalNode() const EA_NOEXCEPT {
    __builtin_trap() /* STUB: not implemented */;
}
		allocator_type& internalAllocator() EA_NOEXCEPT {
    __builtin_trap() /* STUB: not implemented */;
}
		const allocator_type& internalAllocator() const EA_NOEXCEPT {
    __builtin_trap() /* STUB: not implemented */;
}

	public:
		const allocator_type& get_allocator() const EA_NOEXCEPT;
		allocator_type&       get_allocator() EA_NOEXCEPT;
		void                  set_allocator(const allocator_type& allocator);

	protected:
		ListBase();
		ListBase(const allocator_type& a);
	   ~ListBase();

		node_type* DoAllocateNode();
		void       DoFreeNode(node_type* pNode);

		void DoInit() EA_NOEXCEPT;
		void DoClear();

	}; // ListBase




	/// list
	///
	/// -- size() is O(n) --
	/// Note that as of this writing, list::size() is an O(n) operation when EASTL_LIST_SIZE_CACHE is disabled. 
	/// That is, getting the size of the list is not a fast operation, as it requires traversing the list and 
	/// counting the nodes. We could make list::size() be fast by having a member mSize variable. There are reasons 
	/// for having such functionality and reasons for not having such functionality. We currently choose
	/// to not have a member mSize variable as it would add four bytes to the class, add a tiny amount
	/// of processing to functions such as insert and erase, and would only serve to improve the size
	/// function, but no others. The alternative argument is that the C++ standard states that std::list
	/// should be an O(1) operation (i.e. have a member size variable), most C++ standard library list
	/// implementations do so, the size is but an integer which is quick to update, and many users 
	/// expect to have a fast size function. The EASTL_LIST_SIZE_CACHE option changes this.
	/// To consider: Make size caching an optional template parameter.
	///
	/// Pool allocation
	/// If you want to make a custom memory pool for a list container, your pool 
	/// needs to contain items of type list::node_type. So if you have a memory
	/// pool that has a constructor that takes the size of pool items and the
	/// count of pool items, you would do this (assuming that MemoryPool implements
	/// the Allocator interface):
	///     typedef list<Widget, MemoryPool> WidgetList;           // Delare your WidgetList type.
	///     MemoryPool myPool(sizeof(WidgetList::node_type), 100); // Make a pool of 100 Widget nodes.
	///     WidgetList myList(&myPool);                            // Create a list that uses the pool.
	///
	template <typename T, typename Allocator = EASTLAllocatorType>
	class list : public ListBase<T, Allocator>
	{
		typedef ListBase<T, Allocator>                  base_type;
		typedef list<T, Allocator>                      this_type;

	protected:
		using base_type::mNodeAllocator;
		using base_type::DoAllocateNode;
		using base_type::DoFreeNode;
		using base_type::DoClear;
		using base_type::DoInit;
#if EASTL_LIST_SIZE_CACHE
		using base_type::mSize;
#endif
		using base_type::internalNode;
		using base_type::internalAllocator;

	public:
		typedef T                                       value_type;
		typedef T*                                      pointer;
		typedef const T*                                const_pointer;
		typedef T&                                      reference;
		typedef const T&                                const_reference;
		typedef ListIterator<T, T*, T&>                 iterator;
		typedef ListIterator<T, const T*, const T&>     const_iterator;
		typedef eastl::reverse_iterator<iterator>       reverse_iterator;
		typedef eastl::reverse_iterator<const_iterator> const_reverse_iterator;
		typedef typename base_type::size_type           size_type;
		typedef typename base_type::difference_type     difference_type;
		typedef typename base_type::allocator_type      allocator_type;
		typedef typename base_type::node_type           node_type;
		typedef typename base_type::base_node_type      base_node_type;

		using base_type::get_allocator;

		static_assert(!is_const<value_type>::value, "vector<T> value_type must be non-const.");
		static_assert(!is_volatile<value_type>::value, "vector<T> value_type must be non-volatile.");

	public:
		list();
		list(const allocator_type& allocator);
		explicit list(size_type n, const allocator_type& allocator = EASTL_LIST_DEFAULT_ALLOCATOR);
		list(size_type n, const value_type& value, const allocator_type& allocator = EASTL_LIST_DEFAULT_ALLOCATOR);
		list(const this_type& x);
		list(const this_type& x, const allocator_type& allocator);
		list(this_type&& x);
		list(this_type&&, const allocator_type&);
		list(std::initializer_list<value_type> ilist, const allocator_type& allocator = EASTL_LIST_DEFAULT_ALLOCATOR);

		template <typename InputIterator>
		list(InputIterator first, InputIterator last); // allocator arg removed because VC7.1 fails on the default arg. To do: Make a second version of this function without a default arg.

		this_type& operator=(const this_type& x);
		this_type& operator=(std::initializer_list<value_type> ilist);
		this_type& operator=(this_type&& x);

		// In the case that the two containers' allocators are unequal, swap copies elements instead
		// of replacing them in place. In this case swap is an O(n) operation instead of O(1).
		void swap(this_type& x);

		void assign(size_type n, const value_type& value);

		template <typename InputIterator>                       // It turns out that the C++ std::list specifies a two argument
		void assign(InputIterator first, InputIterator last);   // version of assign that takes (int size, int value). These are not 
																// iterators, so we need to do a template compiler trick to do the right thing.
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

		void resize(size_type n, const value_type& value);
		void resize(size_type n);

		reference       front();
		const_reference front() const;

		reference       back();
		const_reference back() const;

		template <typename... Args>
		reference emplace_front(Args&&... args);

		template <typename... Args>
		reference emplace_back(Args&&... args);

		void      push_front(const value_type& value);
		void      push_front(value_type&& x);
		reference push_front();
		void*     push_front_uninitialized();

		void      push_back(const value_type& value);
		void      push_back(value_type&& x);
		reference push_back();
		void*     push_back_uninitialized();

		void pop_front();
		void pop_back();

		template <typename... Args>
		iterator emplace(const_iterator position, Args&&... args);

		iterator insert(const_iterator position);
		iterator insert(const_iterator position, const value_type& value);
		iterator insert(const_iterator position, value_type&& x);
		iterator insert(const_iterator position, std::initializer_list<value_type> ilist);
		iterator insert(const_iterator position, size_type n, const value_type& value);

		template <typename InputIterator>
		iterator insert(const_iterator position, InputIterator first, InputIterator last);

		iterator erase(const_iterator position);
		iterator erase(const_iterator first, const_iterator last);

		reverse_iterator erase(const_reverse_iterator position);
		reverse_iterator erase(const_reverse_iterator first, const_reverse_iterator last);

		void clear() EA_NOEXCEPT;
		void reset_lose_memory() EA_NOEXCEPT;    // This is a unilateral reset to an initially empty state. No destructors are called, no deallocation occurs.

		size_type remove(const T& x);

		template <typename Predicate>
		size_type remove_if(Predicate);

		void reverse() EA_NOEXCEPT;

		// splice inserts elements in the range [first,last) before position and removes the elements from x.
		// In the case that the two containers' allocators are unequal, splice copies elements 
		// instead of splicing them. In this case elements are not removed from x, and iterators 
		// into the spliced elements from x continue to point to the original values in x.
		void splice(const_iterator position, this_type& x);
		void splice(const_iterator position, this_type& x, const_iterator i);
		void splice(const_iterator position, this_type& x, const_iterator first, const_iterator last);
		void splice(const_iterator position, this_type&& x);
		void splice(const_iterator position, this_type&& x, const_iterator i);
		void splice(const_iterator position, this_type&& x, const_iterator first, const_iterator last);

	public:
		// For merge, see notes for splice regarding the handling of unequal allocators.
		void merge(this_type& x);
		void merge(this_type&& x);

		template <typename Compare>
		void merge(this_type& x, Compare compare);

		template <typename Compare>
		void merge(this_type&& x, Compare compare);

		size_type unique();

		template <typename BinaryPredicate>
		size_type unique(BinaryPredicate);

		// Sorting functionality
		// This is independent of the global sort algorithms, as lists are 
		// linked nodes and can be sorted more efficiently by moving nodes
		// around in ways that global sort algorithms aren't privy to.
		void sort();

		template<typename Compare>
		void sort(Compare compare);

	public:
		bool validate() const;
		int  validate_iterator(const_iterator i) const;

	protected:
		node_type* DoCreateNode();

		template<typename... Args>
		node_type* DoCreateNode(Args&&... args);

		template <typename Integer>
		void DoAssign(Integer n, Integer value, true_type);

		template <typename InputIterator>
		void DoAssign(InputIterator first, InputIterator last, false_type);

		void DoAssignValues(size_type n, const value_type& value);

		template <typename Integer>
		void DoInsert(ListNodeBase* pNode, Integer n, Integer value, true_type);

		template <typename InputIterator>
		void DoInsert(ListNodeBase* pNode, InputIterator first, InputIterator last, false_type);

		void DoInsertValues(ListNodeBase* pNode, size_type n, const value_type& value);
	   
		template<typename... Args>
		void DoInsertValue(ListNodeBase* pNode, Args&&... args);

		void DoErase(ListNodeBase* pNode);

		void DoSwap(this_type& x);

		template <typename Compare>
		iterator DoSort(iterator i1, iterator end2, size_type n, Compare& compare);

	}; // class list





	///////////////////////////////////////////////////////////////////////
	// ListNodeBase
	///////////////////////////////////////////////////////////////////////

	// Swaps the nodes a and b in the lists to which they belong. This is similar to 
	// splicing a into b's list and b into a's list at the same time.
	// Works by swapping the members of a and b, and fixes up the lists that a and b 
	// were part of to point to the new members.
	inline void ListNodeBase::swap(ListNodeBase& a, ListNodeBase& b) EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	// splices the [first,last) range from its current list into our list before this node.
	inline void ListNodeBase::splice(ListNodeBase* first, ListNodeBase* last) EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	inline void ListNodeBase::reverse() EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	inline void ListNodeBase::insert(ListNodeBase* pNext) EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	// Removes this node from the list that it's in. Assumes that the 
	// node is within a list and thus that its prev/next pointers are valid.
	inline void ListNodeBase::remove() EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	// Inserts the standalone range [pFirst, pFinal] before pPosition. Assumes that the
	// range is not within a list and thus that it's prev/next pointers are not valid.
	// Assumes that this node is within a list and thus that its prev/next pointers are valid.
	inline void ListNodeBase::insert_range(ListNodeBase* pFirst, ListNodeBase* pFinal) EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	// Removes the range [pFirst, pFinal] from the list that it's in. Assumes that the 
	// range is within a list and thus that its prev/next pointers are valid.
	inline void ListNodeBase::remove_range(ListNodeBase* pFirst, ListNodeBase* pFinal) EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	///////////////////////////////////////////////////////////////////////
	// ListIterator
	///////////////////////////////////////////////////////////////////////

	template <typename T, typename Pointer, typename Reference>
	inline ListIterator<T, Pointer, Reference>::ListIterator() EA_NOEXCEPT
		: mpNode() // To consider: Do we really need to intialize mpNode?
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Pointer, typename Reference>
	inline ListIterator<T, Pointer, Reference>::ListIterator(const ListNodeBase* pNode) EA_NOEXCEPT
		: mpNode(const_cast<base_node_type*>(pNode))
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Pointer, typename Reference>
	inline typename ListIterator<T, Pointer, Reference>::this_type
	ListIterator<T, Pointer, Reference>::next() const EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Pointer, typename Reference>
	inline typename ListIterator<T, Pointer, Reference>::this_type
	ListIterator<T, Pointer, Reference>::prev() const EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Pointer, typename Reference>
	inline typename ListIterator<T, Pointer, Reference>::reference
	ListIterator<T, Pointer, Reference>::operator*() const EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Pointer, typename Reference>
	inline typename ListIterator<T, Pointer, Reference>::pointer
	ListIterator<T, Pointer, Reference>::operator->() const EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Pointer, typename Reference>
	inline typename ListIterator<T, Pointer, Reference>::this_type&
	ListIterator<T, Pointer, Reference>::operator++() EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Pointer, typename Reference>
	inline typename ListIterator<T, Pointer, Reference>::this_type
	ListIterator<T, Pointer, Reference>::operator++(int) EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Pointer, typename Reference>
	inline typename ListIterator<T, Pointer, Reference>::this_type&
	ListIterator<T, Pointer, Reference>::operator--() EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Pointer, typename Reference>
	inline typename ListIterator<T, Pointer, Reference>::this_type 
	ListIterator<T, Pointer, Reference>::operator--(int) EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	// The C++ defect report #179 requires that we support comparisons between const and non-const iterators.
	// Thus we provide additional template paremeters here to support this. The defect report does not
	// require us to support comparisons between reverse_iterators and const_reverse_iterators.
	template <typename T, typename PointerA, typename ReferenceA, typename PointerB, typename ReferenceB>
	inline bool operator==(const ListIterator<T, PointerA, ReferenceA>& a, 
						   const ListIterator<T, PointerB, ReferenceB>& b) EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename PointerA, typename ReferenceA, typename PointerB, typename ReferenceB>
	inline bool operator!=(const ListIterator<T, PointerA, ReferenceA>& a, 
						   const ListIterator<T, PointerB, ReferenceB>& b) EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	// We provide a version of operator!= for the case where the iterators are of the 
	// same type. This helps prevent ambiguity errors in the presence of rel_ops.
	template <typename T, typename Pointer, typename Reference>
	inline bool operator!=(const ListIterator<T, Pointer, Reference>& a, 
						   const ListIterator<T, Pointer, Reference>& b) EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}



	///////////////////////////////////////////////////////////////////////
	// ListBase
	///////////////////////////////////////////////////////////////////////

	template <typename T, typename Allocator>
	inline ListBase<T, Allocator>::ListBase()
		: mNodeAllocator(base_node_type(), allocator_type(EASTL_LIST_DEFAULT_NAME))
		  #if EASTL_LIST_SIZE_CACHE
		  , mSize(0)
		  #endif
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T, typename Allocator>
	inline ListBase<T, Allocator>::ListBase(const allocator_type& allocator)
		: mNodeAllocator(base_node_type(), allocator)
		  #if EASTL_LIST_SIZE_CACHE
		  , mSize(0)
		  #endif
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline ListBase<T, Allocator>::~ListBase()
	{
		DoClear();
	}


	template <typename T, typename Allocator>
	const typename ListBase<T, Allocator>::allocator_type&
	ListBase<T, Allocator>::get_allocator() const EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	typename ListBase<T, Allocator>::allocator_type&
	ListBase<T, Allocator>::get_allocator() EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline void ListBase<T, Allocator>::set_allocator(const allocator_type& allocator)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename ListBase<T, Allocator>::node_type*
	ListBase<T, Allocator>::DoAllocateNode()
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline void ListBase<T, Allocator>::DoFreeNode(node_type* p)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline void ListBase<T, Allocator>::DoInit() EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline void ListBase<T, Allocator>::DoClear()
	{
    __builtin_trap() /* STUB: not implemented */;
}



	///////////////////////////////////////////////////////////////////////
	// list
	///////////////////////////////////////////////////////////////////////

	template <typename T, typename Allocator>
	inline list<T, Allocator>::list()
		: base_type()
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline list<T, Allocator>::list(const allocator_type& allocator)
		: base_type(allocator)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline list<T, Allocator>::list(size_type n, const allocator_type& allocator)
		: base_type(allocator)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline list<T, Allocator>::list(size_type n, const value_type& value, const allocator_type& allocator)
		: base_type(allocator) 
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline list<T, Allocator>::list(const this_type& x)
		: base_type(x.internalAllocator())
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline list<T, Allocator>::list(const this_type& x, const allocator_type& allocator)
		: base_type(allocator)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline list<T, Allocator>::list(this_type&& x)
		: base_type(eastl::move(x.internalAllocator()))
	{
		swap(x);
	}


	template <typename T, typename Allocator>
	inline list<T, Allocator>::list(this_type&& x, const allocator_type& allocator)
		: base_type(allocator)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline list<T, Allocator>::list(std::initializer_list<value_type> ilist, const allocator_type& allocator)
		: base_type(allocator)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	template <typename InputIterator>
	list<T, Allocator>::list(InputIterator first, InputIterator last)
		: base_type(EASTL_LIST_DEFAULT_ALLOCATOR)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	typename list<T, Allocator>::iterator
	inline list<T, Allocator>::begin() EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename list<T, Allocator>::const_iterator
	list<T, Allocator>::begin() const EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename list<T, Allocator>::const_iterator
	list<T, Allocator>::cbegin() const EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename list<T, Allocator>::iterator
	list<T, Allocator>::end() EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename list<T, Allocator>::const_iterator
	list<T, Allocator>::end() const EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename list<T, Allocator>::const_iterator
	list<T, Allocator>::cend() const EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename list<T, Allocator>::reverse_iterator
	list<T, Allocator>::rbegin() EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename list<T, Allocator>::const_reverse_iterator
	list<T, Allocator>::rbegin() const EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename list<T, Allocator>::const_reverse_iterator
	list<T, Allocator>::crbegin() const EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename list<T, Allocator>::reverse_iterator
	list<T, Allocator>::rend() EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename list<T, Allocator>::const_reverse_iterator
	list<T, Allocator>::rend() const EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename list<T, Allocator>::const_reverse_iterator
	list<T, Allocator>::crend() const EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename list<T, Allocator>::reference
	list<T, Allocator>::front()
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename list<T, Allocator>::const_reference
	list<T, Allocator>::front() const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename list<T, Allocator>::reference
	list<T, Allocator>::back()
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename list<T, Allocator>::const_reference
	list<T, Allocator>::back() const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline bool list<T, Allocator>::empty() const EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename list<T, Allocator>::size_type
	list<T, Allocator>::size() const EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	typename list<T, Allocator>::this_type&
	list<T, Allocator>::operator=(const this_type& x)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	typename list<T, Allocator>::this_type&
	list<T, Allocator>::operator=(this_type&& x)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	typename list<T, Allocator>::this_type&
	list<T, Allocator>::operator=(std::initializer_list<value_type> ilist)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline void list<T, Allocator>::assign(size_type n, const value_type& value)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	// It turns out that the C++ std::list specifies a two argument
	// version of assign that takes (int size, int value). These are not 
	// iterators, so we need to do a template compiler trick to do the right thing.
	template <typename T, typename Allocator>
	template <typename InputIterator>
	inline void list<T, Allocator>::assign(InputIterator first, InputIterator last)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline void list<T, Allocator>::assign(std::initializer_list<value_type> ilist)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline void list<T, Allocator>::clear() EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline void list<T, Allocator>::reset_lose_memory() EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	void list<T, Allocator>::resize(size_type n, const value_type& value)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline void list<T, Allocator>::resize(size_type n)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	template <typename... Args>
	typename list<T, Allocator>::reference list<T, Allocator>::emplace_front(Args&&... args)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T, typename Allocator>
	template <typename... Args>
	typename list<T, Allocator>::reference list<T, Allocator>::emplace_back(Args&&... args)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline void list<T, Allocator>::push_front(const value_type& value)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline void list<T, Allocator>::push_front(value_type&& value)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename list<T, Allocator>::reference
	list<T, Allocator>::push_front()
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline void* list<T, Allocator>::push_front_uninitialized()
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline void list<T, Allocator>::pop_front()
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline void list<T, Allocator>::push_back(const value_type& value)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline void list<T, Allocator>::push_back(value_type&& value)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename list<T, Allocator>::reference
	list<T, Allocator>::push_back()
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline void* list<T, Allocator>::push_back_uninitialized()
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline void list<T, Allocator>::pop_back()
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	template <typename... Args>
	inline typename list<T, Allocator>::iterator
	list<T, Allocator>::emplace(const_iterator position, Args&&... args)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename list<T, Allocator>::iterator
	list<T, Allocator>::insert(const_iterator position)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	
	template <typename T, typename Allocator>
	inline typename list<T, Allocator>::iterator
	list<T, Allocator>::insert(const_iterator position, const value_type& value)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename list<T, Allocator>::iterator
	list<T, Allocator>::insert(const_iterator position, value_type&& value)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T, typename Allocator>
	inline typename list<T, Allocator>::iterator
	list<T, Allocator>::insert(const_iterator position, size_type n, const value_type& value)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	template <typename InputIterator>
	inline typename list<T, Allocator>::iterator
	list<T, Allocator>::insert(const_iterator position, InputIterator first, InputIterator last)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename list<T, Allocator>::iterator 
	list<T, Allocator>::insert(const_iterator position, std::initializer_list<value_type> ilist)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename list<T, Allocator>::iterator
	list<T, Allocator>::erase(const_iterator position)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	typename list<T, Allocator>::iterator
	list<T, Allocator>::erase(const_iterator first, const_iterator last)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename list<T, Allocator>::reverse_iterator
	list<T, Allocator>::erase(const_reverse_iterator position)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	typename list<T, Allocator>::reverse_iterator
	list<T, Allocator>::erase(const_reverse_iterator first, const_reverse_iterator last)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	typename list<T, Allocator>::size_type list<T, Allocator>::remove(const value_type& value)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	template <typename Predicate>
	inline typename list<T, Allocator>::size_type list<T, Allocator>::remove_if(Predicate predicate)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline void list<T, Allocator>::reverse() EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline void list<T, Allocator>::splice(const_iterator position, this_type& x)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T, typename Allocator>
	inline void list<T, Allocator>::splice(const_iterator position, this_type&& x)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline void list<T, Allocator>::splice(const_iterator position, list& x, const_iterator i)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline void list<T, Allocator>::splice(const_iterator position, list<T,Allocator>&& x, const_iterator i)
	{
    __builtin_trap() /* STUB: not implemented */;
}
		

	template <typename T, typename Allocator>
	inline void list<T, Allocator>::splice(const_iterator position, this_type& x, const_iterator first, const_iterator last)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline void list<T, Allocator>::splice(const_iterator position, list<T,Allocator>&& x, const_iterator first, const_iterator last)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	// does not propagate allocators on swap.
	// in addition, requires T be copy constructible and copy assignable, which isn't required by the standard.
	template <typename T, typename Allocator>
	inline void list<T, Allocator>::swap(this_type& x)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	void list<T, Allocator>::merge(this_type& x)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	void list<T, Allocator>::merge(this_type&& x)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	template <typename Compare>
	void list<T, Allocator>::merge(this_type& x, Compare compare)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	template <typename Compare>
	void list<T, Allocator>::merge(this_type&& x, Compare compare)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	typename list<T, Allocator>::size_type list<T, Allocator>::unique()
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	template <typename BinaryPredicate>
	typename list<T, Allocator>::size_type list<T, Allocator>::unique(BinaryPredicate predicate)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	void list<T, Allocator>::sort()
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	template <typename Compare>
	void list<T, Allocator>::sort(Compare compare)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	template <typename Compare>
	typename list<T, Allocator>::iterator
	list<T, Allocator>::DoSort(iterator i1, iterator end2, size_type n, Compare& compare)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	template<typename... Args>
	inline typename list<T, Allocator>::node_type*
	list<T, Allocator>::DoCreateNode(Args&&... args)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename list<T, Allocator>::node_type*
	list<T, Allocator>::DoCreateNode()
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	template <typename Integer>
	inline void list<T, Allocator>::DoAssign(Integer n, Integer value, true_type)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	template <typename InputIterator>
	void list<T, Allocator>::DoAssign(InputIterator first, InputIterator last, false_type)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	void list<T, Allocator>::DoAssignValues(size_type n, const value_type& value)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	template <typename Integer>
	inline void list<T, Allocator>::DoInsert(ListNodeBase* pNode, Integer n, Integer value, true_type)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	template <typename InputIterator>
	inline void list<T, Allocator>::DoInsert(ListNodeBase* pNode, InputIterator first, InputIterator last, false_type)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline void list<T, Allocator>::DoInsertValues(ListNodeBase* pNode, size_type n, const value_type& value)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	template<typename... Args>
	inline void list<T, Allocator>::DoInsertValue(ListNodeBase* pNode, Args&&... args)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline void list<T, Allocator>::DoErase(ListNodeBase* pNode)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline void list<T, Allocator>::DoSwap(this_type& x)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline bool list<T, Allocator>::validate() const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline int list<T, Allocator>::validate_iterator(const_iterator i) const
	{
    __builtin_trap() /* STUB: not implemented */;
}



	///////////////////////////////////////////////////////////////////////
	// global operators
	///////////////////////////////////////////////////////////////////////

	template <typename T, typename Allocator>
	bool operator==(const list<T, Allocator>& a, const list<T, Allocator>& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}

#if defined(EA_COMPILER_HAS_THREE_WAY_COMPARISON)
	template <typename T, typename Allocator>
	inline synth_three_way_result<T> operator<=>(const list<T, Allocator>& a, const list<T, Allocator>& b)
	{
		return eastl::lexicographical_compare_three_way(a.begin(), a.end(), b.begin(), b.end(), synth_three_way{});
	}
#else
	template <typename T, typename Allocator>
	bool operator<(const list<T, Allocator>& a, const list<T, Allocator>& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T, typename Allocator>
	bool operator!=(const list<T, Allocator>& a, const list<T, Allocator>& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T, typename Allocator>
	bool operator>(const list<T, Allocator>& a, const list<T, Allocator>& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T, typename Allocator>
	bool operator<=(const list<T, Allocator>& a, const list<T, Allocator>& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T, typename Allocator>
	bool operator>=(const list<T, Allocator>& a, const list<T, Allocator>& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}
#endif
	template <typename T, typename Allocator>
	void swap(list<T, Allocator>& a, list<T, Allocator>& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	///////////////////////////////////////////////////////////////////////
	// erase / erase_if
	//
	// https://en.cppreference.com/w/cpp/container/list/erase2
	///////////////////////////////////////////////////////////////////////
	template <class T, class Allocator, class U>
	typename list<T, Allocator>::size_type erase(list<T, Allocator>& c, const U& value)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <class T, class Allocator, class Predicate>
	typename list<T, Allocator>::size_type erase_if(list<T, Allocator>& c, Predicate predicate)
	{
    __builtin_trap() /* STUB: not implemented */;
}


} // namespace eastl


EA_RESTORE_SN_WARNING()

EA_RESTORE_VC_WARNING();


#endif // Header include guard
