#include <cstdlib>
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) Electronic Arts Inc. All rights reserved.
/////////////////////////////////////////////////////////////////////////////


#ifndef EASTL_RED_BLACK_TREE_H
#define EASTL_RED_BLACK_TREE_H


#include <EABase/eabase.h>
#if defined(EA_PRAGMA_ONCE_SUPPORTED)
	#pragma once
#endif

#include <EASTL/internal/config.h>
#include <EASTL/type_traits.h>
#include <EASTL/allocator.h>
#include <EASTL/iterator.h>
#include <EASTL/utility.h>
#include <EASTL/algorithm.h>
#include <EASTL/initializer_list.h>
#include <EASTL/tuple.h>
#include <EASTL/memory.h>
#if EASTL_EXCEPTIONS_ENABLED
#include <stdexcept>
#endif

EA_DISABLE_ALL_VC_WARNINGS()
#include <new>
#include <stddef.h>
EA_RESTORE_ALL_VC_WARNINGS()


// 4512/4626 - 'class' : assignment operator could not be generated
// 4530 - C++ exception handler used, but unwind semantics are not enabled. Specify /EHsc
// 4571 - catch(...) semantics changed since Visual C++ 7.1; structured exceptions (SEH) are no longer caught.
EA_DISABLE_VC_WARNING(4512 4626 4530 4571);


namespace eastl
{

	/// EASTL_RBTREE_DEFAULT_NAME
	///
	/// Defines a default container name in the absence of a user-provided name.
	///
	#ifndef EASTL_RBTREE_DEFAULT_NAME
		#define EASTL_RBTREE_DEFAULT_NAME EASTL_DEFAULT_NAME_PREFIX " rbtree" // Unless the user overrides something, this is "EASTL rbtree".
	#endif


	/// EASTL_RBTREE_DEFAULT_ALLOCATOR
	///
	#ifndef EASTL_RBTREE_DEFAULT_ALLOCATOR
		#define EASTL_RBTREE_DEFAULT_ALLOCATOR allocator_type(EASTL_RBTREE_DEFAULT_NAME)
	#endif


	/// EASTL_RBTREE_LEGACY_SWAP_BEHAVIOUR_REQUIRES_COPY_CTOR
	///
	#ifndef EASTL_RBTREE_LEGACY_SWAP_BEHAVIOUR_REQUIRES_COPY_CTOR
		#define EASTL_RBTREE_LEGACY_SWAP_BEHAVIOUR_REQUIRES_COPY_CTOR 0
	#endif


	/// RBTreeColor
	///
	enum RBTreeColor
	{
		kRBTreeColorRed,
		kRBTreeColorBlack
	};



	/// RBTreeColor
	///
	enum RBTreeSide
	{
		kRBTreeSideLeft,
		kRBTreeSideRight
	};



	/// rbtree_node_base
	///
	/// We define a rbtree_node_base separately from rbtree_node (below), because it 
	/// allows us to have non-templated operations, and it makes it so that the 
	/// rbtree anchor node doesn't carry a T with it, which would waste space and 
	/// possibly lead to surprising the user due to extra Ts existing that the user 
	/// didn't explicitly create. The downside to all of this is that it makes debug 
	/// viewing of an rbtree harder, given that the node pointers are of type 
	/// rbtree_node_base and not rbtree_node.
	///
	struct rbtree_node_base
	{
		typedef rbtree_node_base this_type;

	public:
		this_type* mpNodeRight;  // Declared first because it is used most often.
		this_type* mpNodeLeft;
		this_type* mpNodeParent;
		char       mColor;       // We only need one bit here, would be nice if we could stuff that bit somewhere else.
	};


	/// rbtree_node
	///
	template <typename Value>
	struct rbtree_node : public rbtree_node_base
	{
		Value mValue; // For set and multiset, this is the user's value, for map and multimap, this is a pair of key/value.

		// This type is never constructed, so to avoid a MSVC warning we "delete" the copy constructor.
		//
		// Potentially we could provide a constructor that would satisfy the compiler and change the code to use this constructor
		// instead of constructing mValue in place within an unconstructed rbtree_node.
		#if defined(_MSC_VER)
			rbtree_node(const rbtree_node&) = delete;
		#endif
	};




	// rbtree_node_base functions
	//
	// These are the fundamental functions that we use to maintain the 
	// tree. The bulk of the work of the tree maintenance is done in 
	// these functions.
	//
	EASTL_API rbtree_node_base* RBTreeIncrement    (const rbtree_node_base* pNode);
	EASTL_API rbtree_node_base* RBTreeDecrement    (const rbtree_node_base* pNode);
	EASTL_API rbtree_node_base* RBTreeGetMinChild  (const rbtree_node_base* pNode);
	EASTL_API rbtree_node_base* RBTreeGetMaxChild  (const rbtree_node_base* pNode);
	EASTL_API size_t            RBTreeGetBlackCount(const rbtree_node_base* pNodeTop,
													const rbtree_node_base* pNodeBottom);
	EASTL_API void              RBTreeInsert       (      rbtree_node_base* pNode,
														  rbtree_node_base* pNodeParent, 
														  rbtree_node_base* pNodeAnchor,
														  RBTreeSide insertionSide);
	EASTL_API void              RBTreeErase        (      rbtree_node_base* pNode,
														  rbtree_node_base* pNodeAnchor); 







	/// rbtree_iterator
	///
	template <typename T, typename Pointer, typename Reference>
	struct rbtree_iterator
	{
		typedef rbtree_iterator<T, Pointer, Reference>      this_type;
		typedef rbtree_iterator<T, T*, T&>                  iterator;
		typedef rbtree_iterator<T, const T*, const T&>      const_iterator;
		typedef eastl_size_t                                size_type;     // See config.h for the definition of eastl_size_t, which defaults to size_t.
		typedef ptrdiff_t                                   difference_type;
		typedef T                                           value_type;
		typedef rbtree_node_base                            base_node_type;
		typedef rbtree_node<T>                              node_type;
		typedef Pointer                                     pointer;
		typedef Reference                                   reference;
		typedef eastl::bidirectional_iterator_tag    iterator_category;

	private:
		base_node_type* mpNode;

	public:
		rbtree_iterator();
		explicit rbtree_iterator(const base_node_type* pNode);
		// Note: this isn't always a copy constructor, iterator is not always equal to this_type
		rbtree_iterator(const iterator& x);
		// Note: this isn't always a copy assignment operator, iterator is not always equal to this_type
		rbtree_iterator& operator=(const iterator& x);

		// Calling these on the end() of a tree invokes undefined behavior.
		reference operator*() const;
		pointer   operator->() const;

		rbtree_iterator& operator++();
		rbtree_iterator  operator++(int);

		rbtree_iterator& operator--();
		rbtree_iterator  operator--(int);
	private:

		template<class U, class PtrA, class RefA, class PtrB, class RefB>
		friend bool operator==(const rbtree_iterator<U, PtrA, RefA>&, const rbtree_iterator<U, PtrB, RefB>&);

		template<class U, class PtrA, class RefA, class PtrB, class RefB>
		friend bool operator!=(const rbtree_iterator<U, PtrA, RefA>&, const rbtree_iterator<U, PtrB, RefB>&);

		template<class U, class PtrA, class RefA>
		friend bool operator!=(const rbtree_iterator<U, PtrA, RefA>&, const rbtree_iterator<U, PtrA, RefA>&);

