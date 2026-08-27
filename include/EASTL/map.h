#include <cstdlib>
///////////////////////////////////////////////////////////////////////////////
// Copyright (c) Electronic Arts Inc. All rights reserved.
//////////////////////////////////////////////////////////////////////////////


#ifndef EASTL_MAP_H
#define EASTL_MAP_H


#include <EASTL/internal/config.h>
#include <EASTL/internal/red_black_tree.h>
#include <EASTL/functional.h>
#include <EASTL/utility.h>
#if EASTL_EXCEPTIONS_ENABLED
#include <stdexcept>
#endif

#if defined(EA_PRAGMA_ONCE_SUPPORTED)
	#pragma once // Some compilers (e.g. VC++) benefit significantly from using this. We've measured 3-4% build speed improvements in apps as a result.
#endif



namespace eastl
{

	/// EASTL_MAP_DEFAULT_NAME
	///
	/// Defines a default container name in the absence of a user-provided name.
	///
	#ifndef EASTL_MAP_DEFAULT_NAME
		#define EASTL_MAP_DEFAULT_NAME EASTL_DEFAULT_NAME_PREFIX " map" // Unless the user overrides something, this is "EASTL map".
	#endif


	/// EASTL_MULTIMAP_DEFAULT_NAME
	///
	/// Defines a default container name in the absence of a user-provided name.
	///
	#ifndef EASTL_MULTIMAP_DEFAULT_NAME
		#define EASTL_MULTIMAP_DEFAULT_NAME EASTL_DEFAULT_NAME_PREFIX " multimap" // Unless the user overrides something, this is "EASTL multimap".
	#endif


	/// EASTL_MAP_DEFAULT_ALLOCATOR
	///
	#ifndef EASTL_MAP_DEFAULT_ALLOCATOR
		#define EASTL_MAP_DEFAULT_ALLOCATOR allocator_type(EASTL_MAP_DEFAULT_NAME)
	#endif

	/// EASTL_MULTIMAP_DEFAULT_ALLOCATOR
	///
	#ifndef EASTL_MULTIMAP_DEFAULT_ALLOCATOR
		#define EASTL_MULTIMAP_DEFAULT_ALLOCATOR allocator_type(EASTL_MULTIMAP_DEFAULT_NAME)
	#endif



