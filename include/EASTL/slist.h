#include <cstdlib>
///////////////////////////////////////////////////////////////////////////////
// Copyright (c) Electronic Arts Inc. All rights reserved.
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// An slist is a singly-linked list. The C++ standard library doesn't define
// such a thing as an slist, nor does the C++ TR1. Our implementation of slist
// largely follows the design of the SGI STL slist container, which is also 
// found in STLPort. Singly-linked lists use less memory than doubly-linked 
// lists, but are less flexible. 
//
// In looking at slist, you will notice a lot of references to things like
// 'before first', 'before last', 'insert after', and 'erase after'. This is 
// due to the fact that std::list insert and erase works on the node before
// the referenced node, whereas slist is singly linked and operations are only
// efficient if they work on the node after the referenced node. This is because
// with an slist node you know the node after it but not the node before it.
//
///////////////////////////////////////////////////////////////////////////////



#ifndef EASTL_SLIST_H
#define EASTL_SLIST_H


#include <EASTL/internal/config.h>
#include <EASTL/allocator.h>
#include <EASTL/type_traits.h>
#include <EASTL/iterator.h>
#include <EASTL/algorithm.h>
#include <EASTL/initializer_list.h>
#include <EASTL/memory.h>
#include <EASTL/sort.h>
#include <EASTL/bonus/compressed_pair.h>
#include <stddef.h>
#if EASTL_EXCEPTIONS_ENABLED
#include <stdexcept>
#endif

EA_DISABLE_ALL_VC_WARNINGS();

	#include <new>

EA_RESTORE_ALL_VC_WARNINGS();

EA_DISABLE_SN_WARNING(828); // The EDG SN compiler has a bug in its handling of variadic template arguments and mistakenly reports "parameter "args" was never referenced"


// 4530 - C++ exception handler used, but unwind semantics are not enabled. Specify /EHsc
// 4345 - Behavior change: an object of POD type constructed with an initializer of the form () will be default-initialized
// 4571 - catch(...) semantics changed since Visual C++ 7.1; structured exceptions (SEH) are no longer caught.
EA_DISABLE_VC_WARNING(4530 4345 4571);


#if defined(EA_PRAGMA_ONCE_SUPPORTED)
	#pragma once // Some compilers (e.g. VC++) benefit significantly from using this. We've measured 3-4% build speed improvements in apps as a result.
#endif



namespace eastl
{

	/// EASTL_SLIST_DEFAULT_NAME
	///
	/// Defines a default container name in the absence of a user-provided name.
	///
	#ifndef EASTL_SLIST_DEFAULT_NAME
		#define EASTL_SLIST_DEFAULT_NAME EASTL_DEFAULT_NAME_PREFIX " slist" // Unless the user overrides something, this is "EASTL slist".
	#endif


	/// EASTL_SLIST_DEFAULT_ALLOCATOR
	///
	#ifndef EASTL_SLIST_DEFAULT_ALLOCATOR
		#define EASTL_SLIST_DEFAULT_ALLOCATOR allocator_type(EASTL_SLIST_DEFAULT_NAME)
	#endif



	/// SListNodeBase
	///
	/// This is a standalone struct so that operations on it can be done without templates
	/// and so that an empty slist can have an SListNodeBase and thus not create any 
	/// instances of T.
	///
	struct SListNodeBase
	{
		SListNodeBase* mpNext;
	};


	template <typename T>
	struct SListNode : public SListNodeBase
	{
		T mValue;
	};

	/// SListIterator
	///
	template <typename T, typename Pointer, typename Reference>
	struct SListIterator
	{
		typedef SListIterator<T, Pointer, Reference>   this_type;
		typedef SListIterator<T, T*, T&>               iterator;
		typedef SListIterator<T, const T*, const T&>   const_iterator;
		typedef eastl_size_t                           size_type;     // See config.h for the definition of eastl_size_t, which defaults to size_t.
		typedef ptrdiff_t                              difference_type;
		typedef T                                      value_type;
		typedef SListNodeBase                          base_node_type;
		typedef SListNode<T>                           node_type;
		typedef Pointer                                pointer;
		typedef Reference                              reference;
		typedef eastl::forward_iterator_tag     iterator_category;