		// rbtree uses mpNode.
		template <class Key, class Value, class Compare, class Allocator,
				  class ExtractKey, bool bMutableIterators, bool bUniqueKeys>
		friend class rbtree;

		// for the "copy" constructor, which uses non-const iterator even in the
		// const_iterator case.
		friend iterator;
		friend const_iterator;
	}; // rbtree_iterator


	///////////////////////////////////////////////////////////////////////////////
	// rb_base_compare_ebo
	//
	// Utilizes the "empty base-class optimization" to reduce the size of the rbtree
	// when its Compare template argument is an empty class.
	///////////////////////////////////////////////////////////////////////////////

	template <typename Compare, bool /*isEmpty*/ = is_empty<Compare>::value>
	struct rb_base_compare_ebo
	{
	protected:
		rb_base_compare_ebo() : mCompare() {
    __builtin_trap() /* STUB: not implemented */;
}
		rb_base_compare_ebo(const Compare& compare) : mCompare(compare) {
    __builtin_trap() /* STUB: not implemented */;
}

		Compare& get_compare() {
    __builtin_trap() /* STUB: not implemented */;
}
		const Compare& get_compare() const {
    __builtin_trap() /* STUB: not implemented */;
}

		template <typename T, typename U>
		bool compare(const T& lhs, const U& rhs) 
		{
    __builtin_trap() /* STUB: not implemented */;
}

		template <typename T, typename U>
		bool compare(const T& lhs, const U& rhs) const
		{
    __builtin_trap() /* STUB: not implemented */;
}

	private:
		Compare mCompare;
	};

	template <typename Compare>
	struct rb_base_compare_ebo<Compare, true> : private Compare
	{
	protected:
		rb_base_compare_ebo() {
    __builtin_trap() /* STUB: not implemented */;
}
		rb_base_compare_ebo(const Compare& compare) : Compare(compare) {
    __builtin_trap() /* STUB: not implemented */;
}

		Compare& get_compare() {
    __builtin_trap() /* STUB: not implemented */;
}
		const Compare& get_compare() const {
    __builtin_trap() /* STUB: not implemented */;
}

		template <typename T, typename U>
		bool compare(const T& lhs, const U& rhs) 
		{
    __builtin_trap() /* STUB: not implemented */;
}

		template <typename T, typename U>
		bool compare(const T& lhs, const U& rhs) const
		{
    __builtin_trap() /* STUB: not implemented */;
}
	};



	///////////////////////////////////////////////////////////////////////////////
    // rb_base
    //
    // This class allows us to use a generic rbtree as the basis of map, multimap,
    // set, and multiset transparently. The vital template parameters for this are 
    // the ExtractKey and the bUniqueKeys parameters.
    //
    // If the rbtree has a value type of the form pair<T1, T2> (i.e. it is a map or
    // multimap and not a set or multiset) and a key extraction policy that returns 
    // the first part of the pair, the rbtree gets a mapped_type typedef. 
    // If it satisfies those criteria and also has unique keys, then it also gets an 
    // operator[] (which only map and set have and multimap and multiset don't have).
    //
    ///////////////////////////////////////////////////////////////////////////////



	/// rb_base
	/// This specialization is used for 'set'. In this case, Key and Value 
	/// will be the same as each other and ExtractKey will be eastl::use_self.
	///
	template <typename Key, typename Value, typename Compare, typename ExtractKey, bool bUniqueKeys, typename RBTree>
	struct rb_base : public rb_base_compare_ebo<Compare>
	{
		typedef ExtractKey extract_key;

	protected:
		using rb_base_compare_ebo<Compare>::compare;
		using rb_base_compare_ebo<Compare>::get_compare;

	public:
		rb_base() {
    __builtin_trap() /* STUB: not implemented */;
}
		rb_base(const Compare& compare) : rb_base_compare_ebo<Compare>(compare) {
    __builtin_trap() /* STUB: not implemented */;
}
	};


	/// rb_base
	/// This class is used for 'multiset'.
	/// In this case, Key and Value will be the same as each 
	/// other and ExtractKey will be eastl::use_self.
	///
	template <typename Key, typename Value, typename Compare, typename ExtractKey, typename RBTree>
	struct rb_base<Key, Value, Compare, ExtractKey, false, RBTree> : public rb_base_compare_ebo<Compare>
	{
		typedef ExtractKey extract_key;

	protected:
		using rb_base_compare_ebo<Compare>::compare;
		using rb_base_compare_ebo<Compare>::get_compare;

	public:
		rb_base() {
    __builtin_trap() /* STUB: not implemented */;
}
		rb_base(const Compare& compare) : rb_base_compare_ebo<Compare>(compare) {
    __builtin_trap() /* STUB: not implemented */;
}
	};


	/// rb_base
	/// This specialization is used for 'map'.
	///
	template <typename Key, typename Pair, typename Compare, typename RBTree>
	struct rb_base<Key, Pair, Compare, eastl::use_first<Pair>, true, RBTree> : public rb_base_compare_ebo<Compare>
	{
		typedef eastl::use_first<Pair> extract_key;

		using rb_base_compare_ebo<Compare>::compare;
		using rb_base_compare_ebo<Compare>::get_compare;

	public:
		rb_base() {
    __builtin_trap() /* STUB: not implemented */;
}
		rb_base(const Compare& compare) : rb_base_compare_ebo<Compare>(compare) {
    __builtin_trap() /* STUB: not implemented */;
}
	};


	/// rb_base
	/// This specialization is used for 'multimap'.
	///
	template <typename Key, typename Pair, typename Compare, typename RBTree>
	struct rb_base<Key, Pair, Compare, eastl::use_first<Pair>, false, RBTree> : public rb_base_compare_ebo<Compare>
	{
		typedef eastl::use_first<Pair> extract_key;

		using rb_base_compare_ebo<Compare>::compare;
		using rb_base_compare_ebo<Compare>::get_compare;

	public:
		rb_base() {
    __builtin_trap() /* STUB: not implemented */;
}
		rb_base(const Compare& compare) : rb_base_compare_ebo<Compare>(compare) {
    __builtin_trap() /* STUB: not implemented */;
}
	};