	/// map
	///
	/// Implements a canonical map. 
	///
	/// The large majority of the implementation of this class is found in the rbtree
	/// base class. We control the behaviour of rbtree via template parameters.
	///
	/// Pool allocation
	/// If you want to make a custom memory pool for a map container, your pool 
	/// needs to contain items of type map::node_type. So if you have a memory
	/// pool that has a constructor that takes the size of pool items and the
	/// count of pool items, you would do this (assuming that MemoryPool implements
	/// the Allocator interface):
	///     typedef map<Widget, int, less<Widget>, MemoryPool> WidgetMap;  // Delare your WidgetMap type.
	///     MemoryPool myPool(sizeof(WidgetMap::node_type), 100);          // Make a pool of 100 Widget nodes.
	///     WidgetMap myMap(&myPool);                                      // Create a map that uses the pool.
	///
	template <typename Key, typename T, typename Compare = eastl::less<Key>, typename Allocator = EASTLAllocatorType>
	class map
		: public rbtree<Key, eastl::pair<const Key, T>, Compare, Allocator, eastl::use_first<eastl::pair<const Key, T> >, true, true>
	{
	public:
		typedef rbtree<Key, eastl::pair<const Key, T>, Compare, Allocator,
						eastl::use_first<eastl::pair<const Key, T> >, true, true>   base_type;
		typedef map<Key, T, Compare, Allocator>                                     this_type;
		typedef typename base_type::size_type                                       size_type;
		typedef typename base_type::key_type                                        key_type;
		typedef T                                                                   mapped_type;
		typedef typename base_type::value_type                                      value_type;
		typedef typename base_type::node_type                                       node_type;
		typedef typename base_type::iterator                                        iterator;
		typedef typename base_type::const_iterator                                  const_iterator;
		typedef typename base_type::allocator_type                                  allocator_type;
		typedef typename base_type::insert_return_type                              insert_return_type;
		typedef typename base_type::extract_key                                     extract_key;
		// Other types are inherited from the base class.

		using base_type::begin;
		using base_type::end;
		using base_type::find;
		using base_type::lower_bound;
		using base_type::upper_bound;
		using base_type::insert;
		using base_type::erase;

		static_assert(!is_const<value_type>::value, "map<T> value_type must be non-const.");
		static_assert(!is_volatile<value_type>::value, "map<T> value_type must be non-volatile.");

	protected:
		using base_type::compare;
		using base_type::get_compare;

	public:
		class value_compare 
		{
		protected:
			friend class map;
			Compare compare;
			value_compare(Compare c) : compare(c) {
    __builtin_trap() /* STUB: not implemented */;
}

		public:
			bool operator()(const value_type& x, const value_type& y) const 
				{
    __builtin_trap() /* STUB: not implemented */;
}
		};

	public:
		map(const allocator_type& allocator = EASTL_MAP_DEFAULT_ALLOCATOR);
		map(const Compare& compare, const allocator_type& allocator = EASTL_MAP_DEFAULT_ALLOCATOR);
		map(const this_type& x);
		map(this_type&& x);
		map(this_type&& x, const allocator_type& allocator);
		map(std::initializer_list<value_type> ilist, const Compare& compare = Compare(), const allocator_type& allocator = EASTL_MAP_DEFAULT_ALLOCATOR);
		map(std::initializer_list<value_type> ilist, const allocator_type& allocator);

		template <typename Iterator>
		map(Iterator itBegin, Iterator itEnd); // allocator arg removed because VC7.1 fails on the default arg. To consider: Make a second version of this function without a default arg.

		// missing constructors, to implement:
		// 
		// map(const this_type& x, const allocator_type& allocator);
		// 
		// template <typename InputIterator>
		// map(InputIterator first, InputIterator last, const Compare& comp = Compare(), const Allocator& alloc = Allocator());
		//
		// template <typename InputIterator>
		// map(InputIterator first, InputIterator last, const Allocator& alloc);

		this_type& operator=(const this_type& x) {
    __builtin_trap() /* STUB: not implemented */;
}
		this_type& operator=(std::initializer_list<value_type> ilist) {
    __builtin_trap() /* STUB: not implemented */;
}
		this_type& operator=(this_type&& x) {
    __builtin_trap() /* STUB: not implemented */;
}

	public:
		/// This is an extension to the C++ standard. We insert a default-constructed 
		/// element with the given key. The reason for this is that we can avoid the 
		/// potentially expensive operation of creating and/or copying a mapped_type
		/// object on the stack. Note that C++11 move insertions and variadic emplace
		/// support make this extension mostly no longer necessary.
		insert_return_type insert(const Key& key);

		// Standard conversion overload to avoid the overhead of mismatched 'pair<const Key, Value>' types.
		template <typename P, eastl::enable_if_t<eastl::is_constructible_v<value_type, P&&>, bool> = true>
		insert_return_type insert(P&& otherValue) {
    __builtin_trap() /* STUB: not implemented */;
}

		value_compare value_comp() const;

		size_type erase(const Key& key) {
    __builtin_trap() /* STUB: not implemented */;
}

		template<typename KX, typename Cmp = Compare,
			eastl::enable_if_t<!eastl::is_convertible_v<KX&&, iterator> && !eastl::is_convertible_v<KX&&, const_iterator>
			&& eastl::detail::is_transparent_comparison_v<Cmp>, bool> = true>
		size_type erase(KX&& key) {
    __builtin_trap() /* STUB: not implemented */;
}

		size_type count(const Key& key) const;

		template<typename KX, typename Cmp = Compare, eastl::enable_if_t<eastl::detail::is_transparent_comparison_v<Cmp>, bool> = true>
		size_type count(const KX& key) const {
    __builtin_trap() /* STUB: not implemented */;
}

		eastl::pair<iterator, iterator>             equal_range(const Key& key) {
    __builtin_trap() /* STUB: not implemented */;
}
		eastl::pair<const_iterator, const_iterator> equal_range(const Key& key) const {
    __builtin_trap() /* STUB: not implemented */;
}

		template<typename KX, typename Cmp = Compare, eastl::enable_if_t<eastl::detail::is_transparent_comparison_v<Cmp>, bool> = true>
		eastl::pair<iterator, iterator>             equal_range(const KX& key) {
    __builtin_trap() /* STUB: not implemented */;
}
		template<typename KX, typename Cmp = Compare, eastl::enable_if_t<eastl::detail::is_transparent_comparison_v<Cmp>, bool> = true>
		eastl::pair<const_iterator, const_iterator> equal_range(const KX& key) const {
    __builtin_trap() /* STUB: not implemented */;
}

		T& operator[](const Key& key); // Of map, multimap, set, and multimap, only map has operator[].
		T& operator[](Key&& key);

		template<typename KX, typename Cmp = Compare, eastl::enable_if_t<eastl::detail::is_transparent_comparison_v<Cmp>, bool> = true>
		T& operator[](KX&& key) {
    __builtin_trap() /* STUB: not implemented */;
}

		T& at(const Key& key) {
    __builtin_trap() /* STUB: not implemented */;
}
		const T& at(const Key& key) const {
    __builtin_trap() /* STUB: not implemented */;
}

		template<typename KX, typename Cmp = Compare, eastl::enable_if_t<eastl::detail::is_transparent_comparison_v<Cmp>, bool> = true>
		T& at(const KX& key) {
    __builtin_trap() /* STUB: not implemented */;
}
		template<typename KX, typename Cmp = Compare, eastl::enable_if_t<eastl::detail::is_transparent_comparison_v<Cmp>, bool> = true>
		const T& at(const KX& key) const {
    __builtin_trap() /* STUB: not implemented */;
}

		template <class... Args> eastl::pair<iterator, bool> try_emplace(const key_type& k, Args&&... args);
		template <class... Args> eastl::pair<iterator, bool> try_emplace(key_type&& k, Args&&... args);
		template <class KX, class... Args, typename Cmp = Compare,
			eastl::enable_if_t<!eastl::is_convertible_v<KX&&, const_iterator> && !eastl::is_convertible_v<KX&&, iterator>
			&& eastl::detail::is_transparent_comparison_v<Cmp>, bool> = true>
		eastl::pair<iterator, bool> try_emplace(KX&& k, Args&&... args) {
    __builtin_trap() /* STUB: not implemented */;
}
		template <class... Args> iterator                    try_emplace(const_iterator position, const key_type& k, Args&&... args);
		template <class... Args> iterator                    try_emplace(const_iterator position, key_type&& k, Args&&... args);
		template <class KX, class... Args, typename Cmp = Compare, eastl::enable_if_t<eastl::detail::is_transparent_comparison_v<Cmp>, bool> = true>
		iterator                    try_emplace(const_iterator hint, KX&& k, Args&&... args) {
    __builtin_trap() /* STUB: not implemented */;
}

	private:
		template<typename KX>
		size_type DoErase(KX&& key);

		template <class KFwd, class... Args>
		eastl::pair<iterator, bool> try_emplace_forward(KFwd&& k, Args&&... args);

		template <class KFwd, class... Args>
		iterator try_emplace_forward(const_iterator hint, KFwd&& key, Args&&... args);

		template<typename KX>
		eastl::pair<iterator, iterator>             DoEqualRange(const KX& key);
		template<typename KX>
		eastl::pair<const_iterator, const_iterator> DoEqualRange(const KX& key) const;

		template<typename KX>
		T& DoAt(const KX& key);
		template<typename KX>
		const T& DoAt(const KX& key) const;
	}; // map