	public:
		base_node_type* mpNode;

	public:
		SListIterator();
		SListIterator(const SListNodeBase* pNode);
		
		template <typename This = this_type, enable_if_t<!is_same_v<This, iterator>, bool> = true>
		inline SListIterator(const iterator& x)
			: mpNode(x.mpNode)
		{
    __builtin_trap() /* STUB: not implemented */;
}

		reference operator*() const;
		pointer   operator->() const;

		this_type& operator++();
		this_type  operator++(int);
	};



	/// SListBase
	///
	/// See VectorBase (class vector) for an explanation of why we 
	/// create this separate base class.
	///
	template <typename T, typename Allocator>
	struct SListBase
	{
	public:
		typedef Allocator                              allocator_type;
		typedef SListNode<T>                           node_type;
		typedef eastl_size_t                           size_type;     // See config.h for the definition of eastl_size_t, which defaults to size_t.
		typedef ptrdiff_t                              difference_type;
		typedef SListNodeBase                      base_node_type; // We use SListNodeBase instead of SListNode<T> because we don't want to create a T.

	protected:
		eastl::compressed_pair<base_node_type, allocator_type>  mNodeAllocator;
		#if EASTL_SLIST_SIZE_CACHE
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
		SListBase();
		SListBase(const allocator_type& a);
	   ~SListBase();

		node_type* DoAllocateNode();
		void       DoFreeNode(node_type* pNode);

		SListNodeBase* DoEraseAfter(SListNodeBase* pNode);
		SListNodeBase* DoEraseAfter(SListNodeBase* pNode, SListNodeBase* pNodeLast);

	}; // class SListBase



	/// slist
	///
	/// This is the equivalent of C++11's forward_list.
	///
	/// -- size() is O(n) --
	/// Note that as of this writing, list::size() is an O(n) operation when EASTL_SLIST_SIZE_CACHE is disabled. 
	/// That is, getting the size of the list is not a fast operation, as it requires traversing the list and 
	/// counting the nodes. We could make list::size() be fast by having a member mSize variable. There are reasons 
	/// for having such functionality and reasons for not having such functionality. We currently choose
	/// to not have a member mSize variable as it would add four bytes to the class, add a tiny amount
	/// of processing to functions such as insert and erase, and would only serve to improve the size
	/// function, but no others. The alternative argument is that the C++ standard states that std::list
	/// should be an O(1) operation (i.e. have a member size variable), most C++ standard library list
	/// implementations do so, the size is but an integer which is quick to update, and many users 
	/// expect to have a fast size function. The EASTL_SLIST_SIZE_CACHE option changes this.
	/// To consider: Make size caching an optional template parameter.
	///
	/// Pool allocation
	/// If you want to make a custom memory pool for a list container, your pool 
	/// needs to contain items of type slist::node_type. So if you have a memory
	/// pool that has a constructor that takes the size of pool items and the
	/// count of pool items, you would do this (assuming that MemoryPool implements
	/// the Allocator interface):
	///     typedef slist<Widget, MemoryPool> WidgetList;          // Delare your WidgetList type.
	///     MemoryPool myPool(sizeof(WidgetList::node_type), 100); // Make a pool of 100 Widget nodes.
	///     WidgetList myList(&myPool);                            // Create a list that uses the pool.
	///
	template <typename T, typename Allocator = EASTLAllocatorType >
	class slist : public SListBase<T, Allocator>
	{
		typedef SListBase<T, Allocator>              base_type;
		typedef slist<T, Allocator>                  this_type;

	protected:
		using base_type::mNodeAllocator;
		using base_type::DoEraseAfter;
		using base_type::DoAllocateNode;
		using base_type::DoFreeNode;
#if EASTL_SLIST_SIZE_CACHE
		using base_type::mSize;
#endif
		using base_type::internalNode;
		using base_type::internalAllocator;