	/// rbtree
	///
	/// rbtree is the red-black tree basis for the map, multimap, set, and multiset 
	/// containers. Just about all the work of those containers is done here, and 
	/// they are merely a shell which sets template policies that govern the code
	/// generation for this rbtree.
	///
	/// This rbtree implementation is pretty much the same as all other modern 
	/// rbtree implementations, as the topic is well known and researched. We may
	/// choose to implement a "relaxed balancing" option at some point in the 
	/// future if it is deemed worthwhile. Most rbtree implementations don't do this.
	///
	/// The primary rbtree member variable is mAnchor, which is a node_type and 
	/// acts as the end node. However, like any other node, it has mpNodeLeft,
	/// mpNodeRight, and mpNodeParent members. We do the conventional trick of 
	/// assigning begin() (left-most rbtree node) to mpNodeLeft, assigning 
	/// 'end() - 1' (a.k.a. rbegin()) to mpNodeRight, and assigning the tree root
	/// node to mpNodeParent. 
	///
	/// Compare (functor): This is a comparison class which defaults to 'less'.
	/// It is a common STL thing which takes two arguments and returns true if  
	/// the first is less than the second.
	///
	/// ExtractKey (functor): This is a class which gets the key from a stored
	/// node. With map and set, the node is a pair, whereas with set and multiset
	/// the node is just the value. ExtractKey will be either eastl::use_first (map and multimap)
	/// or eastl::use_self (set and multiset).
	///
	/// bMutableIterators (bool): true if rbtree::iterator is a mutable
	/// iterator, false if iterator and const_iterator are both const iterators. 
	/// It will be true for map and multimap and false for set and multiset.
	///
	/// bUniqueKeys (bool): true if the keys are to be unique, and false if there
	/// can be multiple instances of a given key. It will be true for set and map 
	/// and false for multiset and multimap.
	///
	/// To consider: Add an option for relaxed tree balancing. This could result 
	/// in performance improvements but would require a more complicated implementation.
	///
	///////////////////////////////////////////////////////////////////////
	/// Heterogeneous lookup, insertion and erasure
	/// See
	/// https://en.cppreference.com/w/cpp/utility/functional#Transparent_function_objects
	/// https://en.cppreference.com/w/cpp/utility/functional/less_void
	/// https://en.cppreference.com/w/cpp/container/set/find
	/// 
	/// You can avoid creating key objects when calling member functions
	/// with a key_type parameter by declaring the container with
	/// transparent comparison type (eg. less<void>) and passing objects
	/// to be passed to this function object.
	/// 
	/// This optimization is supported for member functions that take a
	/// key_type parameter, ie. heterogeneous lookup, insertion and erasure,
	/// not just find().
	/// 
	/// Using transparent types is safer than using find_as because the
	/// latter requires the user specify comparison objects which must have
	/// the same semantics as the container's comparison object, otherwise
	/// the behaviour is undefined.
	/// 
	/// find_as
	/// Note: Prefer heterogeneous lookup (see above).
	/// 
	/// In order to support the ability to have a tree of strings but
	/// be able to do efficiently lookups via char pointers (i.e. so they
	/// aren't converted to string objects), we provide the find_as
	/// function. This function allows you to do a find with a key of a
	/// type other than the tree's key type. See the find_as function
	/// for more documentation on this.
	///
	template <typename Key, typename Value, typename Compare, typename Allocator, 
			  typename ExtractKey, bool bMutableIterators, bool bUniqueKeys>
	class rbtree
		: public rb_base<Key, Value, Compare, ExtractKey, bUniqueKeys, 
							rbtree<Key, Value, Compare, Allocator, ExtractKey, bMutableIterators, bUniqueKeys> >
	{
	public:
		typedef ptrdiff_t                                                                       difference_type;
		typedef eastl_size_t                                                                    size_type;     // See config.h for the definition of eastl_size_t, which defaults to size_t.
		typedef Key                                                                             key_type;
		typedef Value                                                                           value_type;
		typedef rbtree_node<value_type>                                                         node_type;
		typedef value_type&                                                                     reference;
		typedef const value_type&                                                               const_reference;
		typedef value_type*                                                                     pointer;
		typedef const value_type*                                                               const_pointer;

		typedef typename conditional<bMutableIterators,
					rbtree_iterator<value_type, value_type*, value_type&>, 
					rbtree_iterator<value_type, const value_type*, const value_type&> >::type   iterator;
		typedef rbtree_iterator<value_type, const value_type*, const value_type&>               const_iterator;
		typedef eastl::reverse_iterator<iterator>                                               reverse_iterator;
		typedef eastl::reverse_iterator<const_iterator>                                         const_reverse_iterator;

		typedef Allocator                                                                       allocator_type;
		typedef Compare                                                                         key_compare;
		typedef typename conditional<bUniqueKeys, eastl::pair<iterator, bool>, iterator>::type  insert_return_type;  // map/set::insert return a pair, multimap/multiset::iterator return an iterator.
		typedef rbtree<Key, Value, Compare, Allocator, 
						ExtractKey, bMutableIterators, bUniqueKeys>                             this_type;
		typedef rb_base<Key, Value, Compare, ExtractKey, bUniqueKeys, this_type>                base_type;
		typedef integral_constant<bool, bUniqueKeys>                                            has_unique_keys_type;
		typedef typename base_type::extract_key                                                 extract_key;

	protected:
		using base_type::compare;
		using base_type::get_compare;

	public:
		rbtree_node_base  mAnchor;      /// This node acts as end() and its mpLeft points to begin(), and mpRight points to rbegin() (the last node on the right).
		size_type         mnSize;       /// Stores the count of nodes in the tree (not counting the anchor node).
		allocator_type    mAllocator;   // To do: Use base class optimization to make this go away.

	public:
		// ctor/dtor
		rbtree();
		rbtree(const allocator_type& allocator);
		rbtree(const Compare& compare, const allocator_type& allocator = EASTL_RBTREE_DEFAULT_ALLOCATOR);
		rbtree(const this_type& x);
		rbtree(this_type&& x);
		rbtree(this_type&& x, const allocator_type& allocator);

		template <typename InputIterator>
		rbtree(InputIterator first, InputIterator last, const Compare& compare, const allocator_type& allocator = EASTL_RBTREE_DEFAULT_ALLOCATOR);

	   ~rbtree();

	public:
		// properties
		const allocator_type& get_allocator() const EA_NOEXCEPT;
		allocator_type&       get_allocator() EA_NOEXCEPT;
		void                  set_allocator(const allocator_type& allocator);

		const key_compare& key_comp() const {
    __builtin_trap() /* STUB: not implemented */;
}
		key_compare&       key_comp()       {
    __builtin_trap() /* STUB: not implemented */;
}

		this_type& operator=(const this_type& x);
		this_type& operator=(std::initializer_list<value_type> ilist);
		this_type& operator=(this_type&& x);

		void swap(this_type& x);

	public: 
		// iterators
		iterator        begin() EA_NOEXCEPT;
		const_iterator  begin() const EA_NOEXCEPT;
		const_iterator  cbegin() const EA_NOEXCEPT;

		iterator        end() EA_NOEXCEPT;
		const_iterator  end() const EA_NOEXCEPT;
		const_iterator  cend() const EA_NOEXCEPT;

		reverse_iterator        rbegin() EA_NOEXCEPT;
		const_reverse_iterator  rbegin() const EA_NOEXCEPT;
		const_reverse_iterator  crbegin() const EA_NOEXCEPT;

		reverse_iterator        rend() EA_NOEXCEPT;
		const_reverse_iterator  rend() const EA_NOEXCEPT;
		const_reverse_iterator  crend() const EA_NOEXCEPT;

	public:
		bool      empty() const EA_NOEXCEPT;
		size_type size() const EA_NOEXCEPT;

		template <class... Args>
		insert_return_type emplace(Args&&... args);

		template <class... Args> 
		iterator emplace_hint(const_iterator position, Args&&... args);

		// Currently limited to value_type instead of P because it collides with insert(InputIterator, InputIterator).
		// To allow this to work with templated P we need to implement a compile-time specialization for the
		// case that P&& is const_iterator and have that specialization handle insert(InputIterator, InputIterator)
		// instead of insert(InputIterator, InputIterator). Curiously, neither libstdc++ nor libc++
		// implement this function either, which suggests they ran into the same problem I did here
		// and haven't yet resolved it (at least as of March 2014, GCC 4.8.1).
		iterator insert(const_iterator hint, value_type&& value);

		/// map::insert and set::insert return a pair, while multimap::insert and
		/// multiset::insert return an iterator.
		insert_return_type insert(const value_type& value);
		insert_return_type insert(value_type&& value);

		// C++ standard: inserts value if and only if there is no element with 
		// key equivalent to the key of t in containers with unique keys; always 
		// inserts value in containers with equivalent keys. Always returns the 
		// iterator pointing to the element with key equivalent to the key of value. 
		// iterator position is a hint pointing to where the insert should start
		// to search. However, there is a potential defect/improvement report on this behaviour:
		// LWG issue #233 (http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2005/n1780.html)
		// We follow the same approach as SGI STL/STLPort and use the position as
		// a forced insertion position for the value when possible.
		iterator insert(const_iterator position, const value_type& value);

		void insert(std::initializer_list<value_type> ilist);

		template <typename InputIterator>
		void insert(InputIterator first, InputIterator last);

		// TODO(rparolin):
		// insert_return_type insert(node_type&& nh);
		// iterator insert(const_iterator hint, node_type&& nh);

		template <class M> pair<iterator, bool> insert_or_assign(const key_type& k, M&& obj) {
    __builtin_trap() /* STUB: not implemented */;
}
		template <class M> pair<iterator, bool> insert_or_assign(key_type&& k, M&& obj) {
    __builtin_trap() /* STUB: not implemented */;
}
		template<typename KX, typename M, typename Cmp = Compare, eastl::enable_if_t<eastl::detail::is_transparent_comparison_v<Cmp>, bool> = true>
		pair<iterator, bool>					insert_or_assign(KX&& k, M&& obj) {
    __builtin_trap() /* STUB: not implemented */;
}
		template <class M> iterator             insert_or_assign(const_iterator hint, const key_type& k, M&& obj) {
    __builtin_trap() /* STUB: not implemented */;
}
		template <class M> iterator             insert_or_assign(const_iterator hint, key_type&& k, M&& obj) {
    __builtin_trap() /* STUB: not implemented */;
}
		template<typename KX, typename M, typename Cmp = Compare, eastl::enable_if_t<eastl::detail::is_transparent_comparison_v<Cmp>, bool> = true>
		iterator								insert_or_assign(const_iterator hint, KX&& k, M&& obj) {
    __builtin_trap() /* STUB: not implemented */;
}

		template <typename Iter = iterator, typename eastl::enable_if<!eastl::is_same_v<Iter, const_iterator>, int>::type = 0>
		iterator         erase(iterator position) {
    __builtin_trap() /* STUB: not implemented */;
}
		iterator         erase(const_iterator position);
		iterator         erase(const_iterator first, const_iterator last);
		reverse_iterator erase(const_reverse_iterator position);
		reverse_iterator erase(const_reverse_iterator first, const_reverse_iterator last);

		// For some reason, multiple STL versions make a specialization 
		// for erasing an array of key_types. I'm pretty sure we don't
		// need this, but just to be safe we will follow suit. 
		// The implementation is trivial. Returns void because the values
		// could well be randomly distributed throughout the tree and thus
		// a return value would be nearly meaningless.
		void erase(const key_type* first, const key_type* last);

		void clear();
		void reset_lose_memory(); // This is a unilateral reset to an initially empty state. No destructors are called, no deallocation occurs.

		iterator       find(const key_type& key) {
    __builtin_trap() /* STUB: not implemented */;
}
		const_iterator find(const key_type& key) const {
    __builtin_trap() /* STUB: not implemented */;
}

		template<typename KX, typename Cmp = Compare, eastl::enable_if_t<eastl::detail::is_transparent_comparison_v<Cmp>, bool> = true>
		iterator       find(const KX& key) {
    __builtin_trap() /* STUB: not implemented */;
}
		template<typename KX, typename Cmp = Compare, eastl::enable_if_t<eastl::detail::is_transparent_comparison_v<Cmp>, bool> = true>
		const_iterator find(const KX& key) const {
    __builtin_trap() /* STUB: not implemented */;
}

		/// Implements a find whereby the user supplies a comparison of a different type
		/// than the tree's value_type. A useful case of this is one whereby you have
		/// a container of string objects but want to do searches via passing in char pointers.
		/// The problem is that without this kind of find, you need to do the expensive operation
		/// of converting the char pointer to a string so it can be used as the argument to the
		/// find function.
		///
		/// Example usage (note that the compare uses string as first type and char* as second):
		///     set<string> strings;
		///     strings.find_as("hello", less<>());
		///
		template <typename U, typename Compare2> iterator       find_as(const U& u, Compare2 compare2);
		template <typename U, typename Compare2> const_iterator find_as(const U& u, Compare2 compare2) const;

		bool contains(const key_type& key) const {
    __builtin_trap() /* STUB: not implemented */;
}

		template<typename KX, typename Cmp = Compare, eastl::enable_if_t<eastl::detail::is_transparent_comparison_v<Cmp>, bool> = true>
		bool contains(const KX& key) const {
    __builtin_trap() /* STUB: not implemented */;
}

		iterator       lower_bound(const key_type& key) {
    __builtin_trap() /* STUB: not implemented */;
}
		const_iterator lower_bound(const key_type& key) const {
    __builtin_trap() /* STUB: not implemented */;
}

		template<typename KX, typename Cmp = Compare, eastl::enable_if_t<eastl::detail::is_transparent_comparison_v<Cmp>, bool> = true>
		iterator       lower_bound(const KX& key) {
    __builtin_trap() /* STUB: not implemented */;
}
		template<typename KX, typename Cmp = Compare, eastl::enable_if_t<eastl::detail::is_transparent_comparison_v<Cmp>, bool> = true>
		const_iterator lower_bound(const KX& key) const {
    __builtin_trap() /* STUB: not implemented */;
}

		iterator       upper_bound(const key_type& key) {
    __builtin_trap() /* STUB: not implemented */;
}
		const_iterator upper_bound(const key_type& key) const {
    __builtin_trap() /* STUB: not implemented */;
}

		template<typename KX, typename Cmp = Compare, eastl::enable_if_t<eastl::detail::is_transparent_comparison_v<Cmp>, bool> = true>
		iterator       upper_bound(const KX& key) {
    __builtin_trap() /* STUB: not implemented */;
}
		template<typename KX, typename Cmp = Compare, eastl::enable_if_t<eastl::detail::is_transparent_comparison_v<Cmp>, bool> = true>
		const_iterator upper_bound(const KX& key) const {
    __builtin_trap() /* STUB: not implemented */;
}

		bool validate() const;
		int  validate_iterator(const_iterator i) const;

	protected:
		node_type* DoAllocateNode();
		void       DoFreeNode(node_type* pNode);

		node_type* DoCreateNodeFromKey(const key_type& key);

		template<class... Args>
		node_type* DoCreateNode(Args&&... args);
		node_type* DoCreateNode(const value_type& value);
		node_type* DoCreateNode(value_type&& value);
		node_type* DoCreateNode(const node_type* pNodeSource, rbtree_node_base* pNodeParent);

		rbtree_node_base* DoCopySubtree(const node_type* pNodeSource, rbtree_node_base* pNodeDest);
		void       DoNukeSubtree(rbtree_node_base* pNode);

		template <class... Args>
		eastl::pair<iterator, bool> DoInsertValue(true_type, Args&&... args);

		template <class... Args>
		iterator DoInsertValue(false_type, Args&&... args);

		eastl::pair<iterator, bool> DoInsertValue(true_type, value_type&& value);
		iterator DoInsertValue(false_type, value_type&& value);

		template <class... Args>
		iterator DoInsertValueImpl(rbtree_node_base* pNodeParent, bool bForceToLeft, const key_type& key, Args&&... args);
		iterator DoInsertValueImpl(rbtree_node_base* pNodeParent, bool bForceToLeft, const key_type& key, node_type* pNodeNew);

		eastl::pair<iterator, bool> DoInsertKey(true_type, const key_type& key);
		iterator                    DoInsertKey(false_type, const key_type& key);

		template <class... Args>
		iterator DoInsertValueHint(true_type, const_iterator position, Args&&... args);

		template <class... Args>
		iterator DoInsertValueHint(false_type, const_iterator position, Args&&... args);

		iterator DoInsertValueHint(true_type, const_iterator position, value_type&& value);
		iterator DoInsertValueHint(false_type, const_iterator position, value_type&& value);

		iterator DoInsertKey(true_type, const_iterator position, const key_type& key);  // By design we return iterator and not a pair.
		iterator DoInsertKey(false_type, const_iterator position, const key_type& key);
		iterator DoInsertKeyImpl(rbtree_node_base* pNodeParent, bool bForceToLeft, const key_type& key);

		template <typename KX>
		rbtree_node_base* DoGetKeyInsertionPositionUniqueKeys(bool& canInsert, const KX& key);
		rbtree_node_base* DoGetKeyInsertionPositionNonuniqueKeys(const key_type& key);

		template <typename KX>
		rbtree_node_base* DoGetKeyInsertionPositionUniqueKeysHint(const_iterator position, bool& bForceToLeft, const KX& key);
		rbtree_node_base* DoGetKeyInsertionPositionNonuniqueKeysHint(const_iterator position, bool& bForceToLeft, const key_type& key);

		template<typename KX, typename M>
		pair<iterator, bool>	DoInsertOrAssign(KX&& k, M&& obj);
		template<typename KX, typename M>
		iterator				DoInsertOrAssign(const_iterator hint, KX&& k, M&& obj);

		template<typename KX>
		iterator DoFind(const KX& key);

		template<typename KX>
		const_iterator DoFind(const KX& key) const;

		template<typename KX>
		iterator DoLowerBound(const KX& key);

		template<typename KX>
		const_iterator DoLowerBound(const KX& key) const;

		template<typename KX>
		iterator DoUpperBound(const KX& key);

		template<typename KX>
		const_iterator DoUpperBound(const KX& key) const;

	}; // rbtree