	/// multimap
	///
	/// Implements a canonical multimap.
	///
	/// The large majority of the implementation of this class is found in the rbtree
	/// base class. We control the behaviour of rbtree via template parameters.
	///
	/// Pool allocation
	/// If you want to make a custom memory pool for a multimap container, your pool 
	/// needs to contain items of type multimap::node_type. So if you have a memory
	/// pool that has a constructor that takes the size of pool items and the
	/// count of pool items, you would do this (assuming that MemoryPool implements
	/// the Allocator interface):
	///     typedef multimap<Widget, int, less<Widget>, MemoryPool> WidgetMap;  // Delare your WidgetMap type.
	///     MemoryPool myPool(sizeof(WidgetMap::node_type), 100);               // Make a pool of 100 Widget nodes.
	///     WidgetMap myMap(&myPool);                                           // Create a map that uses the pool.
	///
	template <typename Key, typename T, typename Compare = eastl::less<Key>, typename Allocator = EASTLAllocatorType>
	class multimap
		: public rbtree<Key, eastl::pair<const Key, T>, Compare, Allocator, eastl::use_first<eastl::pair<const Key, T> >, true, false>
	{
	public:
		typedef rbtree<Key, eastl::pair<const Key, T>, Compare, Allocator, 
						eastl::use_first<eastl::pair<const Key, T> >, true, false>  base_type;
		typedef multimap<Key, T, Compare, Allocator>                                this_type;
		typedef typename base_type::size_type                                       size_type;
		typedef typename base_type::key_type                                        key_type;
		typedef T                                                                   mapped_type;
		typedef typename base_type::value_type                                      value_type;
		typedef typename base_type::node_type                                       node_type;
		typedef typename base_type::iterator                                        iterator;
		typedef typename base_type::const_iterator                                  const_iterator;
		typedef typename base_type::allocator_type                                  allocator_type;
		typedef typename base_type::insert_return_type                              insert_return_type;
		typedef typename base_type::extract_key                                     extract_key;
		// Other types are inherited from the base class.

		using base_type::begin;
		using base_type::end;
		using base_type::find;
		using base_type::lower_bound;
		using base_type::upper_bound;
		using base_type::insert;
		using base_type::erase;

		static_assert(!is_const<value_type>::value, "multimap<T> value_type must be non-const.");
		static_assert(!is_volatile<value_type>::value, "multimap<T> value_type must be non-volatile.");

	protected:
		using base_type::compare;
		using base_type::get_compare;

	public:
		class value_compare 
		{
		protected:
			friend class multimap;
			Compare compare;
			value_compare(Compare c) : compare(c) {
    __builtin_trap() /* STUB: not implemented */;
}

		public:
			bool operator()(const value_type& x, const value_type& y) const 
				{
    __builtin_trap() /* STUB: not implemented */;
}
		};

	public:
		multimap(const allocator_type& allocator = EASTL_MULTIMAP_DEFAULT_ALLOCATOR);
		multimap(const Compare& compare, const allocator_type& allocator = EASTL_MULTIMAP_DEFAULT_ALLOCATOR);
		multimap(const this_type& x);
		multimap(this_type&& x);
		multimap(this_type&& x, const allocator_type& allocator);
		multimap(std::initializer_list<value_type> ilist, const Compare& compare = Compare(), const allocator_type& allocator = EASTL_MULTIMAP_DEFAULT_ALLOCATOR);
		multimap(std::initializer_list<value_type> ilist, const allocator_type& allocator);

		template <typename Iterator>
		multimap(Iterator itBegin, Iterator itEnd); // allocator arg removed because VC7.1 fails on the default arg. To consider: Make a second version of this function without a default arg.

		// missing constructors, to implement:
		// 
		// multimap(const this_type& x, const allocator_type& allocator);
		// 
		// template <typename InputIterator>
		// multimap(InputIterator first, InputIterator last, const Compare& comp = Compare(), const Allocator& alloc = Allocator());
		//
		// template <typename InputIterator>
		// multimap(InputIterator first, InputIterator last, const Allocator& alloc);

		this_type& operator=(const this_type& x) {
    __builtin_trap() /* STUB: not implemented */;
}
		this_type& operator=(std::initializer_list<value_type> ilist) {
    __builtin_trap() /* STUB: not implemented */;
}
		this_type& operator=(this_type&& x) {
    __builtin_trap() /* STUB: not implemented */;
}

	public:
		/// This is an extension to the C++ standard. We insert a default-constructed 
		/// element with the given key. The reason for this is that we can avoid the 
		/// potentially expensive operation of creating and/or copying a mapped_type
		/// object on the stack. Note that C++11 move insertions and variadic emplace
		/// support make this extension mostly no longer necessary.
		insert_return_type insert(const Key& key);

		// Standard conversion overload to avoid the overhead of mismatched 'pair<const Key, Value>' types.
		template <typename P, eastl::enable_if_t<eastl::is_constructible_v<value_type, P&&>, bool> = true>
		insert_return_type insert(P&& otherValue) {
    __builtin_trap() /* STUB: not implemented */;
}

		value_compare value_comp() const;

		size_type erase(const Key& key) {
    __builtin_trap() /* STUB: not implemented */;
}
		
		template<typename KX, typename Cmp = Compare,
			eastl::enable_if_t<!eastl::is_convertible_v<KX&&, iterator> && !eastl::is_convertible_v<KX&&, const_iterator>
			&& eastl::detail::is_transparent_comparison_v<Cmp>, bool> = true>
		size_type erase(KX&& key) {
    __builtin_trap() /* STUB: not implemented */;
}

		size_type count(const Key& key) const {
    __builtin_trap() /* STUB: not implemented */;
}

		template<typename KX, typename Cmp = Compare, eastl::enable_if_t<eastl::detail::is_transparent_comparison_v<Cmp>, bool> = true>
		size_type count(const KX& key) const {
    __builtin_trap() /* STUB: not implemented */;
}

		eastl::pair<iterator, iterator>             equal_range(const Key& key) {
    __builtin_trap() /* STUB: not implemented */;
}
		eastl::pair<const_iterator, const_iterator> equal_range(const Key& key) const {
    __builtin_trap() /* STUB: not implemented */;
}

		template<typename KX, typename Cmp = Compare, eastl::enable_if_t<eastl::detail::is_transparent_comparison_v<Cmp>, bool> = true>
		eastl::pair<iterator, iterator>             equal_range(const KX& key) {
    __builtin_trap() /* STUB: not implemented */;
}
		template<typename KX, typename Cmp = Compare, eastl::enable_if_t<eastl::detail::is_transparent_comparison_v<Cmp>, bool> = true>
		eastl::pair<const_iterator, const_iterator> equal_range(const KX& key) const {
    __builtin_trap() /* STUB: not implemented */;
}

		/// equal_range_small
		/// This is a special version of equal_range which is optimized for the 
		/// case of there being few or no duplicated keys in the tree.
		eastl::pair<iterator, iterator>             equal_range_small(const Key& key) {
    __builtin_trap() /* STUB: not implemented */;
}
		eastl::pair<const_iterator, const_iterator> equal_range_small(const Key& key) const {
    __builtin_trap() /* STUB: not implemented */;
}

		template<typename KX, typename Cmp = Compare, eastl::enable_if_t<eastl::detail::is_transparent_comparison_v<Cmp>, bool> = true>
		eastl::pair<iterator, iterator>             equal_range_small(const KX& key) {
    __builtin_trap() /* STUB: not implemented */;
}
		template<typename KX, typename Cmp = Compare, eastl::enable_if_t<eastl::detail::is_transparent_comparison_v<Cmp>, bool> = true>
		eastl::pair<const_iterator, const_iterator> equal_range_small(const KX& key) const {
    __builtin_trap() /* STUB: not implemented */;
}

	private:
		// these base member functions are not included in multimaps
		using base_type::insert_or_assign;

		template<typename KX>
		size_type DoErase(KX&& key);

		template<typename KX>
		size_type DoCount(const KX& key) const;

		template<typename KX>
		eastl::pair<iterator, iterator>             DoEqualRange(const KX& key);
		template<typename KX>
		eastl::pair<const_iterator, const_iterator> DoEqualRange(const KX& key) const;

		template<typename KX>
		eastl::pair<iterator, iterator>             DoEqualRangeSmall(const KX& key);
		template<typename KX>
		eastl::pair<const_iterator, const_iterator> DoEqualRangeSmall(const KX& key) const;
	}; // multimap