	public:
		typedef T                                    value_type;
		typedef value_type*                          pointer;
		typedef const value_type*                    const_pointer;
		typedef value_type&                          reference;
		typedef const value_type&                    const_reference;
		typedef SListIterator<T, T*, T&>             iterator;
		typedef SListIterator<T, const T*, const T&> const_iterator;
		typedef typename base_type::size_type        size_type;
		typedef typename base_type::difference_type  difference_type;
		typedef typename base_type::allocator_type   allocator_type;
		typedef typename base_type::node_type        node_type;
		typedef typename base_type::base_node_type   base_node_type;

		static_assert(!is_const<value_type>::value, "slist<T> value_type must be non-const.");
		static_assert(!is_volatile<value_type>::value, "slist<T> value_type must be non-volatile.");

	public:
		slist();
		slist(const allocator_type& allocator);
		explicit slist(size_type n, const allocator_type& allocator = EASTL_SLIST_DEFAULT_ALLOCATOR);
		slist(size_type n, const value_type& value, const allocator_type& allocator = EASTL_SLIST_DEFAULT_ALLOCATOR);
		slist(const this_type& x);
		slist(std::initializer_list<value_type> ilist, const allocator_type& allocator = EASTL_SLIST_DEFAULT_ALLOCATOR);
		slist(this_type&& x);
		slist(this_type&& x, const allocator_type& allocator);

		template <typename InputIterator>
		slist(InputIterator first, InputIterator last); // allocator arg removed because VC7.1 fails on the default arg. To do: Make a second version of this function without a default arg.

		this_type& operator=(const this_type& x);
		this_type& operator=(std::initializer_list<value_type>);
		this_type& operator=(this_type&& x);

		void swap(this_type& x);

		void assign(size_type n, const value_type& value);
		void assign(std::initializer_list<value_type> ilist);

		template <typename InputIterator>
		void assign(InputIterator first, InputIterator last);

		iterator       begin() EA_NOEXCEPT;
		const_iterator begin() const EA_NOEXCEPT;
		const_iterator cbegin() const EA_NOEXCEPT;

		iterator       end() EA_NOEXCEPT;
		const_iterator end() const EA_NOEXCEPT;
		const_iterator cend() const EA_NOEXCEPT;

		iterator       before_begin() EA_NOEXCEPT;
		const_iterator before_begin() const EA_NOEXCEPT;
		const_iterator cbefore_begin() const EA_NOEXCEPT;

		iterator        previous(const_iterator position);
		const_iterator  previous(const_iterator position) const;

		reference       front();
		const_reference front() const;

		template <class... Args>
		reference emplace_front(Args&&... args);

		void      push_front(const value_type& value);
		reference push_front();
		void      push_front(value_type&& value);

		void      pop_front();

		bool      empty() const EA_NOEXCEPT;
		size_type size() const EA_NOEXCEPT;

		void resize(size_type n, const value_type& value);
		void resize(size_type n);

		iterator insert(const_iterator position);
		iterator insert(const_iterator position, const value_type& value);
		void     insert(const_iterator position, size_type n, const value_type& value);

		template <typename InputIterator>
		void insert(const_iterator position, InputIterator first, InputIterator last);

		// Returns an iterator pointing to the last inserted element, or position if insertion count is zero.
		iterator insert_after(const_iterator position);
		iterator insert_after(const_iterator position, const value_type& value);
		iterator insert_after(const_iterator position, size_type n, const value_type& value);
		iterator insert_after(const_iterator position, std::initializer_list<value_type> ilist);
		iterator insert_after(const_iterator position, value_type&& value);

		template <class... Args>
		iterator emplace_after(const_iterator position, Args&&... args);

		template <typename InputIterator>
		iterator insert_after(const_iterator position, InputIterator first, InputIterator last);

		iterator erase(const_iterator position);
		iterator erase(const_iterator first, const_iterator last);

		iterator erase_after(const_iterator position);
		iterator erase_after(const_iterator before_first, const_iterator last);

		void clear() EA_NOEXCEPT;
		void reset_lose_memory() EA_NOEXCEPT;    // This is a unilateral reset to an initially empty state. No destructors are called, no deallocation occurs.

		size_type remove(const value_type& value);

		template <typename Predicate>
		size_type remove_if(Predicate predicate);

		void reverse() EA_NOEXCEPT;