	///////////////////////////////////////////////////////////////////////
	// rbtree_node_base functions
	///////////////////////////////////////////////////////////////////////

	EASTL_API inline rbtree_node_base* RBTreeGetMinChild(const rbtree_node_base* pNodeBase)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	EASTL_API inline rbtree_node_base* RBTreeGetMaxChild(const rbtree_node_base* pNodeBase)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	// The rest of the functions are non-trivial and are found in 
	// the corresponding .cpp file to this file.



	///////////////////////////////////////////////////////////////////////
	// rbtree_iterator functions
	///////////////////////////////////////////////////////////////////////

	template <typename T, typename Pointer, typename Reference>
	rbtree_iterator<T, Pointer, Reference>::rbtree_iterator()
		: mpNode(NULL) {
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Pointer, typename Reference>
	rbtree_iterator<T, Pointer, Reference>::rbtree_iterator(const base_node_type* pNode)
		: mpNode(const_cast<base_node_type*>(pNode)) {
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Pointer, typename Reference>
	rbtree_iterator<T, Pointer, Reference>::rbtree_iterator(const iterator& x)
		: mpNode(x.mpNode) {
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T, typename Pointer, typename Reference>
	typename rbtree_iterator<T, Pointer, Reference>::this_type&
	rbtree_iterator<T, Pointer, Reference>::operator=(const iterator& x)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T, typename Pointer, typename Reference>
	typename rbtree_iterator<T, Pointer, Reference>::reference
	rbtree_iterator<T, Pointer, Reference>::operator*() const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Pointer, typename Reference>
	typename rbtree_iterator<T, Pointer, Reference>::pointer
	rbtree_iterator<T, Pointer, Reference>::operator->() const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Pointer, typename Reference>
	typename rbtree_iterator<T, Pointer, Reference>::this_type&
	rbtree_iterator<T, Pointer, Reference>::operator++()
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Pointer, typename Reference>
	typename rbtree_iterator<T, Pointer, Reference>::this_type
	rbtree_iterator<T, Pointer, Reference>::operator++(int)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Pointer, typename Reference>
	typename rbtree_iterator<T, Pointer, Reference>::this_type&
	rbtree_iterator<T, Pointer, Reference>::operator--()
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Pointer, typename Reference>
	typename rbtree_iterator<T, Pointer, Reference>::this_type
	rbtree_iterator<T, Pointer, Reference>::operator--(int)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	// The C++ defect report #179 requires that we support comparisons between const and non-const iterators.
	// Thus we provide additional template paremeters here to support this. The defect report does not
	// require us to support comparisons between reverse_iterators and const_reverse_iterators.
	template <typename T, typename PointerA, typename ReferenceA, typename PointerB, typename ReferenceB>
	inline bool operator==(const rbtree_iterator<T, PointerA, ReferenceA>& a, 
						   const rbtree_iterator<T, PointerB, ReferenceB>& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename PointerA, typename ReferenceA, typename PointerB, typename ReferenceB>
	inline bool operator!=(const rbtree_iterator<T, PointerA, ReferenceA>& a, 
						   const rbtree_iterator<T, PointerB, ReferenceB>& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	// We provide a version of operator!= for the case where the iterators are of the 
	// same type. This helps prevent ambiguity errors in the presence of rel_ops.
	template <typename T, typename Pointer, typename Reference>
	inline bool operator!=(const rbtree_iterator<T, Pointer, Reference>& a, 
						   const rbtree_iterator<T, Pointer, Reference>& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}




	///////////////////////////////////////////////////////////////////////
	// rbtree functions
	///////////////////////////////////////////////////////////////////////

	template <typename K, typename V, typename C, typename A, typename E, bool bM, bool bU>
	inline rbtree<K, V, C, A, E, bM, bU>::rbtree()
		: mAnchor(),
		  mnSize(0),
		  mAllocator(EASTL_RBTREE_DEFAULT_NAME)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename C, typename A, typename E, bool bM, bool bU>
	inline rbtree<K, V, C, A, E, bM, bU>::rbtree(const allocator_type& allocator)
		: mAnchor(),
		  mnSize(0),
		  mAllocator(allocator)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename C, typename A, typename E, bool bM, bool bU>
	inline rbtree<K, V, C, A, E, bM, bU>::rbtree(const C& compare, const allocator_type& allocator)
		: base_type(compare),
		  mAnchor(),
		  mnSize(0),
		  mAllocator(allocator)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename C, typename A, typename E, bool bM, bool bU>
	inline rbtree<K, V, C, A, E, bM, bU>::rbtree(const this_type& x)
		: base_type(x.get_compare()),
		  mAnchor(),
		  mnSize(0),
		  mAllocator(x.mAllocator)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename C, typename A, typename E, bool bM, bool bU>
	inline rbtree<K, V, C, A, E, bM, bU>::rbtree(this_type&& x)
		: base_type(x.get_compare()),
		  mAnchor(),
		  mnSize(0),
		  mAllocator(x.mAllocator)
	{
		reset_lose_memory();
		swap(x);
	}

	template <typename K, typename V, typename C, typename A, typename E, bool bM, bool bU>
	inline rbtree<K, V, C, A, E, bM, bU>::rbtree(this_type&& x, const allocator_type& allocator)
		: base_type(x.get_compare()),
		  mAnchor(),
		  mnSize(0),
		  mAllocator(allocator)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename C, typename A, typename E, bool bM, bool bU>
	template <typename InputIterator>
	inline rbtree<K, V, C, A, E, bM, bU>::rbtree(InputIterator first, InputIterator last, const C& compare, const allocator_type& allocator)
		: base_type(compare),
		  mAnchor(),
		  mnSize(0),
		  mAllocator(allocator)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename C, typename A, typename E, bool bM, bool bU>
	inline rbtree<K, V, C, A, E, bM, bU>::~rbtree()
	{
		// Erase the entire tree. DoNukeSubtree is not a 
		// conventional erase function, as it does no rebalancing.
		DoNukeSubtree((node_type*)mAnchor.mpNodeParent);
	}


	template <typename K, typename V, typename C, typename A, typename E, bool bM, bool bU>
	inline const typename rbtree<K, V, C, A, E, bM, bU>::allocator_type&
	rbtree<K, V, C, A, E, bM, bU>::get_allocator() const EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename C, typename A, typename E, bool bM, bool bU>
	inline typename rbtree<K, V, C, A, E, bM, bU>::allocator_type&
	rbtree<K, V, C, A, E, bM, bU>::get_allocator() EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename C, typename A, typename E, bool bM, bool bU>
	inline void rbtree<K, V, C, A, E, bM, bU>::set_allocator(const allocator_type& allocator)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename C, typename A, typename E, bool bM, bool bU>
	inline typename rbtree<K, V, C, A, E, bM, bU>::size_type
	rbtree<K, V, C, A, E, bM, bU>::size() const EA_NOEXCEPT
		{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename C, typename A, typename E, bool bM, bool bU>
	inline bool rbtree<K, V, C, A, E, bM, bU>::empty() const EA_NOEXCEPT
		{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename C, typename A, typename E, bool bM, bool bU>
	inline typename rbtree<K, V, C, A, E, bM, bU>::iterator
	rbtree<K, V, C, A, E, bM, bU>::begin() EA_NOEXCEPT
		{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename C, typename A, typename E, bool bM, bool bU>
	inline typename rbtree<K, V, C, A, E, bM, bU>::const_iterator
	rbtree<K, V, C, A, E, bM, bU>::begin() const EA_NOEXCEPT
		{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename C, typename A, typename E, bool bM, bool bU>
	inline typename rbtree<K, V, C, A, E, bM, bU>::const_iterator
	rbtree<K, V, C, A, E, bM, bU>::cbegin() const EA_NOEXCEPT
		{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename C, typename A, typename E, bool bM, bool bU>
	inline typename rbtree<K, V, C, A, E, bM, bU>::iterator
	rbtree<K, V, C, A, E, bM, bU>::end() EA_NOEXCEPT
		{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename C, typename A, typename E, bool bM, bool bU>
	inline typename rbtree<K, V, C, A, E, bM, bU>::const_iterator
	rbtree<K, V, C, A, E, bM, bU>::end() const EA_NOEXCEPT
		{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename C, typename A, typename E, bool bM, bool bU>
	inline typename rbtree<K, V, C, A, E, bM, bU>::const_iterator
	rbtree<K, V, C, A, E, bM, bU>::cend() const EA_NOEXCEPT
		{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename C, typename A, typename E, bool bM, bool bU>
	inline typename rbtree<K, V, C, A, E, bM, bU>::reverse_iterator
	rbtree<K, V, C, A, E, bM, bU>::rbegin() EA_NOEXCEPT
		{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename C, typename A, typename E, bool bM, bool bU>
	inline typename rbtree<K, V, C, A, E, bM, bU>::const_reverse_iterator
	rbtree<K, V, C, A, E, bM, bU>::rbegin() const EA_NOEXCEPT
		{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename C, typename A, typename E, bool bM, bool bU>
	inline typename rbtree<K, V, C, A, E, bM, bU>::const_reverse_iterator
	rbtree<K, V, C, A, E, bM, bU>::crbegin() const EA_NOEXCEPT
		{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename C, typename A, typename E, bool bM, bool bU>
	inline typename rbtree<K, V, C, A, E, bM, bU>::reverse_iterator
	rbtree<K, V, C, A, E, bM, bU>::rend() EA_NOEXCEPT
		{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename C, typename A, typename E, bool bM, bool bU>
	inline typename rbtree<K, V, C, A, E, bM, bU>::const_reverse_iterator
	rbtree<K, V, C, A, E, bM, bU>::rend() const EA_NOEXCEPT
		{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename C, typename A, typename E, bool bM, bool bU>
	inline typename rbtree<K, V, C, A, E, bM, bU>::const_reverse_iterator
	rbtree<K, V, C, A, E, bM, bU>::crend() const EA_NOEXCEPT
		{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename C, typename A, typename E, bool bM, bool bU>
	inline typename rbtree<K, V, C, A, E, bM, bU>::this_type&
	rbtree<K, V, C, A, E, bM, bU>::operator=(const this_type& x)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename K, typename V, typename C, typename A, typename E, bool bM, bool bU>
	inline typename rbtree<K, V, C, A, E, bM, bU>::this_type&
	rbtree<K, V, C, A, E, bM, bU>::operator=(this_type&& x)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename K, typename V, typename C, typename A, typename E, bool bM, bool bU>
	inline typename rbtree<K, V, C, A, E, bM, bU>::this_type&
	rbtree<K, V, C, A, E, bM, bU>::operator=(std::initializer_list<value_type> ilist)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename C, typename A, typename E, bool bM, bool bU>
	void rbtree<K, V, C, A, E, bM, bU>::swap(this_type& x)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename C, typename A, typename E, bool bM, bool bU>
	template <class... Args>
	inline typename rbtree<K, V, C, A, E, bM, bU>::insert_return_type // map/set::insert return a pair, multimap/multiset::iterator return an iterator.
	rbtree<K, V, C, A, E, bM, bU>::emplace(Args&&... args)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename K, typename V, typename C, typename A, typename E, bool bM, bool bU>
	template <class... Args> 
	typename rbtree<K, V, C, A, E, bM, bU>::iterator
	rbtree<K, V, C, A, E, bM, bU>::emplace_hint(const_iterator position, Args&&... args)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename C, typename A, typename E, bool bM, bool bU>
	inline typename rbtree<K, V, C, A, E, bM, bU>::iterator 
	rbtree<K, V, C, A, E, bM, bU>::insert(const_iterator position, value_type&& value)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename C, typename A, typename E, bool bM, bool bU>
	inline typename rbtree<K, V, C, A, E, bM, bU>::insert_return_type // map/set::insert return a pair, multimap/multiset::iterator return an iterator.
	rbtree<K, V, C, A, E, bM, bU>::insert(const value_type& value)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename C, typename A, typename E, bool bM, bool bU>
	inline typename rbtree<K, V, C, A, E, bM, bU>::insert_return_type // map/set::insert return a pair, multimap/multiset::iterator return an iterator.
	rbtree<K, V, C, A, E, bM, bU>::insert(value_type&& value)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename C, typename A, typename E, bool bM, bool bU>
	typename rbtree<K, V, C, A, E, bM, bU>::iterator
	rbtree<K, V, C, A, E, bM, bU>::insert(const_iterator position, const value_type& value)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename K, typename V, typename C, typename A, typename E, bool bM, bool bU>
	template <typename KX, typename M>
	eastl::pair<typename rbtree<K, V, C, A, E, bM, bU>::iterator, bool>
	rbtree<K, V, C, A, E, bM, bU>::DoInsertOrAssign(KX&& k, M&& obj)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename K, typename V, typename C, typename A, typename E, bool bM, bool bU>
	template <typename KX, typename M>
	typename rbtree<K, V, C, A, E, bM, bU>::iterator
	rbtree<K, V, C, A, E, bM, bU>::DoInsertOrAssign(const_iterator hint, KX&& k, M&& obj)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename K, typename V, typename C, typename A, typename E, bool bM, bool bU>
	template <typename KX>
	rbtree_node_base*
	rbtree<K, V, C, A, E, bM, bU>::DoGetKeyInsertionPositionUniqueKeys(bool& canInsert, const KX& key)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename C, typename A, typename E, bool bM, bool bU>
	rbtree_node_base*
	rbtree<K, V, C, A, E, bM, bU>::DoGetKeyInsertionPositionNonuniqueKeys(const key_type& key)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename C, typename A, typename E, bool bM, bool bU>
	eastl::pair<typename rbtree<K, V, C, A, E, bM, bU>::iterator, bool> 
	rbtree<K, V, C, A, E, bM, bU>::DoInsertValue(true_type, value_type&& value)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename C, typename A, typename E, bool bM, bool bU>
	typename rbtree<K, V, C, A, E, bM, bU>::iterator 
	rbtree<K, V, C, A, E, bM, bU>::DoInsertValue(false_type, value_type&& value)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename C, typename A, typename E, bool bM, bool bU>
	template <class... Args>
	eastl::pair<typename rbtree<K, V, C, A, E, bM, bU>::iterator, bool>
	rbtree<K, V, C, A, E, bM, bU>::DoInsertValue(true_type, Args&&... args) // true_type means keys are unique.
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename C, typename A, typename E, bool bM, bool bU>
	template <class... Args>
	typename rbtree<K, V, C, A, E, bM, bU>::iterator
	rbtree<K, V, C, A, E, bM, bU>::DoInsertValue(false_type, Args&&... args) // false_type means keys are not unique.
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename C, typename A, typename E, bool bM, bool bU>
	template <class... Args>
	typename rbtree<K, V, C, A, E, bM, bU>::iterator
	rbtree<K, V, C, A, E, bM, bU>::DoInsertValueImpl(rbtree_node_base* pNodeParent, bool bForceToLeft, const key_type& key, Args&&... args)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	
	template <typename K, typename V, typename C, typename A, typename E, bool bM, bool bU>
	typename rbtree<K, V, C, A, E, bM, bU>::iterator
	rbtree<K, V, C, A, E, bM, bU>::DoInsertValueImpl(rbtree_node_base* pNodeParent, bool bForceToLeft, const key_type& key, node_type* pNodeNew)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename C, typename A, typename E, bool bM, bool bU>
	eastl::pair<typename rbtree<K, V, C, A, E, bM, bU>::iterator, bool>
	rbtree<K, V, C, A, E, bM, bU>::DoInsertKey(true_type, const key_type& key) // true_type means keys are unique.
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename C, typename A, typename E, bool bM, bool bU>
	typename rbtree<K, V, C, A, E, bM, bU>::iterator
	rbtree<K, V, C, A, E, bM, bU>::DoInsertKey(false_type, const key_type& key) // false_type means keys are not unique.
	{
    __builtin_trap() /* STUB: not implemented */;
}



	template <typename K, typename V, typename C, typename A, typename E, bool bM, bool bU>
	template <typename KX>
	rbtree_node_base*
	rbtree<K, V, C, A, E, bM, bU>::DoGetKeyInsertionPositionUniqueKeysHint(const_iterator position, bool& bForceToLeft, const KX& key)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename C, typename A, typename E, bool bM, bool bU>
	rbtree_node_base*
	rbtree<K, V, C, A, E, bM, bU>::DoGetKeyInsertionPositionNonuniqueKeysHint(const_iterator position, bool& bForceToLeft, const key_type& key)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename K, typename V, typename C, typename A, typename E, bool bM, bool bU>
	template <class... Args>
	typename rbtree<K, V, C, A, E, bM, bU>::iterator
	rbtree<K, V, C, A, E, bM, bU>::DoInsertValueHint(true_type, const_iterator position, Args&&... args) // true_type means keys are unique.
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename C, typename A, typename E, bool bM, bool bU>
	template <class... Args>
	typename rbtree<K, V, C, A, E, bM, bU>::iterator
	rbtree<K, V, C, A, E, bM, bU>::DoInsertValueHint(false_type, const_iterator position, Args&&... args) // false_type means keys are not unique.
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename C, typename A, typename E, bool bM, bool bU>
	typename rbtree<K, V, C, A, E, bM, bU>::iterator
	rbtree<K, V, C, A, E, bM, bU>::DoInsertValueHint(true_type, const_iterator position, value_type&& value) // true_type means keys are unique.
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename C, typename A, typename E, bool bM, bool bU>
	typename rbtree<K, V, C, A, E, bM, bU>::iterator
	rbtree<K, V, C, A, E, bM, bU>::DoInsertValueHint(false_type, const_iterator position, value_type&& value) // false_type means keys are not unique.
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename C, typename A, typename E, bool bM, bool bU>
	typename rbtree<K, V, C, A, E, bM, bU>::iterator
	rbtree<K, V, C, A, E, bM, bU>::DoInsertKey(true_type, const_iterator position, const key_type& key) // true_type means keys are unique.
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename C, typename A, typename E, bool bM, bool bU>
	typename rbtree<K, V, C, A, E, bM, bU>::iterator
	rbtree<K, V, C, A, E, bM, bU>::DoInsertKey(false_type, const_iterator position, const key_type& key) // false_type means keys are not unique.
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename C, typename A, typename E, bool bM, bool bU>
	typename rbtree<K, V, C, A, E, bM, bU>::iterator
	rbtree<K, V, C, A, E, bM, bU>::DoInsertKeyImpl(rbtree_node_base* pNodeParent, bool bForceToLeft, const key_type& key)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename C, typename A, typename E, bool bM, bool bU>
	void rbtree<K, V, C, A, E, bM, bU>::insert(std::initializer_list<value_type> ilist)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename C, typename A, typename E, bool bM, bool bU>
	template <typename InputIterator>
	void rbtree<K, V, C, A, E, bM, bU>::insert(InputIterator first, InputIterator last)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename C, typename A, typename E, bool bM, bool bU>
	inline void rbtree<K, V, C, A, E, bM, bU>::clear()
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename C, typename A, typename E, bool bM, bool bU>
	inline void rbtree<K, V, C, A, E, bM, bU>::reset_lose_memory()
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename C, typename A, typename E, bool bM, bool bU>
	inline typename rbtree<K, V, C, A, E, bM, bU>::iterator
	rbtree<K, V, C, A, E, bM, bU>::erase(const_iterator position)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename C, typename A, typename E, bool bM, bool bU>
	typename rbtree<K, V, C, A, E, bM, bU>::iterator
	rbtree<K, V, C, A, E, bM, bU>::erase(const_iterator first, const_iterator last)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename C, typename A, typename E, bool bM, bool bU>
	inline typename rbtree<K, V, C, A, E, bM, bU>::reverse_iterator
	rbtree<K, V, C, A, E, bM, bU>::erase(const_reverse_iterator position)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename C, typename A, typename E, bool bM, bool bU>
	typename rbtree<K, V, C, A, E, bM, bU>::reverse_iterator
	rbtree<K, V, C, A, E, bM, bU>::erase(const_reverse_iterator first, const_reverse_iterator last)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename C, typename A, typename E, bool bM, bool bU>
	inline void rbtree<K, V, C, A, E, bM, bU>::erase(const key_type* first, const key_type* last)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename K, typename V, typename C, typename A, typename E, bool bM, bool bU>
	template <typename KX>
	typename rbtree<K, V, C, A, E, bM, bU>::iterator
	rbtree<K, V, C, A, E, bM, bU>::DoFind(const KX& key)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename K, typename V, typename C, typename A, typename E, bool bM, bool bU>
	template <typename KX>
	inline typename rbtree<K, V, C, A, E, bM, bU>::const_iterator
	rbtree<K, V, C, A, E, bM, bU>::DoFind(const KX& key) const
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename K, typename V, typename C, typename A, typename E, bool bM, bool bU>
	template <typename U, typename Compare2>
	typename rbtree<K, V, C, A, E, bM, bU>::iterator
	rbtree<K, V, C, A, E, bM, bU>::find_as(const U& u, Compare2 compare2)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename C, typename A, typename E, bool bM, bool bU>
	template <typename U, typename Compare2>
	inline typename rbtree<K, V, C, A, E, bM, bU>::const_iterator
	rbtree<K, V, C, A, E, bM, bU>::find_as(const U& u, Compare2 compare2) const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename C, typename A, typename E, bool bM, bool bU>
	template <typename KX>
	typename rbtree<K, V, C, A, E, bM, bU>::iterator
	rbtree<K, V, C, A, E, bM, bU>::DoLowerBound(const KX& key)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename C, typename A, typename E, bool bM, bool bU>
	template <typename KX>
	inline typename rbtree<K, V, C, A, E, bM, bU>::const_iterator
	rbtree<K, V, C, A, E, bM, bU>::DoLowerBound(const KX& key) const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename C, typename A, typename E, bool bM, bool bU>
	template <typename KX>
	typename rbtree<K, V, C, A, E, bM, bU>::iterator
	rbtree<K, V, C, A, E, bM, bU>::DoUpperBound(const KX& key)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename C, typename A, typename E, bool bM, bool bU>
	template <typename KX>
	inline typename rbtree<K, V, C, A, E, bM, bU>::const_iterator
	rbtree<K, V, C, A, E, bM, bU>::DoUpperBound(const KX& key) const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	// To do: Move this validate function entirely to a template-less implementation.
	template <typename K, typename V, typename C, typename A, typename E, bool bM, bool bU>
	bool rbtree<K, V, C, A, E, bM, bU>::validate() const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename C, typename A, typename E, bool bM, bool bU>
	inline int rbtree<K, V, C, A, E, bM, bU>::validate_iterator(const_iterator i) const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename C, typename A, typename E, bool bM, bool bU>
	inline typename rbtree<K, V, C, A, E, bM, bU>::node_type*
	rbtree<K, V, C, A, E, bM, bU>::DoAllocateNode()
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename C, typename A, typename E, bool bM, bool bU>
	inline void rbtree<K, V, C, A, E, bM, bU>::DoFreeNode(node_type* pNode)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename C, typename A, typename E, bool bM, bool bU>
	typename rbtree<K, V, C, A, E, bM, bU>::node_type*
	rbtree<K, V, C, A, E, bM, bU>::DoCreateNodeFromKey(const key_type& key)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename C, typename A, typename E, bool bM, bool bU>
	typename rbtree<K, V, C, A, E, bM, bU>::node_type*
	rbtree<K, V, C, A, E, bM, bU>::DoCreateNode(const value_type& value)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename C, typename A, typename E, bool bM, bool bU>
	typename rbtree<K, V, C, A, E, bM, bU>::node_type*
	rbtree<K, V, C, A, E, bM, bU>::DoCreateNode(value_type&& value)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename C, typename A, typename E, bool bM, bool bU>
	template<class... Args>
	typename rbtree<K, V, C, A, E, bM, bU>::node_type*
	rbtree<K, V, C, A, E, bM, bU>::DoCreateNode(Args&&... args)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename C, typename A, typename E, bool bM, bool bU>
	typename rbtree<K, V, C, A, E, bM, bU>::node_type*
	rbtree<K, V, C, A, E, bM, bU>::DoCreateNode(const node_type* pNodeSource, rbtree_node_base* pNodeParent)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename C, typename A, typename E, bool bM, bool bU>
	rbtree_node_base*
	rbtree<K, V, C, A, E, bM, bU>::DoCopySubtree(const node_type* pNodeSource, rbtree_node_base* pNodeDest)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename C, typename A, typename E, bool bM, bool bU>
	void rbtree<K, V, C, A, E, bM, bU>::DoNukeSubtree(rbtree_node_base* pNode)
	{
    __builtin_trap() /* STUB: not implemented */;
}



	///////////////////////////////////////////////////////////////////////
	// global operators
	///////////////////////////////////////////////////////////////////////

	template <typename K, typename V, typename A, typename C, typename E, bool bM, bool bU>
	inline bool operator==(const rbtree<K, V, C, A, E, bM, bU>& a, const rbtree<K, V, C, A, E, bM, bU>& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	// Note that in operator< we do comparisons based on the tree value_type with operator<() of the
	// value_type instead of the tree's Compare function. For set/multiset, the value_type is T, while
	// for map/multimap the value_type is a pair<Key, T>. operator< for pair can be seen by looking
	// utility.h, but it basically is uses the operator< for pair.first and pair.second. The C++ standard
	// appears to require this behaviour, whether intentionally or not. If anything, a good reason to do
	// this is for consistency. A map and a vector that contain the same items should compare the same.
	template <typename K, typename V, typename A, typename C, typename E, bool bM, bool bU>
	inline bool operator<(const rbtree<K, V, C, A, E, bM, bU>& a, const rbtree<K, V, C, A, E, bM, bU>& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename A, typename C, typename E, bool bM, bool bU>
	inline bool operator!=(const rbtree<K, V, C, A, E, bM, bU>& a, const rbtree<K, V, C, A, E, bM, bU>& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename A, typename C, typename E, bool bM, bool bU>
	inline bool operator>(const rbtree<K, V, C, A, E, bM, bU>& a, const rbtree<K, V, C, A, E, bM, bU>& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename A, typename C, typename E, bool bM, bool bU>
	inline bool operator<=(const rbtree<K, V, C, A, E, bM, bU>& a, const rbtree<K, V, C, A, E, bM, bU>& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename A, typename C, typename E, bool bM, bool bU>
	inline bool operator>=(const rbtree<K, V, C, A, E, bM, bU>& a, const rbtree<K, V, C, A, E, bM, bU>& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename A, typename C, typename E, bool bM, bool bU>
	inline void swap(rbtree<K, V, C, A, E, bM, bU>& a, rbtree<K, V, C, A, E, bM, bU>& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}


} // namespace eastl


EA_RESTORE_VC_WARNING();


#endif // Header include guard