	///////////////////////////////////////////////////////////////////////
	// map
	///////////////////////////////////////////////////////////////////////

	template <typename Key, typename T, typename Compare, typename Allocator>
	inline map<Key, T, Compare, Allocator>::map(const allocator_type& allocator)
		: base_type(allocator)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Key, typename T, typename Compare, typename Allocator>
	inline map<Key, T, Compare, Allocator>::map(const Compare& compare, const allocator_type& allocator)
		: base_type(compare, allocator)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Key, typename T, typename Compare, typename Allocator>
	inline map<Key, T, Compare, Allocator>::map(const this_type& x)
		: base_type(x)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Key, typename T, typename Compare, typename Allocator>
	inline map<Key, T, Compare, Allocator>::map(this_type&& x)
		: base_type(eastl::move(x))
	{
	}

	template <typename Key, typename T, typename Compare, typename Allocator>
	inline map<Key, T, Compare, Allocator>::map(this_type&& x, const allocator_type& allocator)
		: base_type(eastl::move(x), allocator)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Key, typename T, typename Compare, typename Allocator>
	inline map<Key, T, Compare, Allocator>::map(std::initializer_list<value_type> ilist, const Compare& compare, const allocator_type& allocator)
		: base_type(ilist.begin(), ilist.end(), compare, allocator)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Key, typename T, typename Compare, typename Allocator>
	inline map<Key, T, Compare, Allocator>::map(std::initializer_list<value_type> ilist, const allocator_type& allocator)
		: base_type(ilist.begin(), ilist.end(), Compare(), allocator)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Key, typename T, typename Compare, typename Allocator>
	template <typename Iterator>
	inline map<Key, T, Compare, Allocator>::map(Iterator itBegin, Iterator itEnd)
		: base_type(itBegin, itEnd, Compare(), EASTL_MAP_DEFAULT_ALLOCATOR)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Key, typename T, typename Compare, typename Allocator>
	inline typename map<Key, T, Compare, Allocator>::insert_return_type
	map<Key, T, Compare, Allocator>::insert(const Key& key)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Key, typename T, typename Compare, typename Allocator>
	inline typename map<Key, T, Compare, Allocator>::value_compare 
	map<Key, T, Compare, Allocator>::value_comp() const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Key, typename T, typename Compare, typename Allocator>
	template <typename KX>
	inline typename map<Key, T, Compare, Allocator>::size_type
	map<Key, T, Compare, Allocator>::DoErase(KX&& key)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Key, typename T, typename Compare, typename Allocator>
	inline typename map<Key, T, Compare, Allocator>::size_type
	map<Key, T, Compare, Allocator>::count(const Key& key) const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Key, typename T, typename Compare, typename Allocator>
	template <typename KX>
	inline eastl::pair<typename map<Key, T, Compare, Allocator>::iterator,
					   typename map<Key, T, Compare, Allocator>::iterator>
	map<Key, T, Compare, Allocator>::DoEqualRange(const KX& key)
	{
    __builtin_trap() /* STUB: not implemented */;
}
	