		// splice splices to before position, like with the list container. However, in order to do so 
		// it must walk the list from beginning to position, which is an O(n) operation that can thus 
		// be slow. It's recommended that the splice_after functions be used whenever possible as they are O(1).
		void splice(const_iterator position, this_type& x);
		void splice(const_iterator position, this_type& x, const_iterator i);
		void splice(const_iterator position, this_type& x, const_iterator first, const_iterator last);
		void splice(const_iterator position, this_type&& x);
		void splice(const_iterator position, this_type&& x, const_iterator i);
		void splice(const_iterator position, this_type&& x, const_iterator first, const_iterator last);

		void splice_after(const_iterator position, this_type& x);
		void splice_after(const_iterator position, this_type& x, const_iterator i);
		void splice_after(const_iterator position, this_type& x, const_iterator first, const_iterator last);
		void splice_after(const_iterator position, this_type&& x);
		void splice_after(const_iterator position, this_type&& x, const_iterator i);
		void splice_after(const_iterator position, this_type&& x, const_iterator first, const_iterator last);

		size_type unique();

		template <typename BinaryPredicate>
		size_type unique(BinaryPredicate);

		// Sorting functionality
		// This is independent of the global sort algorithms, as lists are 
		// linked nodes and can be sorted more efficiently by moving nodes
		// around in ways that global sort algorithms aren't privy to.
		void sort();

		template <class Compare>
		void sort(Compare compare);

		// Not yet implemented:
		// void merge(this_type& x);
		// void merge(this_type&& x);
		// template <class Compare>
		// void merge(this_type& x, Compare compare);
		// template <class Compare>
		// void merge(this_type&& x, Compare compare);
		// If these get implemented then make sure to override them in fixed_slist.

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

		template <typename InputIterator>
		node_type* DoInsertAfter(SListNodeBase* pNode, InputIterator first, InputIterator last);

		template <typename Integer>
		node_type* DoInsertAfter(SListNodeBase* pNode, Integer n, Integer value, true_type);

		template <typename InputIterator>
		node_type* DoInsertAfter(SListNodeBase* pNode, InputIterator first, InputIterator last, false_type);

		node_type* DoInsertValueAfter(SListNodeBase* pNode);
		node_type* DoInsertValuesAfter(SListNodeBase* pNode, size_type n, const value_type& value);

		template<typename... Args>
		node_type* DoInsertValueAfter(SListNodeBase* pNode, Args&&... args);

		void DoSwap(this_type& x);

	}; // class slist







	///////////////////////////////////////////////////////////////////////
	// SListNodeBase functions
	///////////////////////////////////////////////////////////////////////

	inline SListNodeBase* SListNodeInsertAfter(SListNodeBase* pPrevNode, SListNodeBase* pNode)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	inline SListNodeBase* SListNodeGetPrevious(SListNodeBase* pNodeBase, const SListNodeBase* pNode)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	inline const SListNodeBase* SListNodeGetPrevious(const SListNodeBase* pNodeBase, const SListNodeBase* pNode)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	inline void SListNodeSpliceAfter(SListNodeBase* pNode, SListNodeBase* pNodeBeforeFirst, SListNodeBase* pNodeBeforeLast)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	inline void SListNodeSpliceAfter(SListNodeBase* pNode, SListNodeBase* pNodeBase)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	inline SListNodeBase* SListNodeReverse(SListNodeBase* pNode)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	inline uint32_t SListNodeGetSize(SListNodeBase* pNode)
	{
    __builtin_trap() /* STUB: not implemented */;
}




	///////////////////////////////////////////////////////////////////////
	// SListIterator functions
	///////////////////////////////////////////////////////////////////////

