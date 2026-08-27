#include <cstdlib>
///////////////////////////////////////////////////////////////////////////////
// Copyright (c) Electronic Arts Inc. All rights reserved.
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// This file is based on the TR1 (technical report 1) reference implementation
// of the unordered_set/unordered_map C++ classes as of about 4/2005. Most likely
// many or all C++ library vendors' implementations of this classes will be 
// based off of the reference version and so will look pretty similar to this
// file as well as other vendors' versions. 
///////////////////////////////////////////////////////////////////////////////


#ifndef EASTL_HASH_MAP_H
#define EASTL_HASH_MAP_H


#include <EASTL/internal/config.h>
#include <EASTL/internal/hashtable.h>
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

	/// EASTL_HASH_MAP_DEFAULT_NAME
	///
	/// Defines a default container name in the absence of a user-provided name.
	///
	#ifndef EASTL_HASH_MAP_DEFAULT_NAME
		#define EASTL_HASH_MAP_DEFAULT_NAME EASTL_DEFAULT_NAME_PREFIX " hash_map" // Unless the user overrides something, this is "EASTL hash_map".
	#endif


	/// EASTL_HASH_MULTIMAP_DEFAULT_NAME
	///
	/// Defines a default container name in the absence of a user-provided name.
	///
	#ifndef EASTL_HASH_MULTIMAP_DEFAULT_NAME
		#define EASTL_HASH_MULTIMAP_DEFAULT_NAME EASTL_DEFAULT_NAME_PREFIX " hash_multimap" // Unless the user overrides something, this is "EASTL hash_multimap".
	#endif


	/// EASTL_HASH_MAP_DEFAULT_ALLOCATOR
	///
	#ifndef EASTL_HASH_MAP_DEFAULT_ALLOCATOR
		#define EASTL_HASH_MAP_DEFAULT_ALLOCATOR allocator_type(EASTL_HASH_MAP_DEFAULT_NAME)
	#endif

	/// EASTL_HASH_MULTIMAP_DEFAULT_ALLOCATOR
	///
	#ifndef EASTL_HASH_MULTIMAP_DEFAULT_ALLOCATOR
		#define EASTL_HASH_MULTIMAP_DEFAULT_ALLOCATOR allocator_type(EASTL_HASH_MULTIMAP_DEFAULT_NAME)
	#endif



	/// hash_map
	///
	/// Implements a hash_map, which is a hashed associative container.
	/// Lookups are O(1) (that is, they are fast) but the container is 
	/// not sorted. Note that lookups are only O(1) if the hash table
	/// is well-distributed (non-colliding). The lookup approaches
	/// O(n) behavior as the table becomes increasingly poorly distributed.
	///
	/// set_max_load_factor
	/// If you want to make a hashtable never increase its bucket usage,
	/// call set_max_load_factor with a very high value such as 100000.f.
	///
	/// bCacheHashCode
	/// We provide the boolean bCacheHashCode template parameter in order 
	/// to allow the storing of the hash code of the key within the map. 
	/// When this option is disabled, the rehashing of the table will 
	/// call the hash function on the key. Setting bCacheHashCode to true 
	/// is useful for cases whereby the calculation of the hash value for
	/// a contained object is very expensive.
	///
	/// Heterogeneous lookup, insertion and erasure
	/// See
	/// https://en.cppreference.com/w/cpp/utility/functional#Transparent_function_objects
	/// https://en.cppreference.com/w/cpp/utility/functional/less_void
	/// https://en.cppreference.com/w/cpp/container/unordered_map/find
	/// 
	/// You can avoid creating key objects when calling member functions
	/// with a key_type parameter by declaring the container with a
	/// transparent hash and comparison type (eg. equal_to<void>) and
	/// passing objects to be passed to these function objects.
	/// 
	/// This optimization is supported for member functions that take a
	/// key_type parameter, ie. heterogeneous lookup, insertion and erasure,
	/// not just find().
	/// 
	/// Using transparent types is safer than using find_as because the
	/// latter requires the user specify hash and equality function objects
	/// which must have the same semantics as the container's hash and
	/// equality objects, otherwise the behaviour is undefined.
	/// 
	/// find_as
	/// Note: Prefer heterogeneous lookup (see above).
	/// 
	/// In order to support the ability to have a hashtable of strings but
	/// be able to do efficiently lookups via char pointers (i.e. so they 
	/// aren't converted to string objects), we provide the find_as 
	/// function. This function allows you to do a find with a key of a
	/// type other than the hashtable key type.
	///
	/// Example find_as usage:
	///     hash_map<string, int> hashMap;
	///     i = hashMap.find_as("hello");    // Use default hash and compare.
	///
	/// Example find_as usage (namespaces omitted for brevity):
	///     hash_map<string, int> hashMap;
	///     i = hashMap.find_as("hello", hash<char*>(), equal_to<>());
	///
	template <typename Key, typename T, typename Hash = eastl::hash<Key>, typename Predicate = eastl::equal_to<Key>, 
			  typename Allocator = EASTLAllocatorType, bool bCacheHashCode = false>
	class hash_map
		: public hashtable<Key, eastl::pair<const Key, T>, Allocator, eastl::use_first<eastl::pair<const Key, T> >, Predicate,
							Hash, mod_range_hashing, default_ranged_hash, prime_rehash_policy, bCacheHashCode, true, true>
	{
	public:
		typedef hashtable<Key, eastl::pair<const Key, T>, Allocator, 
						  eastl::use_first<eastl::pair<const Key, T> >, 
						  Predicate, Hash, mod_range_hashing, default_ranged_hash, 
						  prime_rehash_policy, bCacheHashCode, true, true>        base_type;
		typedef hash_map<Key, T, Hash, Predicate, Allocator, bCacheHashCode>      this_type;
		typedef typename base_type::size_type                                     size_type;
		typedef typename base_type::key_type                                      key_type;
		typedef T                                                                 mapped_type;
		typedef typename base_type::value_type                                    value_type;     // NOTE: 'value_type = pair<const key_type, mapped_type>'.
		typedef typename base_type::allocator_type                                allocator_type;
		typedef typename base_type::node_type                                     node_type;
		typedef typename base_type::insert_return_type                            insert_return_type;
		typedef typename base_type::iterator                                      iterator;
		typedef typename base_type::const_iterator                                const_iterator;

		using base_type::insert;

		static_assert(!is_const<value_type>::value, "hash_map<T> value_type must be non-const.");
		static_assert(!is_volatile<value_type>::value, "hash_map<T> value_type must be non-volatile.");

	public:
		/// hash_map
		///
		/// Default constructor.
		///
		hash_map()
			: this_type(EASTL_HASH_MAP_DEFAULT_ALLOCATOR)
		{
    __builtin_trap() /* STUB: not implemented */;
}


		/// hash_map
		///
		/// Constructor which creates an empty container with allocator.
		///
		explicit hash_map(const allocator_type& allocator)
			: base_type(0, Hash(), mod_range_hashing(), default_ranged_hash(),
						Predicate(), eastl::use_first<eastl::pair<const Key, T> >(), allocator)
		{
    __builtin_trap() /* STUB: not implemented */;
}


		/// hash_map
		///
		/// Constructor which creates an empty container, but start with nBucketCount buckets.
		/// We default to a small nBucketCount value, though the user really should manually 
		/// specify an appropriate value in order to prevent memory from being reallocated.
		///
		/// note: difference in explicit keyword from the standard.
		explicit hash_map(size_type nBucketCount, const Hash& hashFunction = Hash(), 
						  const Predicate& predicate = Predicate(), const allocator_type& allocator = EASTL_HASH_MAP_DEFAULT_ALLOCATOR)
			: base_type(nBucketCount, hashFunction, mod_range_hashing(), default_ranged_hash(), 
						predicate, eastl::use_first<eastl::pair<const Key, T> >(), allocator)
		{
    __builtin_trap() /* STUB: not implemented */;
}

		// hash_map(size_type nBucketCount, const allocator_type& allocator)
		// hash_map(size_type nBucketCount, const Hash& hashFunction, const allocator_type& allocator)


		hash_map(const this_type& x)
		  : base_type(x)
		{
    __builtin_trap() /* STUB: not implemented */;
}

		// hash_map(const this_type& x, const allocator_type& allocator)


		hash_map(this_type&& x)
		  : base_type(eastl::move(x))
		{
		}


		hash_map(this_type&& x, const allocator_type& allocator)
		  : base_type(eastl::move(x), allocator)
		{
    __builtin_trap() /* STUB: not implemented */;
}


		/// hash_map
		///
		/// initializer_list-based constructor. 
		/// Allows for initializing with brace values (e.g. hash_map<int, char*> hm = { {3,"c"}, {4,"d"}, {5,"e"} }; )
		///     
		hash_map(std::initializer_list<value_type> ilist, size_type nBucketCount = 0, const Hash& hashFunction = Hash(), 
				   const Predicate& predicate = Predicate(), const allocator_type& allocator = EASTL_HASH_MAP_DEFAULT_ALLOCATOR)
			: base_type(ilist.begin(), ilist.end(), nBucketCount, hashFunction, mod_range_hashing(), default_ranged_hash(), 
						predicate, eastl::use_first<eastl::pair<const Key, T> >(), allocator)
		{
    __builtin_trap() /* STUB: not implemented */;
}

		hash_map(std::initializer_list<value_type> ilist, const allocator_type& allocator)
			: base_type(ilist.begin(), ilist.end(), 0, Hash(), mod_range_hashing(), default_ranged_hash(), Predicate(), eastl::use_first<eastl::pair<const Key, T> >(), allocator)
		{
    __builtin_trap() /* STUB: not implemented */;
}

		// hash_map(std::initializer_list<value_type> ilist, size_type nBucketCount, const allocator_type& allocator)

		// hash_map(std::initializer_list<value_type> ilist, size_type nBucketCount, const Hash& hashFunction,
		// 	const allocator_type& allocator)

		/// hash_map
		///
		/// An input bucket count of <= 1 causes the bucket count to be equal to the number of 
		/// elements in the input range.
		///
		template <typename ForwardIterator>
		hash_map(ForwardIterator first, ForwardIterator last, size_type nBucketCount = 0, const Hash& hashFunction = Hash(), 
				 const Predicate& predicate = Predicate(), const allocator_type& allocator = EASTL_HASH_MAP_DEFAULT_ALLOCATOR)
			: base_type(first, last, nBucketCount, hashFunction, mod_range_hashing(), default_ranged_hash(), 
						predicate, eastl::use_first<eastl::pair<const Key, T> >(), allocator)
		{
    __builtin_trap() /* STUB: not implemented */;
}

		// template <typename ForwardIterator>
		// hash_map(ForwardIterator first, ForwardIterator last, size_type nBucketCount, const allocator_type& allocator)

		// template <typename ForwardIterator>
		// hash_map(ForwardIterator first, ForwardIterator last, size_type nBucketCount, const Hash& hashFunction, const allocator_type& allocator)

		this_type& operator=(const this_type& x)
		{
    __builtin_trap() /* STUB: not implemented */;
}


		this_type& operator=(std::initializer_list<value_type> ilist)
		{
    __builtin_trap() /* STUB: not implemented */;
}


		this_type& operator=(this_type&& x)
		{
    __builtin_trap() /* STUB: not implemented */;
}


		/// insert
		///
		/// This is an extension to the C++ standard. We insert a default-constructed 
		/// element with the given key. The reason for this is that we can avoid the 
		/// potentially expensive operation of creating and/or copying a mapped_type
		/// object on the stack.
		insert_return_type insert(const key_type& key)
		{
    __builtin_trap() /* STUB: not implemented */;
}

		insert_return_type insert(key_type&& key)
		{
    __builtin_trap() /* STUB: not implemented */;
}

		template <typename P, eastl::enable_if_t<eastl::is_constructible_v<value_type, P&&>, bool> = true>
		insert_return_type insert(P&& otherValue)
		{
    __builtin_trap() /* STUB: not implemented */;
}

		T& at(const key_type& k) {
    __builtin_trap() /* STUB: not implemented */;
}

		const T& at(const key_type& k) const {
    __builtin_trap() /* STUB: not implemented */;
}

		template<typename KX, typename HX = Hash, typename PX = Predicate,
			eastl::enable_if_t<eastl::detail::is_transparent_comparison_v<HX> && eastl::detail::is_transparent_comparison_v<PX>, bool> = true>
		T& at(const KX& k) {
    __builtin_trap() /* STUB: not implemented */;
}

		template<typename KX, typename HX = Hash, typename PX = Predicate,
			eastl::enable_if_t<eastl::detail::is_transparent_comparison_v<HX>&& eastl::detail::is_transparent_comparison_v<PX>, bool> = true>
		const T& at(const KX& k) const {
    __builtin_trap() /* STUB: not implemented */;
}


		mapped_type& operator[](const key_type& key)
		{
    __builtin_trap() /* STUB: not implemented */;
}

		mapped_type& operator[](key_type&& key)
		{
    __builtin_trap() /* STUB: not implemented */;
}

		template <typename KX, typename... Args, typename HX = Hash, typename PX = Predicate,
			eastl::enable_if_t<eastl::detail::is_transparent_comparison_v<HX> && eastl::detail::is_transparent_comparison_v<PX>, bool> = true>
		mapped_type& operator[](KX&& key)
		{
    __builtin_trap() /* STUB: not implemented */;
}

		// try_emplace API added in C++17
		template <class... Args>
		inline insert_return_type try_emplace(const key_type& k, Args&&... args)
		{
    __builtin_trap() /* STUB: not implemented */;
}

		template <class... Args>
		inline insert_return_type try_emplace(key_type&& k, Args&&... args) {
    __builtin_trap() /* STUB: not implemented */;
}

		template <typename KX, typename... Args, typename HX = Hash, typename PX = Predicate,
			eastl::enable_if_t<!eastl::is_convertible_v<KX&&, const_iterator> && !eastl::is_convertible_v<KX&&, iterator>
			&& eastl::detail::is_transparent_comparison_v<HX> && eastl::detail::is_transparent_comparison_v<PX>, bool> = true>
		inline insert_return_type try_emplace(KX&& k, Args&&... args) {
    __builtin_trap() /* STUB: not implemented */;
}

		template <class... Args>
		inline iterator try_emplace(const_iterator, const key_type& k, Args&&... args) {
    __builtin_trap() /* STUB: not implemented */;
}

		template <class... Args>
		inline iterator try_emplace(const_iterator, key_type&& k, Args&&... args) {
    __builtin_trap() /* STUB: not implemented */;
}

		template <typename KX, typename... Args, typename HX = Hash, typename PX = Predicate,
			eastl::enable_if_t<eastl::detail::is_transparent_comparison_v<HX>&& eastl::detail::is_transparent_comparison_v<PX>, bool> = true>
		inline iterator try_emplace(const_iterator, KX&& k, Args&&... args) {
    __builtin_trap() /* STUB: not implemented */;
}

	private:
		template<typename KX>
		T& DoAt(const KX& k)
		{
    __builtin_trap() /* STUB: not implemented */;
}

		template<typename KX>
		const T& DoAt(const KX& k) const
		{
    __builtin_trap() /* STUB: not implemented */;
}

		template <class K, class... Args>
		insert_return_type try_emplace_forwarding(K&& k, Args&&... args)
		{
    __builtin_trap() /* STUB: not implemented */;
}
	}; // hash_map

	/// hash_map erase_if
	///
	/// https://en.cppreference.com/w/cpp/container/unordered_map/erase_if
	template <typename Key, typename T, typename Hash, typename Predicate, typename Allocator, bool bCacheHashCode, typename UserPredicate>
	typename eastl::hash_map<Key, T, Hash, Predicate, Allocator, bCacheHashCode>::size_type erase_if(eastl::hash_map<Key, T, Hash, Predicate, Allocator, bCacheHashCode>& c, UserPredicate predicate)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	/// hash_multimap
	///
	/// Implements a hash_multimap, which is the same thing as a hash_map 
	/// except that contained elements need not be unique. See the 
	/// documentation for hash_set for details.
	///
	template <typename Key, typename T, typename Hash = eastl::hash<Key>, typename Predicate = eastl::equal_to<Key>,
			  typename Allocator = EASTLAllocatorType, bool bCacheHashCode = false>
	class hash_multimap
		: public hashtable<Key, eastl::pair<const Key, T>, Allocator, eastl::use_first<eastl::pair<const Key, T> >, Predicate,
						   Hash, mod_range_hashing, default_ranged_hash, prime_rehash_policy, bCacheHashCode, true, false>
	{
	public:
		typedef hashtable<Key, eastl::pair<const Key, T>, Allocator, 
						  eastl::use_first<eastl::pair<const Key, T> >, 
						  Predicate, Hash, mod_range_hashing, default_ranged_hash, 
						  prime_rehash_policy, bCacheHashCode, true, false>           base_type;
		typedef hash_multimap<Key, T, Hash, Predicate, Allocator, bCacheHashCode>     this_type;
		typedef typename base_type::size_type                                         size_type;
		typedef typename base_type::key_type                                          key_type;
		typedef T                                                                     mapped_type;
		typedef typename base_type::value_type                                        value_type;     // Note that this is pair<const key_type, mapped_type>.
		typedef typename base_type::allocator_type                                    allocator_type;
		typedef typename base_type::node_type                                         node_type;
		typedef typename base_type::insert_return_type                                insert_return_type;
		typedef typename base_type::iterator                                          iterator;

		using base_type::insert;

		static_assert(!is_const<value_type>::value, "hash_multimap<T> value_type must be non-const.");
		static_assert(!is_volatile<value_type>::value, "hash_multimap<T> value_type must be non-volatile.");

	private:
		using base_type::insert_or_assign;

	public:
		/// hash_multimap
		///
		/// Default constructor.
		///
		explicit hash_multimap(const allocator_type& allocator = EASTL_HASH_MULTIMAP_DEFAULT_ALLOCATOR)
			: base_type(0, Hash(), mod_range_hashing(), default_ranged_hash(), 
						Predicate(), eastl::use_first<eastl::pair<const Key, T> >(), allocator)
		{
    __builtin_trap() /* STUB: not implemented */;
}


		/// hash_multimap
		///
		/// Constructor which creates an empty container, but start with nBucketCount buckets.
		/// We default to a small nBucketCount value, though the user really should manually 
		/// specify an appropriate value in order to prevent memory from being reallocated.
		///
		/// note: difference in explicit keyword from the standard.
		explicit hash_multimap(size_type nBucketCount, const Hash& hashFunction = Hash(), 
							   const Predicate& predicate = Predicate(), const allocator_type& allocator = EASTL_HASH_MULTIMAP_DEFAULT_ALLOCATOR)
			: base_type(nBucketCount, hashFunction, mod_range_hashing(), default_ranged_hash(), 
						predicate, eastl::use_first<eastl::pair<const Key, T> >(), allocator)
		{
    __builtin_trap() /* STUB: not implemented */;
}

		// hash_multimap(size_type nBucketCount, const allocator_type& allocator)
		// hash_multimap(size_type nBucketCount, const Hash& hashFunction, const allocator_type& allocator)


		hash_multimap(const this_type& x)
		  : base_type(x)
		{
    __builtin_trap() /* STUB: not implemented */;
}

		// hash_multimap(const this_type& x, const allocator_type& allocator)


		hash_multimap(this_type&& x)
		  : base_type(eastl::move(x))
		{
		}


		hash_multimap(this_type&& x, const allocator_type& allocator)
		  : base_type(eastl::move(x), allocator)
		{
    __builtin_trap() /* STUB: not implemented */;
}


		/// hash_multimap
		///
		/// initializer_list-based constructor. 
		/// Allows for initializing with brace values (e.g. hash_multimap<int, char*> hm = { {3,"c"}, {3,"C"}, {4,"d"} }; )
		///     
		hash_multimap(std::initializer_list<value_type> ilist, size_type nBucketCount = 0, const Hash& hashFunction = Hash(), 
				   const Predicate& predicate = Predicate(), const allocator_type& allocator = EASTL_HASH_MULTIMAP_DEFAULT_ALLOCATOR)
			: base_type(ilist.begin(), ilist.end(), nBucketCount, hashFunction, mod_range_hashing(), default_ranged_hash(), 
						predicate, eastl::use_first<eastl::pair<const Key, T> >(), allocator)
		{
    __builtin_trap() /* STUB: not implemented */;
}

		hash_multimap(std::initializer_list<value_type> ilist, const allocator_type& allocator)
			: base_type(ilist.begin(), ilist.end(), 0, Hash(), mod_range_hashing(), default_ranged_hash(), Predicate(), eastl::use_first<eastl::pair<const Key, T> >(), allocator)
		{
    __builtin_trap() /* STUB: not implemented */;
}

		// hash_multimap(std::initializer_list<value_type> ilist, size_type nBucketCount, const allocator_type& allocator)

		// hash_multimap(std::initializer_list<value_type> ilist, size_type nBucketCount, const Hash& hashFunction,
		// 	const allocator_type& allocator)


		/// hash_multimap
		///
		/// An input bucket count of <= 1 causes the bucket count to be equal to the number of 
		/// elements in the input range.
		///
		template <typename ForwardIterator>
		hash_multimap(ForwardIterator first, ForwardIterator last, size_type nBucketCount = 0, const Hash& hashFunction = Hash(), 
					  const Predicate& predicate = Predicate(), const allocator_type& allocator = EASTL_HASH_MULTIMAP_DEFAULT_ALLOCATOR)
			: base_type(first, last, nBucketCount, hashFunction, mod_range_hashing(), default_ranged_hash(), 
						predicate, eastl::use_first<eastl::pair<const Key, T> >(), allocator)
		{
    __builtin_trap() /* STUB: not implemented */;
}

		// template <typename ForwardIterator>
		// hash_multimap(ForwardIterator first, ForwardIterator last, size_type nBucketCount, const allocator_type& allocator)

		// template <typename ForwardIterator>
		// hash_multimap(ForwardIterator first, ForwardIterator last, size_type nBucketCount, const Hash& hashFunction, const allocator_type& allocator)

		this_type& operator=(const this_type& x)
		{
    __builtin_trap() /* STUB: not implemented */;
}


		this_type& operator=(std::initializer_list<value_type> ilist)
		{
    __builtin_trap() /* STUB: not implemented */;
}


		this_type& operator=(this_type&& x)
		{
    __builtin_trap() /* STUB: not implemented */;
}


		/// insert
		///
		/// This is an extension to the C++ standard. We insert a default-constructed 
		/// element with the given key. The reason for this is that we can avoid the 
		/// potentially expensive operation of creating and/or copying a mapped_type
		/// object on the stack.
		insert_return_type insert(const key_type& key)
		{
    __builtin_trap() /* STUB: not implemented */;
}


		insert_return_type insert(key_type&& key)
		{
    __builtin_trap() /* STUB: not implemented */;
}

		template <typename P, eastl::enable_if_t<eastl::is_constructible_v<value_type, P&&>, bool> = true>
		insert_return_type insert(P&& otherValue)
		{
    __builtin_trap() /* STUB: not implemented */;
}

	}; // hash_multimap

	/// hash_multimap erase_if
	///
	/// https://en.cppreference.com/w/cpp/container/unordered_multimap/erase_if
	template <typename Key, typename T, typename Hash, typename Predicate, typename Allocator, bool bCacheHashCode, typename UserPredicate>
	typename eastl::hash_multimap<Key, T, Hash, Predicate, Allocator, bCacheHashCode>::size_type erase_if(eastl::hash_multimap<Key, T, Hash, Predicate, Allocator, bCacheHashCode>& c, UserPredicate predicate)
	{
    __builtin_trap() /* STUB: not implemented */;
}



	///////////////////////////////////////////////////////////////////////
	// global operators
	///////////////////////////////////////////////////////////////////////

	template <typename Key, typename T, typename Hash, typename Predicate, typename Allocator, bool bCacheHashCode>
	inline bool operator==(const hash_map<Key, T, Hash, Predicate, Allocator, bCacheHashCode>& a, 
						   const hash_map<Key, T, Hash, Predicate, Allocator, bCacheHashCode>& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}

#if !defined(EA_COMPILER_HAS_THREE_WAY_COMPARISON)
	template <typename Key, typename T, typename Hash, typename Predicate, typename Allocator, bool bCacheHashCode>
	inline bool operator!=(const hash_map<Key, T, Hash, Predicate, Allocator, bCacheHashCode>& a, 
						   const hash_map<Key, T, Hash, Predicate, Allocator, bCacheHashCode>& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}
#endif

	template <typename Key, typename T, typename Hash, typename Predicate, typename Allocator, bool bCacheHashCode>
	inline bool operator==(const hash_multimap<Key, T, Hash, Predicate, Allocator, bCacheHashCode>& a, 
						   const hash_multimap<Key, T, Hash, Predicate, Allocator, bCacheHashCode>& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}

#if !defined(EA_COMPILER_HAS_THREE_WAY_COMPARISON)
	template <typename Key, typename T, typename Hash, typename Predicate, typename Allocator, bool bCacheHashCode>
	inline bool operator!=(const hash_multimap<Key, T, Hash, Predicate, Allocator, bCacheHashCode>& a, 
						   const hash_multimap<Key, T, Hash, Predicate, Allocator, bCacheHashCode>& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}
#endif


} // namespace eastl


#endif // Header include guard