	template <typename Key, typename T, typename Compare, typename Allocator>
	template <typename KX>
	inline eastl::pair<typename map<Key, T, Compare, Allocator>::const_iterator, 
					   typename map<Key, T, Compare, Allocator>::const_iterator>
	map<Key, T, Compare, Allocator>::DoEqualRange(const KX& key) const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Key, typename T, typename Compare, typename Allocator>
	inline T& map<Key, T, Compare, Allocator>::operator[](const Key& key)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Key, typename T, typename Compare, typename Allocator>
	inline T& map<Key, T, Compare, Allocator>::operator[](Key&& key)
	{
    __builtin_trap() /* STUB: not implemented */;
}

#if defined(EA_COMPILER_HAS_THREE_WAY_COMPARISON)
	template <typename Key, typename T, typename Compare, typename Allocator>
	inline synth_three_way_result<eastl::pair<const Key, T>> operator<=>(const map<Key, T, Compare, Allocator>& a, 
			const map<Key, T, Compare, Allocator>& b)
	{
		return eastl::lexicographical_compare_three_way(a.begin(), a.end(), b.begin(), b.end(), synth_three_way{});
	}
#endif

	template <typename Key, typename T, typename Compare, typename Allocator>
	template <typename KX>
	inline T& map<Key, T, Compare, Allocator>::DoAt(const KX& key)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename Key, typename T, typename Compare, typename Allocator>
	template <typename KX>
	inline const T& map<Key, T, Compare, Allocator>::DoAt(const KX& key) const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	///////////////////////////////////////////////////////////////////////
	// erase_if
	//
	// https://en.cppreference.com/w/cpp/container/map/erase_if
	///////////////////////////////////////////////////////////////////////
	template <class Key, class T, class Compare, class Allocator, class Predicate>
	typename map<Key, T, Compare, Allocator>::size_type erase_if(map<Key, T, Compare, Allocator>& c, Predicate predicate)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <class Key, class T, class Compare, class Allocator>
	template <class... Args>
	inline eastl::pair<typename map<Key, T, Compare, Allocator>::iterator, bool>
	map<Key, T, Compare, Allocator>::try_emplace(const key_type& key, Args&&... args)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <class Key, class T, class Compare, class Allocator>
	template <class... Args>
	inline eastl::pair<typename map<Key, T, Compare, Allocator>::iterator, bool>
	map<Key, T, Compare, Allocator>::try_emplace(key_type&& key, Args&&... args)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <class Key, class T, class Compare, class Allocator>
	template <class KFwd, class... Args>
	inline eastl::pair<typename map<Key, T, Compare, Allocator>::iterator, bool>
	map<Key, T, Compare, Allocator>::try_emplace_forward(KFwd&& key, Args&&... args)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <class Key, class T, class Compare, class Allocator>
	template <class... Args>
	inline typename map<Key, T, Compare, Allocator>::iterator
	map<Key, T, Compare, Allocator>::try_emplace(const_iterator hint, const key_type& key, Args&&... args)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <class Key, class T, class Compare, class Allocator>
	template <class... Args>
	inline typename map<Key, T, Compare, Allocator>::iterator
	map<Key, T, Compare, Allocator>::try_emplace(const_iterator hint, key_type&& key, Args&&... args)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <class Key, class T, class Compare, class Allocator>
	template <class KFwd, class... Args>
	inline typename map<Key, T, Compare, Allocator>::iterator
	map<Key, T, Compare, Allocator>::try_emplace_forward(const_iterator hint, KFwd&& key, Args&&... args)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	///////////////////////////////////////////////////////////////////////
	// multimap
	///////////////////////////////////////////////////////////////////////