	template <typename T, typename Pointer, typename Reference>
	inline SListIterator<T, Pointer, Reference>::SListIterator()
		: mpNode(NULL)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Pointer, typename Reference>
	inline SListIterator<T, Pointer, Reference>::SListIterator(const SListNodeBase* pNode)
		: mpNode(const_cast<base_node_type*>(pNode))
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Pointer, typename Reference>
	inline typename SListIterator<T, Pointer, Reference>::reference
	SListIterator<T, Pointer, Reference>::operator*() const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Pointer, typename Reference>
	inline typename SListIterator<T, Pointer, Reference>::pointer
	SListIterator<T, Pointer, Reference>::operator->() const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Pointer, typename Reference>
	inline typename SListIterator<T, Pointer, Reference>::this_type&
	SListIterator<T, Pointer, Reference>::operator++()
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Pointer, typename Reference>
	inline typename SListIterator<T, Pointer, Reference>::this_type
	SListIterator<T, Pointer, Reference>::operator++(int)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	// The C++ defect report #179 requires that we support comparisons between const and non-const iterators.
	// Thus we provide additional template paremeters here to support this. The defect report does not
	// require us to support comparisons between reverse_iterators and const_reverse_iterators.
	template <typename T, typename PointerA, typename ReferenceA, typename PointerB, typename ReferenceB>
	inline bool operator==(const SListIterator<T, PointerA, ReferenceA>& a, 
						   const SListIterator<T, PointerB, ReferenceB>& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename PointerA, typename ReferenceA, typename PointerB, typename ReferenceB>
	inline bool operator!=(const SListIterator<T, PointerA, ReferenceA>& a, 
						   const SListIterator<T, PointerB, ReferenceB>& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	// We provide a version of operator!= for the case where the iterators are of the 
	// same type. This helps prevent ambiguity errors in the presence of rel_ops.
	template <typename T, typename Pointer, typename Reference>
	inline bool operator!=(const SListIterator<T, Pointer, Reference>& a, 
						   const SListIterator<T, Pointer, Reference>& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}




	
	///////////////////////////////////////////////////////////////////////
	// SListBase functions
	///////////////////////////////////////////////////////////////////////

	template <typename T, typename Allocator>
	inline SListBase<T, Allocator>::SListBase()
		: mNodeAllocator(base_node_type(), allocator_type(EASTL_SLIST_DEFAULT_NAME))
		  #if EASTL_SLIST_SIZE_CACHE
		  , mSize(0)
		  #endif
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline SListBase<T, Allocator>::SListBase(const allocator_type& allocator)
		: mNodeAllocator(base_node_type(), allocator)
		  #if EASTL_SLIST_SIZE_CACHE
		  , mSize(0)
		  #endif
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline SListBase<T, Allocator>::~SListBase()
	{
		DoEraseAfter(&internalNode(), NULL);
	}


	template <typename T, typename Allocator>
	inline const typename SListBase<T, Allocator>::allocator_type&
	SListBase<T, Allocator>::get_allocator() const EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename SListBase<T, Allocator>::allocator_type&
	SListBase<T, Allocator>::get_allocator() EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	void
	SListBase<T, Allocator>::set_allocator(const allocator_type& allocator)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline SListNode<T>* SListBase<T, Allocator>::DoAllocateNode()
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline void SListBase<T, Allocator>::DoFreeNode(node_type* pNode)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	SListNodeBase* SListBase<T, Allocator>::DoEraseAfter(SListNodeBase* pNode)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	SListNodeBase* SListBase<T, Allocator>::DoEraseAfter(SListNodeBase* pNode, SListNodeBase* pNodeLast)
	{
    __builtin_trap() /* STUB: not implemented */;
}




	///////////////////////////////////////////////////////////////////////
	// slist functions
	///////////////////////////////////////////////////////////////////////

	template <typename T, typename Allocator>
	inline slist<T, Allocator>::slist()
		: base_type()
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline slist<T, Allocator>::slist(const allocator_type& allocator)
		: base_type(allocator)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline slist<T, Allocator>::slist(size_type n, const allocator_type& allocator)
		: base_type(allocator)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline slist<T, Allocator>::slist(size_type n, const value_type& value, const allocator_type& allocator)
		: base_type(allocator)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline slist<T, Allocator>::slist(const slist& x)
		: base_type(x.internalAllocator())
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	slist<T, Allocator>::slist(this_type&& x)
		: base_type(x.internalAllocator())
	{
		swap(x);
	}

	template <typename T, typename Allocator>
	slist<T, Allocator>::slist(this_type&& x, const allocator_type& allocator)
		: base_type(allocator)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline slist<T, Allocator>::slist(std::initializer_list<value_type> ilist, const allocator_type& allocator)
		: base_type(allocator)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	template <typename InputIterator>
	inline slist<T, Allocator>::slist(InputIterator first, InputIterator last)
		: base_type(EASTL_SLIST_DEFAULT_ALLOCATOR)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename slist<T, Allocator>::iterator
	slist<T, Allocator>::begin() EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename slist<T, Allocator>::const_iterator
	slist<T, Allocator>::begin() const EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename slist<T, Allocator>::const_iterator
	slist<T, Allocator>::cbegin() const EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename slist<T, Allocator>::iterator
	slist<T, Allocator>::end() EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename slist<T, Allocator>::const_iterator
	slist<T, Allocator>::end() const EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename slist<T, Allocator>::const_iterator
	slist<T, Allocator>::cend() const EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename slist<T, Allocator>::iterator
	slist<T, Allocator>::before_begin() EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename slist<T, Allocator>::const_iterator
	slist<T, Allocator>::before_begin() const EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename slist<T, Allocator>::const_iterator
	slist<T, Allocator>::cbefore_begin() const EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename slist<T, Allocator>::iterator
	slist<T, Allocator>::previous(const_iterator position)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename slist<T, Allocator>::const_iterator
	slist<T, Allocator>::previous(const_iterator position) const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename slist<T, Allocator>::reference
	slist<T, Allocator>::front()
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename slist<T, Allocator>::const_reference
	slist<T, Allocator>::front() const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	template <class... Args>
	typename slist<T, Allocator>::reference slist<T, Allocator>::emplace_front(Args&&... args)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline void slist<T, Allocator>::push_front(const value_type& value)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename slist<T, Allocator>::reference
	slist<T, Allocator>::push_front()
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T, typename Allocator>
	void slist<T, Allocator>::push_front(value_type&& value)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	void slist<T, Allocator>::pop_front()
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	typename slist<T, Allocator>::this_type& slist<T, Allocator>::operator=(const this_type& x)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	typename slist<T, Allocator>::this_type& slist<T, Allocator>::operator=(this_type&& x)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	typename slist<T, Allocator>::this_type& slist<T, Allocator>::operator=(std::initializer_list<value_type> ilist)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline void slist<T, Allocator>::assign(std::initializer_list<value_type> ilist)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	template <typename InputIterator>                                                // It turns out that the C++ std::list specifies a two argument
	inline void slist<T, Allocator>::assign(InputIterator first, InputIterator last) // version of assign that takes (int size, int value). These are not 
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline void slist<T, Allocator>::assign(size_type n, const value_type& value)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	
	// does not propagate allocators on swap.
	// in addition, requires T be copy constructible and copy assignable, which isn't required by the standard.
	template <typename T, typename Allocator>
	inline void slist<T, Allocator>::swap(this_type& x)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline bool slist<T, Allocator>::empty() const EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename slist<T, Allocator>::size_type
	slist<T, Allocator>::size() const EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline void slist<T, Allocator>::clear() EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline void slist<T, Allocator>::reset_lose_memory() EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	void slist<T, Allocator>::resize(size_type n, const value_type& value)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline void slist<T, Allocator>::resize(size_type n)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename slist<T, Allocator>::iterator
	slist<T, Allocator>::insert(const_iterator position)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename slist<T, Allocator>::iterator
	slist<T, Allocator>::insert(const_iterator position, const value_type& value)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline void slist<T, Allocator>::insert(const_iterator position, size_type n, const value_type& value)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	template <typename InputIterator>
	inline void slist<T, Allocator>::insert(const_iterator position, InputIterator first, InputIterator last)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename slist<T, Allocator>::iterator
	slist<T, Allocator>::insert_after(const_iterator position)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename slist<T, Allocator>::iterator
	slist<T, Allocator>::insert_after(const_iterator position, const value_type& value)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename slist<T, Allocator>::iterator
	slist<T, Allocator>::insert_after(const_iterator position, size_type n, const value_type& value)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename slist<T, Allocator>::iterator
	slist<T, Allocator>::insert_after(const_iterator position, std::initializer_list<value_type> ilist)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	template <typename InputIterator>
	inline typename slist<T, Allocator>::iterator
	slist<T, Allocator>::insert_after(const_iterator position, InputIterator first, InputIterator last)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename slist<T, Allocator>::iterator
	slist<T, Allocator>::insert_after(const_iterator position, value_type&& value)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	template <class... Args>
	inline typename slist<T, Allocator>::iterator
	slist<T, Allocator>::emplace_after(const_iterator position, Args&&... args)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename slist<T, Allocator>::iterator
	slist<T, Allocator>::erase(const_iterator position)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename slist<T, Allocator>::iterator
	slist<T, Allocator>::erase(const_iterator first, const_iterator last)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename slist<T, Allocator>::iterator
	slist<T, Allocator>::erase_after(const_iterator position)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename slist<T, Allocator>::iterator
	slist<T, Allocator>::erase_after(const_iterator before_first, const_iterator last)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	typename slist<T, Allocator>::size_type slist<T, Allocator>::remove(const value_type& value)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T, typename Allocator>
	template <typename Predicate>
	inline typename slist<T, Allocator>::size_type slist<T, Allocator>::remove_if(Predicate predicate)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline void slist<T, Allocator>::splice(const_iterator position, this_type& x)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline void slist<T, Allocator>::splice(const_iterator position, this_type& x, const_iterator i)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline void slist<T, Allocator>::splice(const_iterator position, this_type& x, const_iterator first, const_iterator last)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	void slist<T, Allocator>::splice(const_iterator position, this_type&& x)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T, typename Allocator>
	void slist<T, Allocator>::splice(const_iterator position, this_type&& x, const_iterator i)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T, typename Allocator>
	void slist<T, Allocator>::splice(const_iterator position, this_type&& x, const_iterator first, const_iterator last)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline void slist<T, Allocator>::splice_after(const_iterator position, this_type& x)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline void slist<T, Allocator>::splice_after(const_iterator position, this_type& x, const_iterator i)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline void slist<T, Allocator>::splice_after(const_iterator position, this_type& x, const_iterator first, const_iterator last)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline void slist<T, Allocator>::splice_after(const_iterator position, this_type&& x)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T, typename Allocator>
	inline void slist<T, Allocator>::splice_after(const_iterator position, this_type&& x, const_iterator i) 
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T, typename Allocator>
	inline void slist<T, Allocator>::splice_after(const_iterator position, this_type&& x, const_iterator first, const_iterator last)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	typename slist<T, Allocator>::size_type slist<T, Allocator>::unique()
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	template <typename BinaryPredicate>
	typename slist<T, Allocator>::size_type slist<T, Allocator>::unique(BinaryPredicate predicate)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline void slist<T, Allocator>::sort()
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	template <class Compare>
	inline void slist<T, Allocator>::sort(Compare compare)
	{
    __builtin_trap() /* STUB: not implemented */;
}



	template <typename T, typename Allocator>
	inline void slist<T, Allocator>::reverse() EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	template<typename... Args>
	inline typename slist<T, Allocator>::node_type*
	slist<T, Allocator>::DoCreateNode(Args&&... args)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename slist<T, Allocator>::node_type*
	slist<T, Allocator>::DoCreateNode()
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	template <typename Integer>
	void slist<T, Allocator>::DoAssign(Integer n, Integer value, true_type)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	template <typename InputIterator>
	void slist<T, Allocator>::DoAssign(InputIterator first, InputIterator last, false_type)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	void slist<T, Allocator>::DoAssignValues(size_type n, const value_type& value)
	{
    __builtin_trap() /* STUB: not implemented */;
}
		

	template <typename T, typename Allocator>
	template <typename InputIterator>
	inline typename slist<T, Allocator>::node_type*
	slist<T, Allocator>::DoInsertAfter(SListNodeBase* pNode, InputIterator first, InputIterator last)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	template <typename Integer>
	inline typename slist<T, Allocator>::node_type*
	slist<T, Allocator>::DoInsertAfter(SListNodeBase* pNode, Integer n, Integer value, true_type)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	template <typename InputIterator>
	inline typename slist<T, Allocator>::node_type*
	slist<T, Allocator>::DoInsertAfter(SListNodeBase* pNode, InputIterator first, InputIterator last, false_type)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename slist<T, Allocator>::node_type*
	slist<T, Allocator>::DoInsertValueAfter(SListNodeBase* pNode)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	template<typename... Args>
	inline typename slist<T, Allocator>::node_type*
	slist<T, Allocator>::DoInsertValueAfter(SListNodeBase* pNode, Args&&... args)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename slist<T, Allocator>::node_type*
	slist<T, Allocator>::DoInsertValuesAfter(SListNodeBase* pNode, size_type n, const value_type& value)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline void slist<T, Allocator>::DoSwap(this_type& x)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline bool slist<T, Allocator>::validate() const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline int slist<T, Allocator>::validate_iterator(const_iterator i) const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	///////////////////////////////////////////////////////////////////////
	// global operators
	///////////////////////////////////////////////////////////////////////

	template <typename T, typename Allocator>
	bool operator==(const slist<T, Allocator>& a, const slist<T, Allocator>& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}

#if defined(EA_COMPILER_HAS_THREE_WAY_COMPARISON)
	template <typename T, typename Allocator>
	inline synth_three_way_result<T> operator<=>(const slist<T, Allocator>& a, const slist<T, Allocator>& b)
	{
		return eastl::lexicographical_compare_three_way(a.begin(), a.end(), b.begin(), b.end(), synth_three_way{});
	}
#else
	template <typename T, typename Allocator>
	inline bool operator<(const slist<T, Allocator>& a, const slist<T, Allocator>& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline bool operator!=(const slist<T, Allocator>& a, const slist<T, Allocator>& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline bool operator>(const slist<T, Allocator>& a, const slist<T, Allocator>& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline bool operator<=(const slist<T, Allocator>& a, const slist<T, Allocator>& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline bool operator>=(const slist<T, Allocator>& a, const slist<T, Allocator>& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}
#endif

	template <typename T, typename Allocator>
	inline void swap(slist<T, Allocator>& a, slist<T, Allocator>& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	/// erase / erase_if
	///
	/// https://en.cppreference.com/w/cpp/container/forward_list/erase2
	template <class T, class Allocator, class U>
	typename slist<T, Allocator>::size_type erase(slist<T, Allocator>& c, const U& value)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <class T, class Allocator, class Predicate>
	typename slist<T, Allocator>::size_type erase_if(slist<T, Allocator>& c, Predicate predicate)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	/// insert_iterator
	///
	/// We borrow a trick from SGI STL here and define an insert_iterator 
	/// specialization for slist. This allows slist insertions to be O(1) 
	/// instead of O(n/2), due to caching of the previous node.
	///
	template <typename T, typename Allocator>
	class insert_iterator< slist<T, Allocator> >
	{
	public:
		typedef slist<T, Allocator>                 Container;
		typedef typename Container::const_reference const_reference;
		typedef typename Container::iterator        iterator_type;
		typedef eastl::output_iterator_tag   iterator_category;
		typedef void                                value_type;
		typedef void                                difference_type;
		typedef void                                pointer;
		typedef void                                reference;

	protected:
		Container&    container;
		iterator_type it;

	public:
		insert_iterator(Container& x, iterator_type i)
			: container(x)
		{
    __builtin_trap() /* STUB: not implemented */;
}

		insert_iterator<Container>& operator=(const_reference value)
			{
    __builtin_trap() /* STUB: not implemented */;
}

		insert_iterator<Container>& operator*()
			{
    __builtin_trap() /* STUB: not implemented */;
}

		insert_iterator<Container>& operator++()
			{
    __builtin_trap() /* STUB: not implemented */;
} // This is by design.

		insert_iterator<Container>& operator++(int)
			{
    __builtin_trap() /* STUB: not implemented */;
} // This is by design.

	}; // insert_iterator<slist>


} // namespace eastl

EA_RESTORE_SN_WARNING()

EA_RESTORE_VC_WARNING();


#endif // Header include guard