	template <typename Key, typename T, typename Compare, typename Allocator>
	inline multimap<Key, T, Compare, Allocator>::multimap(const allocator_type& allocator)
		: base_type(allocator)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Key, typename T, typename Compare, typename Allocator>
	inline multimap<Key, T, Compare, Allocator>::multimap(const Compare& compare, const allocator_type& allocator)
		: base_type(compare, allocator)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Key, typename T, typename Compare, typename Allocator>
	inline multimap<Key, T, Compare, Allocator>::multimap(const this_type& x)
		: base_type(x)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Key, typename T, typename Compare, typename Allocator>
	inline multimap<Key, T, Compare, Allocator>::multimap(this_type&& x)
		: base_type(eastl::move(x))
	{
	}

	template <typename Key, typename T, typename Compare, typename Allocator>
	inline multimap<Key, T, Compare, Allocator>::multimap(this_type&& x, const allocator_type& allocator)
		: base_type(eastl::move(x), allocator)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Key, typename T, typename Compare, typename Allocator>
	inline multimap<Key, T, Compare, Allocator>::multimap(std::initializer_list<value_type> ilist, const Compare& compare, const allocator_type& allocator)
		: base_type(ilist.begin(), ilist.end(), compare, allocator)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Key, typename T, typename Compare, typename Allocator>
	inline multimap<Key, T, Compare, Allocator>::multimap(std::initializer_list<value_type> ilist, const allocator_type& allocator)
		: base_type(ilist.begin(), ilist.end(), Compare(), allocator)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Key, typename T, typename Compare, typename Allocator>
	template <typename Iterator>
	inline multimap<Key, T, Compare, Allocator>::multimap(Iterator itBegin, Iterator itEnd)
		: base_type(itBegin, itEnd, Compare(), EASTL_MULTIMAP_DEFAULT_ALLOCATOR)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Key, typename T, typename Compare, typename Allocator>
	inline typename multimap<Key, T, Compare, Allocator>::insert_return_type
	multimap<Key, T, Compare, Allocator>::insert(const Key& key)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Key, typename T, typename Compare, typename Allocator>
	inline typename multimap<Key, T, Compare, Allocator>::value_compare 
	multimap<Key, T, Compare, Allocator>::value_comp() const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Key, typename T, typename Compare, typename Allocator>
	template <typename KX>
	inline typename multimap<Key, T, Compare, Allocator>::size_type
	multimap<Key, T, Compare, Allocator>::DoErase(KX&& key)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Key, typename T, typename Compare, typename Allocator>
	template<typename KX>
	inline typename multimap<Key, T, Compare, Allocator>::size_type
	multimap<Key, T, Compare, Allocator>::DoCount(const KX& key) const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Key, typename T, typename Compare, typename Allocator>
	template<typename KX>
	inline eastl::pair<typename multimap<Key, T, Compare, Allocator>::iterator,
					   typename multimap<Key, T, Compare, Allocator>::iterator>
	multimap<Key, T, Compare, Allocator>::DoEqualRange(const KX& key)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Key, typename T, typename Compare, typename Allocator>
	template<typename KX>
	inline eastl::pair<typename multimap<Key, T, Compare, Allocator>::const_iterator,
					   typename multimap<Key, T, Compare, Allocator>::const_iterator>
	multimap<Key, T, Compare, Allocator>::DoEqualRange(const KX& key) const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Key, typename T, typename Compare, typename Allocator>
	template<typename KX>
	inline eastl::pair<typename multimap<Key, T, Compare, Allocator>::iterator,
					   typename multimap<Key, T, Compare, Allocator>::iterator>
	multimap<Key, T, Compare, Allocator>::DoEqualRangeSmall(const KX& key)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Key, typename T, typename Compare, typename Allocator>
	template<typename KX>
	inline eastl::pair<typename multimap<Key, T, Compare, Allocator>::const_iterator,
					   typename multimap<Key, T, Compare, Allocator>::const_iterator>
	multimap<Key, T, Compare, Allocator>::DoEqualRangeSmall(const KX& key) const
	{
    __builtin_trap() /* STUB: not implemented */;
}



	///////////////////////////////////////////////////////////////////////
	// erase_if
	//
	// https://en.cppreference.com/w/cpp/container/multimap/erase_if
	///////////////////////////////////////////////////////////////////////
	template <class Key, class T, class Compare, class Allocator, class Predicate>
	typename multimap<Key, T, Compare, Allocator>::size_type erase_if(multimap<Key, T, Compare, Allocator>& c, Predicate predicate)
	{
    __builtin_trap() /* STUB: not implemented */;
}

#if defined(EA_COMPILER_HAS_THREE_WAY_COMPARISON)
	template <typename Key, typename T, typename Compare, typename Allocator>
	inline synth_three_way_result<eastl::pair<const Key, T>> operator<=>(const multimap<Key, T, Compare, Allocator>& a, 
			const multimap<Key, T, Compare, Allocator>& b)
	{
		return eastl::lexicographical_compare_three_way(a.begin(), a.end(), b.begin(), b.end(), synth_three_way{});
	}
#endif

} // namespace eastl


#endif // Header include guard




