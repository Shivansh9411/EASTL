#include <cstdlib>
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) Electronic Arts Inc. All rights reserved.
/////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// This file implements a hashtable, much like the C++11 unordered_set/unordered_map.
// proposed classes.
// The primary distinctions between this hashtable and C++11 unordered containers are:
//    - hashtable is savvy to an environment that doesn't have exception handling,
//      as is sometimes the case with console or embedded environments.
//    - hashtable is slightly more space-efficient than a conventional std hashtable 
//      implementation on platforms with 64 bit size_t.  This is 
//      because std STL uses size_t (64 bits) in data structures whereby 32 bits 
//      of data would be fine.
//    - hashtable can contain objects with alignment requirements. TR1 hash tables 
//      cannot do so without a bit of tedious non-portable effort.
//    - hashtable supports debug memory naming natively.
//    - hashtable provides a find function that lets you specify a type that is 
//      different from the hash table key type. This is particularly useful for 
//      the storing of string objects but finding them by char pointers.
//    - hashtable provides a lower level insert function which lets the caller 
//      specify the hash code and optionally the node instance.
///////////////////////////////////////////////////////////////////////////////


#ifndef EASTL_INTERNAL_HASHTABLE_H
#define EASTL_INTERNAL_HASHTABLE_H


#include <EABase/eabase.h>
#if defined(EA_PRAGMA_ONCE_SUPPORTED)
	#pragma once
#endif

#include <EASTL/internal/config.h>
#include <EASTL/type_traits.h>
#include <EASTL/allocator.h>
#include <EASTL/iterator.h>
#include <EASTL/functional.h>
#include <EASTL/utility.h>
#include <EASTL/algorithm.h>
#include <EASTL/initializer_list.h>
#include <EASTL/tuple.h>
#include <EASTL/memory.h>
#include <string.h>
#if EASTL_EXCEPTIONS_ENABLED
#include <stdexcept>
#endif

EA_DISABLE_ALL_VC_WARNINGS()
	#include <new>
	#include <stddef.h>
EA_RESTORE_ALL_VC_WARNINGS()

// 4512/4626 - 'class' : assignment operator could not be generated.
// 4530 - C++ exception handler used, but unwind semantics are not enabled. Specify /EHsc
// 4571 - catch(...) semantics changed since Visual C++ 7.1; structured exceptions (SEH) are no longer caught.
EA_DISABLE_VC_WARNING(4512 4626 4530 4571);


namespace eastl
{

	/// EASTL_HASHTABLE_DEFAULT_NAME
	///
	/// Defines a default container name in the absence of a user-provided name.
	///
	#ifndef EASTL_HASHTABLE_DEFAULT_NAME
		#define EASTL_HASHTABLE_DEFAULT_NAME EASTL_DEFAULT_NAME_PREFIX " hashtable" // Unless the user overrides something, this is "EASTL hashtable".
	#endif


	/// EASTL_HASHTABLE_DEFAULT_ALLOCATOR
	///
	#ifndef EASTL_HASHTABLE_DEFAULT_ALLOCATOR
		#define EASTL_HASHTABLE_DEFAULT_ALLOCATOR allocator_type(EASTL_HASHTABLE_DEFAULT_NAME)
	#endif

	
	/// kHashtableAllocFlagBuckets
	/// Flag to allocator which indicates that we are allocating buckets and not nodes.
	enum { kHashtableAllocFlagBuckets = 0x00400000 };


	/// gpEmptyBucketArray
	///
	/// A shared representation of an empty hash table. This is present so that
	/// a new empty hashtable allocates no memory. It has two entries, one for 
	/// the first lone empty (NULL) bucket, and one for the non-NULL trailing sentinel.
	/// 
	extern EASTL_API void* gpEmptyBucketArray[2];


	/// EASTL_MACRO_SWAP
	///
	/// Use EASTL_MACRO_SWAP because GCC (at least v4.6-4.8) has a bug where it fails to compile eastl::swap(mpBucketArray, x.mpBucketArray).
	///
	#define EASTL_MACRO_SWAP(Type, a, b) \
		{ Type temp = a; a = b; b = temp; }


	/// hash_node
	///
	/// A hash_node stores an element in a hash table, much like a 
	/// linked list node stores an element in a linked list. 
	/// A hash_node additionally can, via template parameter,
	/// store a hash code in the node to speed up hash calculations 
	/// and comparisons in some cases.
	/// 
	template <typename Value, bool bCacheHashCode>
	struct hash_node;

	EA_DISABLE_VC_WARNING(4625 4626) // "copy constructor / assignment operator could not be generated because a base class copy constructor is inaccessible or deleted"
	#ifdef EA_COMPILER_MSVC_2015
		EA_DISABLE_VC_WARNING(5026) // disable warning: "move constructor was implicitly defined as deleted"
	#endif
		template <typename Value>
		struct hash_node<Value, true>
		{
			hash_node() = default;
			hash_node(const hash_node&) = default;
			hash_node(hash_node&&) = default;

			Value        mValue;
			hash_node*   mpNext;
			eastl_size_t mnHashCode;      // See config.h for the definition of eastl_size_t, which defaults to size_t.
		} EASTL_MAY_ALIAS;

		template <typename Value>
		struct hash_node<Value, false>
		{
			hash_node() = default;
			hash_node(const hash_node&) = default;
			hash_node(hash_node&&) = default;

		    Value      mValue;
			hash_node* mpNext;
		} EASTL_MAY_ALIAS;

	#ifdef EA_COMPILER_MSVC_2015
		EA_RESTORE_VC_WARNING()
	#endif
	EA_RESTORE_VC_WARNING()


	// has_hashcode_member
	//
	// Custom type-trait that checks for the existence of a class data member 'mnHashCode'.  
	//
	// In order to explicitly instantiate the hashtable without error we need to SFINAE away the functions that will
	// fail to compile based on if the 'hash_node' contains a 'mnHashCode' member dictated by the hashtable template
	// parameters. The hashtable support this level of configuration to allow users to choose which between the space vs.
	// time optimization.
	//
	namespace Internal
	{
		template <class T>
		struct has_hashcode_member 
		{
		private:
			template <class U> static eastl::no_type test(...);
			template <class U> static eastl::yes_type test(decltype(U::mnHashCode)* = 0);
		public:
			static const bool value = sizeof(test<T>(0)) == sizeof(eastl::yes_type);
		};
	}
	
	static_assert(Internal::has_hashcode_member<hash_node<int, true>>::value, "contains a mnHashCode member");
	static_assert(!Internal::has_hashcode_member<hash_node<int, false>>::value, "doesn't contain a mnHashCode member");

	// convenience macros to increase the readability of the code paths that must SFINAE on if the 'hash_node'
	// contains the cached hashed value or not. 
	#define ENABLE_IF_HAS_HASHCODE(T, RT) typename eastl::enable_if<Internal::has_hashcode_member<T>::value, RT>::type*
	#define ENABLE_IF_HASHCODE_EASTLSIZET(T, RT) typename eastl::enable_if<eastl::is_convertible<T, eastl_size_t>::value, RT>::type
	#define ENABLE_IF_TRUETYPE(T) typename eastl::enable_if<T::value>::type*
	#define DISABLE_IF_TRUETYPE(T) typename eastl::enable_if<!T::value>::type*


	/// node_iterator_base
	///
	/// Node iterators iterate nodes within a given bucket.
	///
	/// We define a base class here because it is shared by both const and
	/// non-const iterators.
	///
	template <typename Value, bool bCacheHashCode>
	struct node_iterator_base
	{
		typedef hash_node<Value, bCacheHashCode> node_type;

		node_type* mpNode;

		node_iterator_base(node_type* pNode)
			: mpNode(pNode) {
    __builtin_trap() /* STUB: not implemented */;
}

		void increment()
			{
    __builtin_trap() /* STUB: not implemented */;
}
	};



	/// node_iterator
	///
	/// Node iterators iterate nodes within a given bucket.
	///
	/// The bConst parameter defines if the iterator is a const_iterator
	/// or an iterator.
	///
	template <typename Value, bool bConst, bool bCacheHashCode>
	struct node_iterator : public node_iterator_base<Value, bCacheHashCode>
	{
	public:
		typedef node_iterator_base<Value, bCacheHashCode>                base_type;
		typedef node_iterator<Value, bConst, bCacheHashCode>             this_type;
		typedef typename base_type::node_type                            node_type;
		typedef Value                                                    value_type;
		typedef typename conditional<bConst, const Value*, Value*>::type pointer;
		typedef typename conditional<bConst, const Value&, Value&>::type reference;
		typedef ptrdiff_t                                                difference_type;
		typedef eastl::forward_iterator_tag                       iterator_category;

	public:
		explicit node_iterator(node_type* pNode = NULL)
			: base_type(pNode) {
    __builtin_trap() /* STUB: not implemented */;
}

		node_iterator(const node_iterator<Value, true, bCacheHashCode>& x)
			: base_type(x.mpNode) {
    __builtin_trap() /* STUB: not implemented */;
}

		reference operator*() const
			{
    __builtin_trap() /* STUB: not implemented */;
}

		pointer operator->() const
			{
    __builtin_trap() /* STUB: not implemented */;
}

		node_iterator& operator++()
			{
    __builtin_trap() /* STUB: not implemented */;
}

		node_iterator operator++(int)
			{
    __builtin_trap() /* STUB: not implemented */;
}

	}; // node_iterator



	/// hashtable_iterator_base
	///
	/// A hashtable_iterator iterates the entire hash table and not just
	/// nodes within a single bucket. Users in general will use a hash
	/// table iterator much more often, as it is much like other container
	/// iterators (e.g. vector::iterator).
	///
	/// We define a base class here because it is shared by both const and
	/// non-const iterators.
	///
	template <typename Value, bool bCacheHashCode>
	struct hashtable_iterator_base
	{
	public:
		typedef hashtable_iterator_base<Value, bCacheHashCode> this_type;
		typedef hash_node<Value, bCacheHashCode>               node_type;

	protected:
		template <typename, typename, typename, typename, typename, typename, typename, typename, typename, bool, bool, bool>
		friend class hashtable;

		template <typename, bool, bool>
		friend struct hashtable_iterator;

		template <typename V, bool b>
		friend bool operator==(const hashtable_iterator_base<V, b>&, const hashtable_iterator_base<V, b>&);

		template <typename V, bool b>
		friend bool operator!=(const hashtable_iterator_base<V, b>&, const hashtable_iterator_base<V, b>&);

		node_type*  mpNode;      // Current node within current bucket.
		node_type** mpBucket;    // Current bucket.

	public:
		hashtable_iterator_base(node_type* pNode, node_type** pBucket)
			: mpNode(pNode), mpBucket(pBucket) {
    __builtin_trap() /* STUB: not implemented */;
}

		void increment_bucket()
		{
    __builtin_trap() /* STUB: not implemented */;
}

		void increment()
		{
    __builtin_trap() /* STUB: not implemented */;
}

	}; // hashtable_iterator_base




	/// hashtable_iterator
	///
	/// A hashtable_iterator iterates the entire hash table and not just
	/// nodes within a single bucket. Users in general will use a hash
	/// table iterator much more often, as it is much like other container
	/// iterators (e.g. vector::iterator).
	///
	/// The bConst parameter defines if the iterator is a const_iterator
	/// or an iterator.
	///
	template <typename Value, bool bConst, bool bCacheHashCode>
	struct hashtable_iterator : public hashtable_iterator_base<Value, bCacheHashCode>
	{
	public:
		typedef hashtable_iterator_base<Value, bCacheHashCode>           base_type;
		typedef hashtable_iterator<Value, bConst, bCacheHashCode>        this_type;
		typedef hashtable_iterator<Value, false, bCacheHashCode>         this_type_non_const;
		typedef typename base_type::node_type                            node_type;
		typedef Value                                                    value_type;
		typedef typename conditional<bConst, const Value*, Value*>::type pointer;
		typedef typename conditional<bConst, const Value&, Value&>::type reference;
		typedef ptrdiff_t                                                difference_type;
		typedef eastl::forward_iterator_tag                       iterator_category;

	public:
		hashtable_iterator(node_type* pNode = NULL, node_type** pBucket = NULL)
			: base_type(pNode, pBucket) {
    __builtin_trap() /* STUB: not implemented */;
}

		hashtable_iterator(node_type** pBucket)
			: base_type(*pBucket, pBucket) {
    __builtin_trap() /* STUB: not implemented */;
}

		template <bool IsConst = bConst, typename enable_if<IsConst, int>::type = 0>
		hashtable_iterator(const this_type_non_const& x)
			: base_type(x.mpNode, x.mpBucket) {
    __builtin_trap() /* STUB: not implemented */;
}

		hashtable_iterator(const hashtable_iterator&) = default;
		hashtable_iterator(hashtable_iterator&&) = default;
		hashtable_iterator& operator=(const hashtable_iterator&) = default;
		hashtable_iterator& operator=(hashtable_iterator&&) = default;

		reference operator*() const
			{
    __builtin_trap() /* STUB: not implemented */;
}

		pointer operator->() const
			{
    __builtin_trap() /* STUB: not implemented */;
}

		hashtable_iterator& operator++()
			{
    __builtin_trap() /* STUB: not implemented */;
}

		hashtable_iterator operator++(int)
			{
    __builtin_trap() /* STUB: not implemented */;
}

		const node_type* get_node() const
			{
    __builtin_trap() /* STUB: not implemented */;
}

	}; // hashtable_iterator




	/// ht_distance
	///
	/// This function returns the same thing as distance() for 
	/// forward iterators but returns zero for input iterators.
	/// The reason why is that input iterators can only be read
	/// once, and calling distance() on an input iterator destroys
	/// the ability to read it. This ht_distance is used only for
	/// optimization and so the code will merely work better with
	/// forward iterators that input iterators.
	///
	template <typename Iterator>
	inline typename eastl::iterator_traits<Iterator>::difference_type
	distance_fw_impl(Iterator /*first*/, Iterator /*last*/, eastl::input_iterator_tag)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename Iterator>
	inline typename eastl::iterator_traits<Iterator>::difference_type
	distance_fw_impl(Iterator first, Iterator last, eastl::forward_iterator_tag)
		{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename Iterator>
	inline typename eastl::iterator_traits<Iterator>::difference_type
	ht_distance(Iterator first, Iterator last)
	{
    __builtin_trap() /* STUB: not implemented */;
}




	/// mod_range_hashing
	///
	/// Implements the algorithm for conversion of a number in the range of
	/// [0, SIZE_T_MAX] to the range of [0, BucketCount).
	///
	struct mod_range_hashing
	{
		uint32_t operator()(size_t r, uint32_t n) const
			{
    __builtin_trap() /* STUB: not implemented */;
}
	};


	/// default_ranged_hash
	///
	/// Default ranged hash function H. In principle it should be a
	/// function object composed from objects of type H1 and H2 such that
	/// h(k, n) = h2(h1(k), n), but that would mean making extra copies of
	/// h1 and h2. So instead we'll just use a tag to tell class template
	/// hashtable to do that composition.
	///
	/// Note: For all containers where hashtable is a base type this is the
	/// only possible H (users can't specify H).
	struct default_ranged_hash{ };


	/// prime_rehash_policy
	///
	/// Default value for rehash policy. Bucket size is (usually) the
	/// smallest prime that keeps the load factor small enough.
	///
	struct EASTL_API prime_rehash_policy
	{
	public:
		float            mfMaxLoadFactor;
		float            mfGrowthFactor;
		mutable uint32_t mnNextResize;

	public:
		prime_rehash_policy(float fMaxLoadFactor = 1.f)
			: mfMaxLoadFactor(fMaxLoadFactor), mfGrowthFactor(2.f), mnNextResize(0) {
    __builtin_trap() /* STUB: not implemented */;
}

		float GetMaxLoadFactor() const
			{
    __builtin_trap() /* STUB: not implemented */;
}

		/// Return a bucket count no greater than nBucketCountHint, 
		/// Don't update member variables while at it.
		static uint32_t GetPrevBucketCountOnly(uint32_t nBucketCountHint);

		/// Return a bucket count no greater than nBucketCountHint.
		/// This function has a side effect of updating mnNextResize.
		uint32_t GetPrevBucketCount(uint32_t nBucketCountHint) const;

		/// Return a bucket count no smaller than nBucketCountHint.
		/// This function has a side effect of updating mnNextResize.
		uint32_t GetNextBucketCount(uint32_t nBucketCountHint) const;

		/// Return a bucket count appropriate for nElementCount elements.
		/// This function has a side effect of updating mnNextResize.
		uint32_t GetBucketCount(uint32_t nElementCount) const;

		/// nBucketCount is current bucket count, nElementCount is current element count,
		/// and nElementAdd is number of elements to be inserted. Do we need 
		/// to increase bucket count? If so, return pair(true, n), where 
		/// n is the new bucket count. If not, return pair(false, 0).
		eastl::pair<bool, uint32_t>
		GetRehashRequired(uint32_t nBucketCount, uint32_t nElementCount, uint32_t nElementAdd) const;
	};





	///////////////////////////////////////////////////////////////////////
	// Base classes for hashtable. We define these base classes because 
	// in some cases we want to do different things depending on the 
	// value of a policy class. In some cases the policy class affects
	// which member functions and nested typedefs are defined; we handle that
	// by specializing base class templates. Several of the base class templates
	// need to access other members of class template hashtable, so we use
	// the "curiously recurring template pattern" (parent class is templated 
	// on type of child class) for them.
	///////////////////////////////////////////////////////////////////////


	/// rehash_base
	///
	/// Give hashtable the get_max_load_factor functions if the rehash 
	/// policy is prime_rehash_policy.
	///
	template <typename RehashPolicy, typename Hashtable>
	struct rehash_base { };

	template <typename Hashtable>
	struct rehash_base<prime_rehash_policy, Hashtable>
	{
		// Returns the max load factor, which is the load factor beyond
		// which we rebuild the container with a new bucket count.
		float get_max_load_factor() const
		{
    __builtin_trap() /* STUB: not implemented */;
}

		// If you want to make the hashtable never rehash (resize), 
		// set the max load factor to be a very high number (e.g. 100000.f).
		void set_max_load_factor(float fMaxLoadFactor)
		{
    __builtin_trap() /* STUB: not implemented */;
}
	};




	/// hash_code_base
	///
	/// Encapsulates two policy issues that aren't quite orthogonal.
	///   (1) The difference between using a ranged hash function and using
	///       the combination of a hash function and a range-hashing function.
	///       In the former case we don't have such things as hash codes, so
	///       we have a dummy type as placeholder.
	///   (2) Whether or not we cache hash codes. Caching hash codes is
	///       meaningless if we have a ranged hash function. This is because
	///       a ranged hash function converts an object directly to its
	///       bucket index without ostensibly using a hash code.
	/// We also put the key extraction and equality comparison function 
	/// objects here, for convenience.
	/// 
	/// Key is unused because we now support heterogenous lookup, so we can't
	/// assume the key type is the same as the hashtable's Key type parameter.
	///
	template <typename /* unused */ Key, typename Value, typename ExtractKey, typename Equal,
			  typename H1, typename H2, typename H, bool bCacheHashCode>
	struct hash_code_base;


	/// hash_code_base
	///
	/// Specialization: ranged hash function, no caching hash codes. 
	/// H1 and H2 are provided but ignored. We define a dummy hash code type.
	///
	/// Note: Never instantiated because for all containers where hashtable
	/// is a base type this is the only possible H (users can't specify H).
	template <typename /* unused */ Key, typename Value, typename ExtractKey, typename Equal, typename H1, typename H2, typename H>
	struct hash_code_base<Key, Value, ExtractKey, Equal, H1, H2, H, false>
	{
	protected:
		ExtractKey  mExtractKey;    // To do: Make this member go away entirely, as it never has any data.
		Equal       mEqual;         // To do: Make this instance use zero space when it is zero size.
		H           mRangedHash;    // To do: Make this instance use zero space when it is zero size

	public:
		H1 hash_function() const
			{
    __builtin_trap() /* STUB: not implemented */;
}

		const Equal& key_eq() const
			{
    __builtin_trap() /* STUB: not implemented */;
}

		Equal& key_eq()
			{
    __builtin_trap() /* STUB: not implemented */;
}

	protected:
		typedef void*    hash_code_t;
		typedef uint32_t bucket_index_t;

		hash_code_base(const ExtractKey& extractKey, const Equal& eq, const H1&, const H2&, const H& h)
			: mExtractKey(extractKey), mEqual(eq), mRangedHash(h) {
    __builtin_trap() /* STUB: not implemented */;
}

		template<typename KeyX>
		hash_code_t get_hash_code(const KeyX&) const
		{
    __builtin_trap() /* STUB: not implemented */;
}

		bucket_index_t bucket_index(hash_code_t, uint32_t) const
			{
    __builtin_trap() /* STUB: not implemented */;
}

		template<typename KeyX>
		bucket_index_t bucket_index(const KeyX& key, hash_code_t, uint32_t nBucketCount) const
			{
    __builtin_trap() /* STUB: not implemented */;
}

		bucket_index_t bucket_index(const hash_node<Value, false>* pNode, uint32_t nBucketCount) const
			{
    __builtin_trap() /* STUB: not implemented */;
}

		template<typename KeyX>
		bool compare(const KeyX& key, hash_code_t, hash_node<Value, false>* pNode) const
			{
    __builtin_trap() /* STUB: not implemented */;
}

		void copy_code(hash_node<Value, false>*, const hash_node<Value, false>*) const
			{
    __builtin_trap() /* STUB: not implemented */;
} // Nothing to do.

		void set_code(hash_node<Value, false>* pDest, hash_code_t c) const
		{
    __builtin_trap() /* STUB: not implemented */;
}

		void base_swap(hash_code_base& x)
		{
    __builtin_trap() /* STUB: not implemented */;
}

	}; // hash_code_base



	// No specialization for ranged hash function while caching hash codes.
	// That combination is meaningless, and trying to do it is an error.


	/// hash_code_base
	///
	/// Specialization: ranged hash function, cache hash codes. 
	/// This combination is meaningless, so we provide only a declaration
	/// and no definition.
	///
	/// Note: Never instantiated because for all containers where hashtable
	/// is a base type this is the only possible H (users can't specify H).
	template <typename Key, typename Value, typename ExtractKey, typename Equal, typename H1, typename H2, typename H>
	struct hash_code_base<Key, Value, ExtractKey, Equal, H1, H2, H, true>;



	/// hash_code_base
	///
	/// Specialization: hash function and range-hashing function, 
	/// no caching of hash codes. H is provided but ignored. 
	/// Provides typedef and accessor required by TR1.
	///
	template <typename /* unused */ Key, typename Value, typename ExtractKey, typename Equal, typename H1, typename H2>
	struct hash_code_base<Key, Value, ExtractKey, Equal, H1, H2, default_ranged_hash, false>
	{
	protected:
		ExtractKey  mExtractKey;
		Equal       mEqual;
		H1          m_h1;
		H2          m_h2;

	public:
		typedef H1 hasher;

		H1 hash_function() const
			{
    __builtin_trap() /* STUB: not implemented */;
}

		const Equal& key_eq() const
			{
    __builtin_trap() /* STUB: not implemented */;
}

		Equal& key_eq()
			{
    __builtin_trap() /* STUB: not implemented */;
}

	protected:
		typedef size_t hash_code_t;
		typedef uint32_t bucket_index_t;
		typedef hash_node<Value, false> node_type;

		hash_code_base(const ExtractKey& ex, const Equal& eq, const H1& h1, const H2& h2, const default_ranged_hash&)
			: mExtractKey(ex), mEqual(eq), m_h1(h1), m_h2(h2) {
    __builtin_trap() /* STUB: not implemented */;
}

		template<typename KeyX>
		hash_code_t get_hash_code(const KeyX& key) const
			{
    __builtin_trap() /* STUB: not implemented */;
}

		bucket_index_t bucket_index(hash_code_t c, uint32_t nBucketCount) const
			{
    __builtin_trap() /* STUB: not implemented */;
}

		template<typename KeyX>
		bucket_index_t bucket_index(const KeyX&, hash_code_t c, uint32_t nBucketCount) const
			{
    __builtin_trap() /* STUB: not implemented */;
}

		bucket_index_t bucket_index(const node_type* pNode, uint32_t nBucketCount) const
			{
    __builtin_trap() /* STUB: not implemented */;
}

		template<typename KeyX>
		bool compare(const KeyX& key, hash_code_t, node_type* pNode) const
			{
    __builtin_trap() /* STUB: not implemented */;
}

		void copy_code(node_type*, const node_type*) const
			{
    __builtin_trap() /* STUB: not implemented */;
} // Nothing to do.

		void set_code(node_type*, hash_code_t) const
			{
    __builtin_trap() /* STUB: not implemented */;
} // Nothing to do.

		void base_swap(hash_code_base& x)
		{
    __builtin_trap() /* STUB: not implemented */;
}

	}; // hash_code_base



	/// hash_code_base
	///
	/// Specialization: hash function and range-hashing function, 
	/// caching hash codes. H is provided but ignored. 
	/// Provides typedef and accessor required by TR1.
	///
	template <typename /* unused */ Key, typename Value, typename ExtractKey, typename Equal, typename H1, typename H2>
	struct hash_code_base<Key, Value, ExtractKey, Equal, H1, H2, default_ranged_hash, true>
	{
	protected:
		ExtractKey  mExtractKey;
		Equal       mEqual;
		H1          m_h1;
		H2          m_h2;

	public:
		typedef H1 hasher;

		H1 hash_function() const
			{
    __builtin_trap() /* STUB: not implemented */;
}

		const Equal& key_eq() const
			{
    __builtin_trap() /* STUB: not implemented */;
}

		Equal& key_eq()
			{
    __builtin_trap() /* STUB: not implemented */;
}

	protected:
		typedef uint32_t hash_code_t;
		typedef uint32_t bucket_index_t;
		typedef hash_node<Value, true> node_type;

		hash_code_base(const ExtractKey& ex, const Equal& eq, const H1& h1, const H2& h2, const default_ranged_hash&)
			: mExtractKey(ex), mEqual(eq), m_h1(h1), m_h2(h2) {
    __builtin_trap() /* STUB: not implemented */;
}

		template<typename KeyX>
		hash_code_t get_hash_code(const KeyX& key) const
			{
    __builtin_trap() /* STUB: not implemented */;
}

		bucket_index_t bucket_index(hash_code_t c, uint32_t nBucketCount) const
			{
    __builtin_trap() /* STUB: not implemented */;
}

		template<typename KeyX>
		bucket_index_t bucket_index(const KeyX&, hash_code_t c, uint32_t nBucketCount) const
			{
    __builtin_trap() /* STUB: not implemented */;
}

		bucket_index_t bucket_index(const node_type* pNode, uint32_t nBucketCount) const
			{
    __builtin_trap() /* STUB: not implemented */;
}

		template<typename KeyX>
		bool compare(const KeyX& key, hash_code_t c, node_type* pNode) const
			{
    __builtin_trap() /* STUB: not implemented */;
}

		void copy_code(node_type* pDest, const node_type* pSource) const
			{
    __builtin_trap() /* STUB: not implemented */;
}

		void set_code(node_type* pDest, hash_code_t c) const
			{
    __builtin_trap() /* STUB: not implemented */;
}

		void base_swap(hash_code_base& x)
		{
    __builtin_trap() /* STUB: not implemented */;
}

	}; // hash_code_base



	namespace internal {

	// Equality and the hash comparison must both be transparent.
	// The hash function we use is dependent on whether we use the default_ranged_hash or not.
	//
	// Note: For all containers where hashtable is a base type this default_ranged_hash the
	// only possible H (users can't specify H).
	template<typename EqX, typename H1X, typename HX>
	struct is_transparent_key_available : eastl::bool_constant<eastl::detail::is_transparent_comparison_v<EqX>
		&& ((eastl::is_same_v<HX, default_ranged_hash> && eastl::detail::is_transparent_comparison_v<H1X>)
			|| (!eastl::is_same_v<HX, default_ranged_hash> && eastl::detail::is_transparent_comparison_v<HX>))> {};

	template<typename EqX, typename H1X, typename HX>
	EA_CONSTEXPR bool is_transparent_key_available_v = is_transparent_key_available<EqX, H1X, HX>::value;

	} // namespace internal




	///////////////////////////////////////////////////////////////////////////
	/// hashtable
	///
	/// Key and Value: arbitrary CopyConstructible types.
	///
	/// ExtractKey: function object that takes a object of type Value
	/// and returns a value of type Key.
	///
	/// Equal: function object that takes two objects of type k and returns
	/// a bool-like value that is true if the two objects are considered equal.
	///
	/// H1: a hash function. A unary function object with argument type
	/// Key and result type size_t. Return values should be distributed
	/// over the entire range [0, numeric_limits<uint32_t>::max()].
	///
	/// H2: a range-hashing function (in the terminology of Tavori and
	/// Dreizin). This is a function which takes the output of H1 and 
	/// converts it to the range of [0, n]. Usually it merely takes the
	/// output of H1 and mods it to n.
	///
	/// H: a ranged hash function (Tavori and Dreizin). This is merely
	/// a class that combines the functionality of H1 and H2 together, 
	/// possibly in some way that is somehow improved over H1 and H2
	/// It is a binary function whose argument types are Key and size_t 
	/// and whose result type is uint32_t. Given arguments k and n, the 
	/// return value is in the range [0, n). Default: h(k, n) = h2(h1(k), n). 
	/// If H is anything other than the default, H1 and H2 are ignored, 
	/// as H is thus overriding H1 and H2.
	///
	/// RehashPolicy: Policy class with three members, all of which govern
	/// the bucket count. nBucket(n) returns a bucket count no smaller
	/// than n. GetBucketCount(n) returns a bucket count appropriate
	/// for an element count of n. GetRehashRequired(nBucketCount, nElementCount, nElementAdd)
	/// determines whether, if the current bucket count is nBucket and the
	/// current element count is nElementCount, we need to increase the bucket
	/// count. If so, returns pair(true, n), where n is the new
	/// bucket count. If not, returns pair(false, <anything>).
	///
	/// Currently it is hard-wired that the number of buckets never
	/// shrinks. Should we allow RehashPolicy to change that?
	///
	/// bCacheHashCode: true if we store the value of the hash
	/// function along with the value. This is a time-space tradeoff.
	/// Storing it may improve lookup speed by reducing the number of 
	/// times we need to call the Equal function.
	///
	/// bMutableIterators: true if hashtable::iterator is a mutable
	/// iterator, false if iterator and const_iterator are both const 
	/// iterators. This is true for hash_map and hash_multimap,
	/// false for hash_set and hash_multiset.
	///
	/// bUniqueKeys: true if the return value of hashtable::count(k)
	/// is always at most one, false if it may be an arbitrary number. 
	/// This is true for hash_set and hash_map and is false for 
	/// hash_multiset and hash_multimap.
	///
	///////////////////////////////////////////////////////////////////////
	/// Note:
	/// If you want to make a hashtable never increase its bucket usage,
	/// call set_max_load_factor with a very high value such as 100000.f.
	///
	/// Heterogeneous lookup, insertion and erasure
	/// See
	/// https://en.cppreference.com/w/cpp/utility/functional#Transparent_function_objects
	/// https://en.cppreference.com/w/cpp/utility/functional/less_void
	/// https://en.cppreference.com/w/cpp/container/map/find
	/// 
	/// You can avoid creating key objects when calling member functions
	/// with a key_type parameter by declaring the container with
	/// transparent comparison types and passing objects to be passed to
	/// these function objects.
	/// 
	/// This optimization is supported for member functions that take a
	/// key_type parameter, ie. heterogeneous lookup, insertion and erasure,
	/// not just find().
	/// 
	/// Using transparent types is safer than using find_as because the
	/// latter requires the user specify function objects which must have
	/// the same semantics as the container's function objects, otherwise
	/// the behaviour is undefined.
	/// 
	/// find_as
	/// Note: Prefer heterogeneous lookup (see above).
	/// 
	/// In order to support the ability to have a hashtable of strings but
	/// be able to do efficiently lookups via char pointers (i.e. so they 
	/// aren't converted to string objects), we provide the find_as 
	/// function. This function allows you to do a find with a key of a
	/// type other than the hashtable key type. See the find_as function
	/// for more documentation on this.
	///
	/// find_by_hash
	/// In the interest of supporting fast operations wherever possible,
	/// we provide a find_by_hash function which finds a node using its
	/// hash code.  This is useful for cases where the node's hash is
	/// already known, allowing us to avoid a redundant hash operation
	/// in the normal find path.
	/// 
	template <typename Key, typename Value, typename Allocator, typename ExtractKey, 
			  typename Equal, typename H1, typename H2, typename H, 
			  typename RehashPolicy, bool bCacheHashCode, bool bMutableIterators, bool bUniqueKeys>
	class hashtable
		:   public rehash_base<RehashPolicy, hashtable<Key, Value, Allocator, ExtractKey, Equal, H1, H2, H, RehashPolicy, bCacheHashCode, bMutableIterators, bUniqueKeys> >,
			public hash_code_base<Key, Value, ExtractKey, Equal, H1, H2, H, bCacheHashCode>
	{
	public:
		typedef Key                                                                                 key_type;
		typedef Value                                                                               value_type;
		typedef typename ExtractKey::result_type                                                    mapped_type;
		typedef hash_code_base<Key, Value, ExtractKey, Equal, H1, H2, H, bCacheHashCode>            hash_code_base_type;
		typedef typename hash_code_base_type::hash_code_t                                           hash_code_t;
		typedef Allocator                                                                           allocator_type;
		typedef Equal                                                                               key_equal;
		typedef ptrdiff_t                                                                           difference_type;
		typedef eastl_size_t                                                                        size_type;     // See config.h for the definition of eastl_size_t, which defaults to size_t.
		typedef value_type&                                                                         reference;
		typedef const value_type&                                                                   const_reference;
		typedef node_iterator<value_type, !bMutableIterators, bCacheHashCode>                       local_iterator;
		typedef node_iterator<value_type, true,               bCacheHashCode>                       const_local_iterator;
		typedef hashtable_iterator<value_type, !bMutableIterators, bCacheHashCode>                  iterator;
		typedef hashtable_iterator<value_type, true,               bCacheHashCode>                  const_iterator;
		typedef hash_node<value_type, bCacheHashCode>                                               node_type;
		typedef typename conditional<bUniqueKeys, eastl::pair<iterator, bool>, iterator>::type      insert_return_type;
		typedef hashtable<Key, Value, Allocator, ExtractKey, Equal, H1, H2, H, 
							RehashPolicy, bCacheHashCode, bMutableIterators, bUniqueKeys>           this_type;
		typedef RehashPolicy                                                                        rehash_policy_type;
		typedef ExtractKey                                                                          extract_key_type;
		typedef H1                                                                                  h1_type;
		typedef H2                                                                                  h2_type;
		typedef H                                                                                   h_type;
		typedef integral_constant<bool, bUniqueKeys>                                                has_unique_keys_type;

		using hash_code_base_type::key_eq;
		using hash_code_base_type::hash_function;
		using hash_code_base_type::mExtractKey;
		using hash_code_base_type::get_hash_code;
		using hash_code_base_type::bucket_index;
		using hash_code_base_type::compare;
		using hash_code_base_type::set_code;
		using hash_code_base_type::copy_code;

		static const bool kCacheHashCode = bCacheHashCode;

	protected:
		node_type**     mpBucketArray;
		size_type       mnBucketCount;
		size_type       mnElementCount;
		RehashPolicy    mRehashPolicy;  // To do: Use base class optimization to make this go away.
		allocator_type  mAllocator;     // To do: Use base class optimization to make this go away.

		struct NodeFindKeyData {
			node_type* node;
			hash_code_t code;
			size_type bucket_index;
		};

	public:
		hashtable(size_type nBucketCount, const H1&, const H2&, const H&, const Equal&, const ExtractKey&, 
				  const allocator_type& allocator = EASTL_HASHTABLE_DEFAULT_ALLOCATOR);
		
		// note: standard only requires InputIterator.
		template <typename FowardIterator>
		hashtable(FowardIterator first, FowardIterator last, size_type nBucketCount, 
				  const H1&, const H2&, const H&, const Equal&, const ExtractKey&, 
				  const allocator_type& allocator = EASTL_HASHTABLE_DEFAULT_ALLOCATOR); 
		
		hashtable(const hashtable& x);

		// initializer_list ctor support is implemented in subclasses (e.g. hash_set).
		// hashtable(initializer_list<value_type>, size_type nBucketCount, const H1&, const H2&, const H&, 
		//           const Equal&, const ExtractKey&, const allocator_type& allocator = EASTL_HASHTABLE_DEFAULT_ALLOCATOR);

		hashtable(this_type&& x);
		hashtable(this_type&& x, const allocator_type& allocator);
	   ~hashtable();

		const allocator_type& get_allocator() const EA_NOEXCEPT;
		allocator_type&       get_allocator() EA_NOEXCEPT;
		void                  set_allocator(const allocator_type& allocator);

		this_type& operator=(const this_type& x);
		this_type& operator=(std::initializer_list<value_type> ilist);
		this_type& operator=(this_type&& x);

		void swap(this_type& x);

		iterator begin() EA_NOEXCEPT
		{
    __builtin_trap() /* STUB: not implemented */;
}

		const_iterator begin() const EA_NOEXCEPT
		{
    __builtin_trap() /* STUB: not implemented */;
}

		const_iterator cbegin() const EA_NOEXCEPT
			{
    __builtin_trap() /* STUB: not implemented */;
}

		iterator end() EA_NOEXCEPT
			{
    __builtin_trap() /* STUB: not implemented */;
}

		const_iterator end() const EA_NOEXCEPT
			{
    __builtin_trap() /* STUB: not implemented */;
}

		const_iterator cend() const EA_NOEXCEPT
			{
    __builtin_trap() /* STUB: not implemented */;
}

		// Returns an iterator to the first item in bucket n.
		local_iterator begin(size_type n) EA_NOEXCEPT
			{
    __builtin_trap() /* STUB: not implemented */;
}

		const_local_iterator begin(size_type n) const EA_NOEXCEPT
			{
    __builtin_trap() /* STUB: not implemented */;
}

		const_local_iterator cbegin(size_type n) const EA_NOEXCEPT
			{
    __builtin_trap() /* STUB: not implemented */;
}

		// Returns an iterator to the last item in a bucket returned by begin(n).
		local_iterator end(size_type) EA_NOEXCEPT
			{
    __builtin_trap() /* STUB: not implemented */;
}

		const_local_iterator end(size_type) const EA_NOEXCEPT
			{
    __builtin_trap() /* STUB: not implemented */;
}

		const_local_iterator cend(size_type) const EA_NOEXCEPT
			{
    __builtin_trap() /* STUB: not implemented */;
}

		bool empty() const EA_NOEXCEPT
			{
    __builtin_trap() /* STUB: not implemented */;
}

		size_type size() const EA_NOEXCEPT
			{
    __builtin_trap() /* STUB: not implemented */;
}

		// size_type max_size() const EA_NOEXCEPT;

		size_type bucket_count() const EA_NOEXCEPT
			{
    __builtin_trap() /* STUB: not implemented */;
}

		// size_type max_bucket_count() const;

		size_type bucket_size(size_type n) const EA_NOEXCEPT
			{
    __builtin_trap() /* STUB: not implemented */;
}

		//size_type bucket(const key_type& k) const
		//    { return bucket_index(k, (hash code here), (uint32_t)mnBucketCount); }

		// template<typename KX> size_type bucket(const KX& x) const;

		// Returns the ratio of element count to bucket count. A return value of 1 means 
		// there's an optimal 1 bucket for each element.
		float load_factor() const EA_NOEXCEPT
			{
    __builtin_trap() /* STUB: not implemented */;
}

		// float max_load_factor() const;
		// void max_load_factor( float ml );

		// Inherited from the base class.
		// Returns the max load factor, which is the load factor beyond
		// which we rebuild the container with a new bucket count.
		// get_max_load_factor comes from rehash_base.
		//    float get_max_load_factor() const;

		// Inherited from the base class.
		// If you want to make the hashtable never rehash (resize), 
		// set the max load factor to be a very high number (e.g. 100000.f).
		// set_max_load_factor comes from rehash_base.
		//    void set_max_load_factor(float fMaxLoadFactor);

		/// Generalization of get_max_load_factor. This is an extension that's
		/// not present in C++ hash tables (unordered containers).
		const rehash_policy_type& rehash_policy() const EA_NOEXCEPT
			{
    __builtin_trap() /* STUB: not implemented */;
}

		/// Generalization of set_max_load_factor. This is an extension that's
		/// not present in C++ hash tables (unordered containers).
		void rehash_policy(const rehash_policy_type& rehashPolicy);

		template <class... Args>
		insert_return_type emplace(Args&&... args);

		template <class... Args>
		iterator emplace_hint(const_iterator position, Args&&... args);

		insert_return_type                     insert(const value_type& value);
		insert_return_type                     insert(value_type&& otherValue);
		iterator                               insert(const_iterator hint, const value_type& value);
		iterator                               insert(const_iterator hint, value_type&& value);
		void                                   insert(std::initializer_list<value_type> ilist);
		template <typename InputIterator> void insert(InputIterator first, InputIterator last);
	  //insert_return_type                     insert(node_type&& nh);
	  //iterator                               insert(const_iterator hint, node_type&& nh);


		// Non-standard extension
		template <class P> // See comments below for the const value_type& equivalent to this function.
		insert_return_type insert(hash_code_t c, node_type* pNodeNew, P&& otherValue);

		// We provide a version of insert which lets the caller directly specify the hash value and 
		// a potential node to insert if needed. This allows for less thread contention in the case
		// of a thread-shared hash table that's accessed during a mutex lock, because the hash calculation
		// and node creation is done outside of the lock. If pNodeNew is supplied by the user (i.e. non-NULL) 
		// then it must be freeable via the hash table's allocator. If the return value is true then this function 
		// took over ownership of pNodeNew, else pNodeNew is still owned by the caller to free or to pass 
		// to another call to insert. pNodeNew need not be assigned the value by the caller, as the insert
		// function will assign value to pNodeNew upon insertion into the hash table. pNodeNew may be 
		// created by the user with the allocate_uninitialized_node function, and freed by the free_uninitialized_node function.
		insert_return_type insert(hash_code_t c, node_type* pNodeNew, const value_type& value);

		template <class M> eastl::pair<iterator, bool> insert_or_assign(const key_type& k, M&& obj) {
    __builtin_trap() /* STUB: not implemented */;
}
		template <class M> eastl::pair<iterator, bool> insert_or_assign(key_type&& k, M&& obj) {
    __builtin_trap() /* STUB: not implemented */;
}
		template<typename KX, typename M, typename EqX = Equal, typename H1X = H1, typename HX = H,
			eastl::enable_if_t<internal::is_transparent_key_available_v<EqX, H1X, HX>, bool> = true>
		eastl::pair<iterator, bool>						insert_or_assign(KX&& k, M&& obj) {
    __builtin_trap() /* STUB: not implemented */;
}
		template <class M> iterator						insert_or_assign(const_iterator hint, const key_type& k, M&& obj) {
    __builtin_trap() /* STUB: not implemented */;
}
		template <class M> iterator						insert_or_assign(const_iterator hint, key_type&& k, M&& obj) {
    __builtin_trap() /* STUB: not implemented */;
}
		template<typename KX, typename M, typename EqX = Equal, typename H1X = H1, typename HX = H,
			eastl::enable_if_t<internal::is_transparent_key_available_v<EqX, H1X, HX>, bool> = true>
		iterator										insert_or_assign(const_iterator hint, KX&& k, M&& obj) {
    __builtin_trap() /* STUB: not implemented */;
}

		// Used to allocate and free memory used by insert(const value_type& value, hash_code_t c, node_type* pNodeNew).
		node_type* allocate_uninitialized_node();
		void       free_uninitialized_node(node_type* pNode);

		template <typename Iter = iterator, typename eastl::enable_if<!eastl::is_same_v<Iter, const_iterator>, int>::type = 0>
		iterator         erase(iterator position) {
    __builtin_trap() /* STUB: not implemented */;
}
		iterator         erase(const_iterator position);
		iterator         erase(const_iterator first, const_iterator last);
		size_type        erase(const key_type& k) {
    __builtin_trap() /* STUB: not implemented */;
}
		template<typename KX, typename EqX = Equal, typename H1X = H1, typename HX = H,
			eastl::enable_if_t<!eastl::is_convertible_v<KX&&, iterator> && !eastl::is_convertible_v<KX&&, const_iterator>
			&& internal::is_transparent_key_available_v<EqX, H1X, HX>, bool> = true>
		size_type        erase(KX&& k) {
    __builtin_trap() /* STUB: not implemented */;
}

		void clear();
		void clear(bool clearBuckets);                  // If clearBuckets is true, we free the bucket memory and set the bucket count back to the newly constructed count.
		void reset_lose_memory() EA_NOEXCEPT;           // This is a unilateral reset to an initially empty state. No destructors are called, no deallocation occurs.
		void rehash(size_type nBucketCount);
		void reserve(size_type nElementCount);

		iterator       find(const key_type& key) {
    __builtin_trap() /* STUB: not implemented */;
}
		const_iterator find(const key_type& key) const {
    __builtin_trap() /* STUB: not implemented */;
}

		template<typename KX, typename EqX = Equal, typename H1X = H1, typename HX = H,
			eastl::enable_if_t<internal::is_transparent_key_available_v<EqX, H1X, HX>, bool> = true>
		iterator       find(const KX& key) {
    __builtin_trap() /* STUB: not implemented */;
}
		template<typename KX, typename EqX = Equal, typename H1X = H1, typename HX = H,
			eastl::enable_if_t<internal::is_transparent_key_available_v<EqX, H1X, HX>, bool> = true>
		const_iterator find(const KX& key) const {
    __builtin_trap() /* STUB: not implemented */;
}

		bool contains(const key_type& key) const {
    __builtin_trap() /* STUB: not implemented */;
}

		template<typename KX, typename EqX = Equal, typename H1X = H1, typename HX = H,
			eastl::enable_if_t<internal::is_transparent_key_available_v<EqX, H1X, HX>, bool> = true>
		bool contains(const KX& key) const {
    __builtin_trap() /* STUB: not implemented */;
}

		/// Implements a find whereby the user supplies a comparison of a different type
		/// than the hashtable value_type. A useful case of this is one whereby you have
		/// a container of string objects but want to do searches via passing in char pointers.
		/// The problem is that without this kind of find, you need to do the expensive operation
		/// of converting the char pointer to a string so it can be used as the argument to the 
		/// find function.
		///
		/// Example usage (namespaces omitted for brevity):
		///     hash_set<string> hashSet;
		///     hashSet.find_as("hello");    // Use default hash and compare.
		///
		/// Example usage (note that the predicate uses string as first type and char* as second):
		///     hash_set<string> hashSet;
		///     hashSet.find_as("hello", hash<char*>(), equal_to<>());
		///
		template <typename U, typename UHash, typename BinaryPredicate>
		iterator       find_as(const U& u, UHash uhash, BinaryPredicate predicate);

		template <typename U, typename UHash, typename BinaryPredicate>
		const_iterator find_as(const U& u, UHash uhash, BinaryPredicate predicate) const;

		// Using default hash and equality objects may result in incorrect semantics (undefined behaviour).
		// Use find() with heterogenous lookup (ie. function objects with a is_transparent type member) or explicitly specify hash and equality objects.
		// See doc\BestPractices.md#search-hash_mapstring-using-heterogeneous-lookup
		template <typename U>
		EA_REMOVE_AT_2025_OCT_MSG("Use heterogeneous lookup instead (see EASTL Best Practices page) or explicitly specify hash and equality objects.")
		iterator       find_as(const U& u);

		// Using default hash and equality objects may result in incorrect semantics (undefined behaviour).
		// Use find() with heterogenous lookup (ie. function objects with a is_transparent type member) or explicitly specify hash and equality objects.
		// See doc\BestPractices.md#search-hash_mapstring-using-heterogeneous-lookup
		template <typename U>
		EA_REMOVE_AT_2025_OCT_MSG("Use heterogeneous lookup instead (see EASTL Best Practices page) or explicitly specify hash and equality objects.")
		const_iterator find_as(const U& u) const;

		// Note: find_by_hash and find_range_by_hash both perform a search based on a hash value.
		// It is important to note that multiple hash values may map to the same hash bucket, so
		// it would be incorrect to assume all items returned match the hash value that
		// was searched for.

		/// Implements a find whereby the user supplies the node's hash code.
		/// It returns an iterator to the first element that matches the given hash. However, there may be multiple elements that match the given hash.

		template<typename HashCodeT>
		ENABLE_IF_HASHCODE_EASTLSIZET(HashCodeT, iterator) find_by_hash(HashCodeT c)
		{
    __builtin_trap() /* STUB: not implemented */;
}

		template<typename HashCodeT>
		ENABLE_IF_HASHCODE_EASTLSIZET(HashCodeT, const_iterator) find_by_hash(HashCodeT c) const
		{
    __builtin_trap() /* STUB: not implemented */;
}

		iterator find_by_hash(const key_type& k, hash_code_t c)
		{
    __builtin_trap() /* STUB: not implemented */;
}

		const_iterator find_by_hash(const key_type& k, hash_code_t c) const
		{
    __builtin_trap() /* STUB: not implemented */;
}

		// todo: heterogeneous find_by_hash

		// Returns a pair that allows iterating over all nodes in a hash bucket
		//   first in the pair returned holds the iterator for the beginning of the bucket,
		//   second in the pair returned holds the iterator for the end of the bucket,
		// If no bucket is found, both values in the pair are set to end().
		//
		// See also the note above.
		eastl::pair<iterator, iterator> find_range_by_hash(hash_code_t c);
		eastl::pair<const_iterator, const_iterator> find_range_by_hash(hash_code_t c) const;

		size_type count(const key_type& k) const EA_NOEXCEPT {
    __builtin_trap() /* STUB: not implemented */;
}

		template<typename KX, typename EqX = Equal, typename H1X = H1, typename HX = H,
			eastl::enable_if_t<internal::is_transparent_key_available_v<EqX, H1X, HX>, bool> = true>
		size_type count(const KX& key) const EA_NOEXCEPT {
    __builtin_trap() /* STUB: not implemented */;
}

		eastl::pair<iterator, iterator>             equal_range(const key_type& k) {
    __builtin_trap() /* STUB: not implemented */;
}
		eastl::pair<const_iterator, const_iterator> equal_range(const key_type& k) const {
    __builtin_trap() /* STUB: not implemented */;
}

		template<typename KX, typename EqX = Equal, typename H1X = H1, typename HX = H,
			eastl::enable_if_t<internal::is_transparent_key_available_v<EqX, H1X, HX>, bool> = true>
		eastl::pair<iterator, iterator>             equal_range(const KX& k) {
    __builtin_trap() /* STUB: not implemented */;
}
		template<typename KX, typename EqX = Equal, typename H1X = H1, typename HX = H,
			eastl::enable_if_t<internal::is_transparent_key_available_v<EqX, H1X, HX>, bool> = true>
		eastl::pair<const_iterator, const_iterator> equal_range(const KX& k) const {
    __builtin_trap() /* STUB: not implemented */;
}

		bool validate() const;
		int  validate_iterator(const_iterator i) const;

	protected:
		// We must remove one of the 'DoGetResultIterator' overloads from the overload-set (via SFINAE) because both can
		// not compile successfully at the same time. The 'bUniqueKeys' template parameter chooses at compile-time the
		// type of 'insert_return_type' between a pair<iterator,bool> and a raw iterator. We must pick between the two
		// overloads that unpacks the iterator from the pair or simply passes the provided iterator to the caller based
		// on the class template parameter.
		template <typename BoolConstantT>
		iterator DoGetResultIterator(BoolConstantT,
		                             const insert_return_type& irt,
		                             ENABLE_IF_TRUETYPE(BoolConstantT) = nullptr) const EA_NOEXCEPT
		{
    __builtin_trap() /* STUB: not implemented */;
}

		template <typename BoolConstantT>
		iterator DoGetResultIterator(BoolConstantT,
		                             const insert_return_type& irt,
		                             DISABLE_IF_TRUETYPE(BoolConstantT) = nullptr) const EA_NOEXCEPT
		{
    __builtin_trap() /* STUB: not implemented */;
}

		// Note: only usable in hash_map / hash_multimap because this function calls: value_type(pair_first_construct, key)
		node_type*  DoAllocateNodeFromKey(const key_type& key);
		// Note: only usable in hash_map / hash_multimap because this function calls: value_type(pair_first_construct, eastl::move(key))
		node_type*  DoAllocateNodeFromKey(key_type&& key);
		void        DoFreeNode(node_type* pNode);
		void        DoFreeNodes(node_type** pBucketArray, size_type);

		node_type** DoAllocateBuckets(size_type n);
		void        DoFreeBuckets(node_type** pBucketArray, size_type n);

		template <bool bDeleteOnException, typename Enabled = bool_constant<bUniqueKeys>, ENABLE_IF_TRUETYPE(Enabled) = nullptr> // only enabled when keys are unique
		eastl::pair<iterator, bool> DoInsertUniqueNode(const key_type& k, hash_code_t c, size_type n, node_type* pNodeNew);

		// this overload will always allocate a node.
		template <typename BoolConstantT, class... Args, ENABLE_IF_TRUETYPE(BoolConstantT) = nullptr>
		eastl::pair<iterator, bool> DoInsertValue(BoolConstantT, Args&&... args);

		// this overload will always allocate a node.
		template <typename BoolConstantT, class... Args, DISABLE_IF_TRUETYPE(BoolConstantT) = nullptr>
		iterator DoInsertValue(BoolConstantT, Args&&... args);


		template <typename BoolConstantT>
		eastl::pair<iterator, bool> DoInsertValueExtra(BoolConstantT,
													   const key_type& k,
													   hash_code_t c,
													   node_type* pNodeNew,
													   value_type&& value,
													   ENABLE_IF_TRUETYPE(BoolConstantT) = nullptr);

		// this overload won't allocate a node if an element with the same key exists.
		template <typename BoolConstantT>
		eastl::pair<iterator, bool> DoInsertValue(BoolConstantT,
												  value_type&& value,
												  ENABLE_IF_TRUETYPE(BoolConstantT) = nullptr);

		// this overload won't allocate a node if an element with the same key exists.
		template <typename BoolConstantT>
		eastl::pair<iterator, bool> DoInsertValue(BoolConstantT,
												  const value_type&& value,
												  ENABLE_IF_TRUETYPE(BoolConstantT) = nullptr);

		template <typename BoolConstantT>
		iterator DoInsertValueExtra(BoolConstantT,
									const key_type& k,
									hash_code_t c,
									node_type* pNodeNew,
									value_type&& value,
									DISABLE_IF_TRUETYPE(BoolConstantT) = nullptr);

		// this overload won't allocate a node if an element with the same key exists.
		template <typename BoolConstantT>
		iterator DoInsertValue(BoolConstantT, value_type&& value, DISABLE_IF_TRUETYPE(BoolConstantT) = nullptr);

		// this overload won't allocate a node if an element with the same key exists.
		template <typename BoolConstantT>
		iterator DoInsertValue(BoolConstantT, const value_type&& value, DISABLE_IF_TRUETYPE(BoolConstantT) = nullptr);


		template <typename BoolConstantT>
		eastl::pair<iterator, bool> DoInsertValueExtra(BoolConstantT,
													   const key_type& k,
													   hash_code_t c,
													   node_type* pNodeNew,
													   const value_type& value,
													   ENABLE_IF_TRUETYPE(BoolConstantT) = nullptr);

		// this overload won't allocate a node if an element with the same key exists.
		template <typename BoolConstantT>
		eastl::pair<iterator, bool> DoInsertValue(BoolConstantT,
		                                          const value_type& value,
		                                          ENABLE_IF_TRUETYPE(BoolConstantT) = nullptr);

		// this overload won't allocate a node if an element with the same key exists.
		template <typename BoolConstantT>
		eastl::pair<iterator, bool> DoInsertValue(BoolConstantT,
		                                          value_type& value,
		                                          ENABLE_IF_TRUETYPE(BoolConstantT) = nullptr);

		template <typename BoolConstantT>
		iterator DoInsertValueExtra(BoolConstantT,
		                            const key_type& k,
		                            hash_code_t c,
		                            node_type* pNodeNew,
		                            const value_type& value,
		                            DISABLE_IF_TRUETYPE(BoolConstantT) = nullptr);

		// this overload won't allocate a node if an element with the same key exists.
		template <typename BoolConstantT>
		iterator DoInsertValue(BoolConstantT, const value_type& value, DISABLE_IF_TRUETYPE(BoolConstantT) = nullptr);

		// this overload won't allocate a node if an element with the same key exists.
		template <typename BoolConstantT>
		iterator DoInsertValue(BoolConstantT, value_type& value, DISABLE_IF_TRUETYPE(BoolConstantT) = nullptr);

		template <class... Args>
		node_type* DoAllocateNode(Args&&... args);
		node_type* DoAllocateNode(value_type&& value);
		node_type* DoAllocateNode(const value_type& value);

		// DoInsertKey is supposed to get hash_code_t c  = get_hash_code(key).
		// it is done in case application has it's own hashset/hashmap-like containter, where hash code is for some reason known prior the insert
		// this allows to save some performance, especially with heavy hash functions
		// 
		// Note: only usable in hash_map / hash_multimap because this function (transitively) calls: value_type(pair_first_construct, key)
		eastl::pair<iterator, bool> DoInsertKey(true_type, const key_type& key, hash_code_t c);
		iterator                    DoInsertKey(false_type, const key_type& key, hash_code_t c);

		// We keep DoInsertKey overload without third parameter, for compatibility with older revisions of EASTL (3.12.07 and earlier)
		// It used to call get_hash_code as a first call inside the DoInsertKey.
		// 
		// Note: only usable in hash_map / hash_multimap because this function (transitively) calls: value_type(pair_first_construct, key)
		eastl::pair<iterator, bool> DoInsertKey(true_type, const key_type& key)  {
    __builtin_trap() /* STUB: not implemented */;
}
		iterator                    DoInsertKey(false_type, const key_type& key) {
    __builtin_trap() /* STUB: not implemented */;
}

		void       DoRehash(size_type nBucketCount);
		template <typename KX>
		node_type* DoFindNode(node_type* pNode, const KX& k, hash_code_t c) const;
		template <typename KX>
		NodeFindKeyData DoFindKeyData(const KX& k) const;

		template <typename T>
		ENABLE_IF_HAS_HASHCODE(T, node_type) DoFindNode(T* pNode, hash_code_t c) const
		{
    __builtin_trap() /* STUB: not implemented */;
}

		template <typename U, typename BinaryPredicate>
		node_type* DoFindNodeT(node_type* pNode, const U& u, BinaryPredicate predicate) const;

	private:
		template <typename V, typename Enabled = bool_constant<bUniqueKeys>, ENABLE_IF_TRUETYPE(Enabled) = nullptr>
		eastl::pair<iterator, bool> DoInsertValueExtraForwarding(const key_type& k,
														hash_code_t c,
														node_type* pNodeNew,
														V&& value);

		template<typename KX, typename M>
		eastl::pair<iterator, bool> DoInsertOrAssign(KX&& k, M&& obj);
		template<typename KX, typename M>
		iterator					DoInsertOrAssign(const_iterator hint, KX&& k, M&& obj);

		template<typename KX>
		size_type        DoErase(KX&& k);

		template<typename KX>
		iterator DoFind(const KX& key);

		template<typename KX>
		const_iterator DoFind(const KX& key) const;

		template<typename KX>
		size_type DoCount(const KX& key) const EA_NOEXCEPT;

		template<typename KX>
		eastl::pair<iterator, iterator>             DoEqualRange(const KX& k);
		template<typename KX>
		eastl::pair<const_iterator, const_iterator> DoEqualRange(const KX& k) const;

	}; // class hashtable





	///////////////////////////////////////////////////////////////////////
	// node_iterator_base
	///////////////////////////////////////////////////////////////////////

	template <typename Value, bool bCacheHashCode>
	inline bool operator==(const node_iterator_base<Value, bCacheHashCode>& a, const node_iterator_base<Value, bCacheHashCode>& b)
		{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename Value, bool bCacheHashCode>
	inline bool operator!=(const node_iterator_base<Value, bCacheHashCode>& a, const node_iterator_base<Value, bCacheHashCode>& b)
		{
    __builtin_trap() /* STUB: not implemented */;
}




	///////////////////////////////////////////////////////////////////////
	// hashtable_iterator_base
	///////////////////////////////////////////////////////////////////////

	template <typename Value, bool bCacheHashCode>
	inline bool operator==(const hashtable_iterator_base<Value, bCacheHashCode>& a, const hashtable_iterator_base<Value, bCacheHashCode>& b)
		{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename Value, bool bCacheHashCode>
	inline bool operator!=(const hashtable_iterator_base<Value, bCacheHashCode>& a, const hashtable_iterator_base<Value, bCacheHashCode>& b)
		{
    __builtin_trap() /* STUB: not implemented */;
}




	///////////////////////////////////////////////////////////////////////
	// hashtable
	///////////////////////////////////////////////////////////////////////

	template <typename K, typename V, typename A, typename EK, typename Eq,
			  typename H1, typename H2, typename H, typename RP, bool bC, bool bM, bool bU>
	hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>
	::hashtable(size_type nBucketCount, const H1& h1, const H2& h2, const H& h,
				const Eq& eq, const EK& ek, const allocator_type& allocator)
		:   rehash_base<RP, hashtable>(),
			hash_code_base<K, V, EK, Eq, H1, H2, H, bC>(ek, eq, h1, h2, h),
			mnBucketCount(0),
			mnElementCount(0),
			mRehashPolicy(),
			mAllocator(allocator)
	{
    __builtin_trap() /* STUB: not implemented */;
}



	template <typename K, typename V, typename A, typename EK, typename Eq,
			  typename H1, typename H2, typename H, typename RP, bool bC, bool bM, bool bU>
	template <typename FowardIterator>
	hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::hashtable(FowardIterator first, FowardIterator last, size_type nBucketCount, 
																	 const H1& h1, const H2& h2, const H& h, 
																	 const Eq& eq, const EK& ek, const allocator_type& allocator)
		:   rehash_base<rehash_policy_type, hashtable>(),
			hash_code_base<key_type, value_type, extract_key_type, key_equal, h1_type, h2_type, h_type, kCacheHashCode>(ek, eq, h1, h2, h),
		  //mnBucketCount(0), // This gets re-assigned below.
			mnElementCount(0),
			mRehashPolicy(),
			mAllocator(allocator)
	{
    __builtin_trap() /* STUB: not implemented */;
}



	template <typename K, typename V, typename A, typename EK, typename Eq,
			  typename H1, typename H2, typename H, typename RP, bool bC, bool bM, bool bU>
	hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::hashtable(const this_type& x)
		:   rehash_base<RP, hashtable>(x),
			hash_code_base<K, V, EK, Eq, H1, H2, H, bC>(x),
			mnBucketCount(x.mnBucketCount),
			mnElementCount(x.mnElementCount),
			mRehashPolicy(x.mRehashPolicy),
			mAllocator(x.mAllocator)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename A, typename EK, typename Eq,
			  typename H1, typename H2, typename H, typename RP, bool bC, bool bM, bool bU>
	hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::hashtable(this_type&& x)
		:   rehash_base<RP, hashtable>(x),
			hash_code_base<K, V, EK, Eq, H1, H2, H, bC>(x),
			mnBucketCount(0),
			mnElementCount(0),
			mRehashPolicy(x.mRehashPolicy),
			mAllocator(x.mAllocator)
	{
		reset_lose_memory(); // We do this here the same as we do it in the default ctor because it puts the container in a proper initial empty state. This code would be cleaner if we could rely on being able to use C++11 delegating constructors and just call the default ctor here.
		swap(x);
	}


	template <typename K, typename V, typename A, typename EK, typename Eq,
			  typename H1, typename H2, typename H, typename RP, bool bC, bool bM, bool bU>
	hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::hashtable(this_type&& x, const allocator_type& allocator)
		:   rehash_base<RP, hashtable>(x),
			hash_code_base<K, V, EK, Eq, H1, H2, H, bC>(x),
			mnBucketCount(0),
			mnElementCount(0),
			mRehashPolicy(x.mRehashPolicy),
			mAllocator(allocator)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename A, typename EK, typename Eq,
			  typename H1, typename H2, typename H, typename RP, bool bC, bool bM, bool bU>
	inline const typename hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::allocator_type&
	hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::get_allocator() const EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}



	template <typename K, typename V, typename A, typename EK, typename Eq,
			  typename H1, typename H2, typename H, typename RP, bool bC, bool bM, bool bU>
	inline typename hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::allocator_type&
	hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::get_allocator() EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}



	template <typename K, typename V, typename A, typename EK, typename Eq,
			  typename H1, typename H2, typename H, typename RP, bool bC, bool bM, bool bU>
	inline void hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::set_allocator(const allocator_type& allocator)
	{
    __builtin_trap() /* STUB: not implemented */;
}



	template <typename K, typename V, typename A, typename EK, typename Eq,
			  typename H1, typename H2, typename H, typename RP, bool bC, bool bM, bool bU>
	inline typename hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::this_type&
	hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::operator=(const this_type& x)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename A, typename EK, typename Eq,
			  typename H1, typename H2, typename H, typename RP, bool bC, bool bM, bool bU>
	inline typename hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::this_type&
	hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::operator=(this_type&& x)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename A, typename EK, typename Eq,
			  typename H1, typename H2, typename H, typename RP, bool bC, bool bM, bool bU>
	inline typename hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::this_type&
	hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::operator=(std::initializer_list<value_type> ilist)
	{
    __builtin_trap() /* STUB: not implemented */;
}



	template <typename K, typename V, typename A, typename EK, typename Eq,
			  typename H1, typename H2, typename H, typename RP, bool bC, bool bM, bool bU>
	inline hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::~hashtable()
	{
		clear();
		DoFreeBuckets(mpBucketArray, mnBucketCount);
	}


	template <typename K, typename V, typename A, typename EK, typename Eq,
			  typename H1, typename H2, typename H, typename RP, bool bC, bool bM, bool bU>
	typename hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::node_type*
	hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::DoAllocateNodeFromKey(const key_type& key)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename A, typename EK, typename Eq,
				typename H1, typename H2, typename H, typename RP, bool bC, bool bM, bool bU>
	typename hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::node_type*
	hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::DoAllocateNodeFromKey(key_type&& key)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename A, typename EK, typename Eq,
			  typename H1, typename H2, typename H, typename RP, bool bC, bool bM, bool bU>
	inline void hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::DoFreeNode(node_type* pNode)
	{
    __builtin_trap() /* STUB: not implemented */;
}



	template <typename K, typename V, typename A, typename EK, typename Eq,
			  typename H1, typename H2, typename H, typename RP, bool bC, bool bM, bool bU>
	void hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::DoFreeNodes(node_type** pNodeArray, size_type n)
	{
    __builtin_trap() /* STUB: not implemented */;
}



	template <typename K, typename V, typename A, typename EK, typename Eq,
			  typename H1, typename H2, typename H, typename RP, bool bC, bool bM, bool bU>
	typename hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::node_type**
	hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::DoAllocateBuckets(size_type n)
	{
    __builtin_trap() /* STUB: not implemented */;
}



	template <typename K, typename V, typename A, typename EK, typename Eq,
			  typename H1, typename H2, typename H, typename RP, bool bC, bool bM, bool bU>
	inline void hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::DoFreeBuckets(node_type** pBucketArray, size_type n)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename A, typename EK, typename Eq,
			  typename H1, typename H2, typename H, typename RP, bool bC, bool bM, bool bU>
	void hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::swap(this_type& x)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename A, typename EK, typename Eq,
			  typename H1, typename H2, typename H, typename RP, bool bC, bool bM, bool bU>
	inline void hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::rehash_policy(const rehash_policy_type& rehashPolicy)
	{
    __builtin_trap() /* STUB: not implemented */;
}



	template <typename K, typename V, typename A, typename EK, typename Eq,
			  typename H1, typename H2, typename H, typename RP, bool bC, bool bM, bool bU>
	template <typename KX>
	inline typename hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::iterator
	hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::DoFind(const KX& k)
	{
    __builtin_trap() /* STUB: not implemented */;
}



	template <typename K, typename V, typename A, typename EK, typename Eq,
			  typename H1, typename H2, typename H, typename RP, bool bC, bool bM, bool bU>
	template <typename KX>
	inline typename hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::const_iterator
	hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::DoFind(const KX& k) const
	{
    __builtin_trap() /* STUB: not implemented */;
}



	template <typename K, typename V, typename A, typename EK, typename Eq,
			  typename H1, typename H2, typename H, typename RP, bool bC, bool bM, bool bU>
	template <typename U, typename UHash, typename BinaryPredicate>
	inline typename hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::iterator
	hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::find_as(const U& other, UHash uhash, BinaryPredicate predicate)
	{
    __builtin_trap() /* STUB: not implemented */;
}



	template <typename K, typename V, typename A, typename EK, typename Eq,
			  typename H1, typename H2, typename H, typename RP, bool bC, bool bM, bool bU>
	template <typename U, typename UHash, typename BinaryPredicate>
	inline typename hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::const_iterator
	hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::find_as(const U& other, UHash uhash, BinaryPredicate predicate) const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	/// hashtable_find
	///
	/// Deprecated: Using default hash and equality objects may result in
	/// incorrect semantics (undefined behaviour).
	/// Use find() with heterogenous lookup (ie. function objects with a
	/// is_transparent type member) or explicitly specify hash and equality
	/// objects.
	///
	/// Helper function that defaults to using hash<U> and equal_to<>.
	/// This makes it so that by default you don't need to provide these.
	/// Note that the default hash functions may not be what you want, though.
	///
	/// Example usage. Instead of this:
	///     hash_set<string> hashSet;
	///     hashSet.find("hello", hash<char*>(), equal_to<>());
	///
	/// You can use this:
	///     hash_set<string> hashSet;
	///     hashtable_find(hashSet, "hello");
	template <typename H, typename U>
	EA_REMOVE_AT_2025_OCT_MSG("Use heterogeneous lookup instead (see EASTL Best Practices page) or explicitly specify hash and equality objects.")
	inline typename H::iterator hashtable_find(H& hashTable, U u)
		{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename H, typename U>
	EA_REMOVE_AT_2025_OCT_MSG("Use heterogeneous lookup instead (see EASTL Best Practices page) or explicitly specify hash and equality objects.")
	inline typename H::const_iterator hashtable_find(const H& hashTable, U u)
		{
    __builtin_trap() /* STUB: not implemented */;
}



	template <typename K, typename V, typename A, typename EK, typename Eq,
			  typename H1, typename H2, typename H, typename RP, bool bC, bool bM, bool bU>
	template <typename U>
	EA_REMOVE_AT_2025_OCT_MSG("Use heterogeneous lookup instead (see EASTL Best Practices page) or explicitly specify hash and equality objects.")
	inline typename hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::iterator
	hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::find_as(const U& other)
	{
    __builtin_trap() /* STUB: not implemented */;
}
		// VC++ doesn't appear to like the following, though it seems correct to me.
		// So we implement the workaround above until we can straighten this out.
		//{ return find_as(other, eastl::hash<U>(), eastl::equal_to<>()); }


	template <typename K, typename V, typename A, typename EK, typename Eq,
			  typename H1, typename H2, typename H, typename RP, bool bC, bool bM, bool bU>
	template <typename U>
	EA_REMOVE_AT_2025_OCT_MSG("Use heterogeneous lookup instead (see EASTL Best Practices page) or explicitly specify hash and equality objects.")
	inline typename hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::const_iterator
	hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::find_as(const U& other) const
	{
    __builtin_trap() /* STUB: not implemented */;
}
		// VC++ doesn't appear to like the following, though it seems correct to me.
		// So we implement the workaround above until we can straighten this out.
		//{ return find_as(other, eastl::hash<U>(), eastl::equal_to<>()); }



	template <typename K, typename V, typename A, typename EK, typename Eq, 
			  typename H1, typename H2, typename H, typename RP, bool bC, bool bM, bool bU>
	eastl::pair<typename hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::const_iterator,
				typename hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::const_iterator>
	hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::find_range_by_hash(hash_code_t c) const
	{
    __builtin_trap() /* STUB: not implemented */;
}



	template <typename K, typename V, typename A, typename EK, typename Eq, 
			  typename H1, typename H2, typename H, typename RP, bool bC, bool bM, bool bU>
	eastl::pair<typename hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::iterator,
				typename hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::iterator>
	hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::find_range_by_hash(hash_code_t c)
	{
    __builtin_trap() /* STUB: not implemented */;
}



	template <typename K, typename V, typename A, typename EK, typename Eq,
			  typename H1, typename H2, typename H, typename RP, bool bC, bool bM, bool bU>
	template <typename KX>
	typename hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::size_type
	hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::DoCount(const KX& k) const EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}



	template <typename K, typename V, typename A, typename EK, typename Eq,
			  typename H1, typename H2, typename H, typename RP, bool bC, bool bM, bool bU>
	template <typename KX>
	eastl::pair<typename hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::iterator,
				typename hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::iterator>
	hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::DoEqualRange(const KX& k)
	{
    __builtin_trap() /* STUB: not implemented */;
}




	template <typename K, typename V, typename A, typename EK, typename Eq,
			  typename H1, typename H2, typename H, typename RP, bool bC, bool bM, bool bU>
	template <typename KX>
	eastl::pair<typename hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::const_iterator,
				typename hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::const_iterator>
	hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::DoEqualRange(const KX& k) const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename A, typename EK, typename Eq,
			  typename H1, typename H2, typename H, typename RP, bool bC, bool bM, bool bU>
	template <typename KX>
	inline typename hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::NodeFindKeyData
	hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::DoFindKeyData(const KX& k) const {
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename K, typename V, typename A, typename EK, typename Eq,
			  typename H1, typename H2, typename H, typename RP, bool bC, bool bM, bool bU>
	template <typename KX>
	inline typename hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::node_type* 
	hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::DoFindNode(node_type* pNode, const KX& k, hash_code_t c) const
	{
    __builtin_trap() /* STUB: not implemented */;
}



	template <typename K, typename V, typename A, typename EK, typename Eq,
			  typename H1, typename H2, typename H, typename RP, bool bC, bool bM, bool bU>
	template <typename U, typename BinaryPredicate>
	inline typename hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::node_type* 
	hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::DoFindNodeT(node_type* pNode, const U& other, BinaryPredicate predicate) const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename A, typename EK, typename Eq,
			  typename H1, typename H2, typename H, typename RP, bool bC, bool bM, bool bU>
	template <bool bDeleteOnException, typename Enabled, ENABLE_IF_TRUETYPE(Enabled)> // only enabled when keys are unique
	eastl::pair<typename hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::iterator, bool>
	hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::DoInsertUniqueNode(const key_type& k, hash_code_t c, size_type n, node_type* pNodeNew)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename K, typename V, typename A, typename EK, typename Eq,
			  typename H1, typename H2, typename H, typename RP, bool bC, bool bM, bool bU>
	template <typename BoolConstantT, class... Args, ENABLE_IF_TRUETYPE(BoolConstantT)>
	eastl::pair<typename hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::iterator, bool>
	hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::DoInsertValue(BoolConstantT, Args&&... args) // true_type means bUniqueKeys is true.
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename A, typename EK, typename Eq,
			  typename H1, typename H2, typename H, typename RP, bool bC, bool bM, bool bU>
	template <typename BoolConstantT, class... Args, DISABLE_IF_TRUETYPE(BoolConstantT)>
	typename hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::iterator
	hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::DoInsertValue(BoolConstantT, Args&&... args) // false_type means bUniqueKeys is false.
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename A, typename EK, typename Eq,
			  typename H1, typename H2, typename H, typename RP, bool bC, bool bM, bool bU>
	template <class... Args>
	typename hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::node_type*
	hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::DoAllocateNode(Args&&... args)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	////////////////////////////////////////////////////////////////////////////////////////////////////
	// Note: The following insertion-related functions are nearly copies of the above three functions,
	// but are for value_type&& and const value_type& arguments. It's useful for us to have the functions
	// below, even when using a fully compliant C++11 compiler that supports the above functions. 
	// The reason is because the specializations below are slightly more efficient because they can delay
	// the creation of a node until it's known that it will be needed.
	////////////////////////////////////////////////////////////////////////////////////////////////////
	template <typename K, typename V, typename A, typename EK, typename Eq,
			  typename H1, typename H2, typename H, typename RP, bool bC, bool bM, bool bU>
	template <typename BoolConstantT>
	inline eastl::pair<typename hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::iterator, bool>
	hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::DoInsertValueExtra(BoolConstantT, const key_type& k,
		hash_code_t c, node_type* pNodeNew, value_type&& value, ENABLE_IF_TRUETYPE(BoolConstantT)) // true_type means bUniqueKeys is true.
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename K, typename V, typename A, typename EK, typename Eq,
			  typename H1, typename H2, typename H, typename RP, bool bC, bool bM, bool bU>
	template <typename BoolConstantT>
	inline eastl::pair<typename hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::iterator, bool>
	hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::DoInsertValueExtra(BoolConstantT, const key_type& k,
		hash_code_t c, node_type* pNodeNew, const value_type& value, ENABLE_IF_TRUETYPE(BoolConstantT)) // true_type means bUniqueKeys is true.
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename K, typename V, typename A, typename EK, typename Eq,
			  typename H1, typename H2, typename H, typename RP, bool bC, bool bM, bool bU>
	template <typename VFwd, typename Enabled, ENABLE_IF_TRUETYPE(Enabled)> // true_type means bUniqueKeys is true.
	eastl::pair<typename hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::iterator, bool>
	hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::DoInsertValueExtraForwarding(const key_type& k,
		hash_code_t c, node_type* pNodeNew, VFwd&& value)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename A, typename EK, typename Eq,
			  typename H1, typename H2, typename H, typename RP, bool bC, bool bM, bool bU>
	template <typename BoolConstantT>
	eastl::pair<typename hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::iterator, bool>
	hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::DoInsertValue(BoolConstantT, value_type&& value, ENABLE_IF_TRUETYPE(BoolConstantT)) // true_type means bUniqueKeys is true.
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename A, typename EK, typename Eq,
			  typename H1, typename H2, typename H, typename RP, bool bC, bool bM, bool bU>
	template <typename BoolConstantT>
	eastl::pair<typename hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::iterator, bool>
	hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::DoInsertValue(BoolConstantT, const value_type&& value, ENABLE_IF_TRUETYPE(BoolConstantT)) // true_type means bUniqueKeys is true.
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename A, typename EK, typename Eq,
			  typename H1, typename H2, typename H, typename RP, bool bC, bool bM, bool bU>
	template <typename BoolConstantT>
	typename hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::iterator
	hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::DoInsertValueExtra(BoolConstantT, const key_type& k, hash_code_t c, node_type* pNodeNew, value_type&& value, 
			DISABLE_IF_TRUETYPE(BoolConstantT)) // false_type means bUniqueKeys is false.
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename A, typename EK, typename Eq,
			  typename H1, typename H2, typename H, typename RP, bool bC, bool bM, bool bU>
	template<typename BoolConstantT>
	typename hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::iterator
	hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::DoInsertValue(BoolConstantT, value_type&& value, DISABLE_IF_TRUETYPE(BoolConstantT)) // false_type means bUniqueKeys is false.
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename A, typename EK, typename Eq,
			  typename H1, typename H2, typename H, typename RP, bool bC, bool bM, bool bU>
	template<typename BoolConstantT>
	typename hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::iterator
	hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::DoInsertValue(BoolConstantT, const value_type&& value, DISABLE_IF_TRUETYPE(BoolConstantT)) // false_type means bUniqueKeys is false.
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename A, typename EK, typename Eq,
			  typename H1, typename H2, typename H, typename RP, bool bC, bool bM, bool bU>
	typename hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::node_type*
	hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::DoAllocateNode(value_type&& value)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename K, typename V, typename A, typename EK, typename Eq,
				typename H1, typename H2, typename H, typename RP, bool bC, bool bM, bool bU>
	template<typename BoolConstantT>
	eastl::pair<typename hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::iterator, bool>
	hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::DoInsertValue(BoolConstantT, const value_type& value, ENABLE_IF_TRUETYPE(BoolConstantT)) // true_type means bUniqueKeys is true.
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename K, typename V, typename A, typename EK, typename Eq,
				typename H1, typename H2, typename H, typename RP, bool bC, bool bM, bool bU>
	template<typename BoolConstantT>
	eastl::pair<typename hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::iterator, bool>
	hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::DoInsertValue(BoolConstantT, value_type& value, ENABLE_IF_TRUETYPE(BoolConstantT)) // true_type means bUniqueKeys is true.
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename A, typename EK, typename Eq,
				typename H1, typename H2, typename H, typename RP, bool bC, bool bM, bool bU>
	template <typename BoolConstantT>
	typename hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::iterator
	hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::DoInsertValueExtra(BoolConstantT, const key_type& k, hash_code_t c, node_type* pNodeNew, const value_type& value,
			DISABLE_IF_TRUETYPE(BoolConstantT)) // false_type means bUniqueKeys is false.
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename A, typename EK, typename Eq,
				typename H1, typename H2, typename H, typename RP, bool bC, bool bM, bool bU>
	template<typename BoolConstantT>
	typename hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::iterator
	hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::DoInsertValue(BoolConstantT, const value_type& value, DISABLE_IF_TRUETYPE(BoolConstantT)) // false_type means bUniqueKeys is false.
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename A, typename EK, typename Eq,
				typename H1, typename H2, typename H, typename RP, bool bC, bool bM, bool bU>
	template<typename BoolConstantT>
	typename hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::iterator
	hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::DoInsertValue(BoolConstantT, value_type& value, DISABLE_IF_TRUETYPE(BoolConstantT)) // false_type means bUniqueKeys is false.
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename A, typename EK, typename Eq,
			  typename H1, typename H2, typename H, typename RP, bool bC, bool bM, bool bU>
	typename hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::node_type*
	hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::DoAllocateNode(const value_type& value)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename A, typename EK, typename Eq,
			  typename H1, typename H2, typename H, typename RP, bool bC, bool bM, bool bU>
	typename hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::node_type*
	hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::allocate_uninitialized_node()
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename A, typename EK, typename Eq,
			  typename H1, typename H2, typename H, typename RP, bool bC, bool bM, bool bU>
	void hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::free_uninitialized_node(node_type* pNode)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename A, typename EK, typename Eq,
			  typename H1, typename H2, typename H, typename RP, bool bC, bool bM, bool bU>
	eastl::pair<typename hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::iterator, bool>
	hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::DoInsertKey(true_type, const key_type& key, const hash_code_t c) // true_type means bUniqueKeys is true.
	{
    __builtin_trap() /* STUB: not implemented */;
}



	template <typename K, typename V, typename A, typename EK, typename Eq,
			  typename H1, typename H2, typename H, typename RP, bool bC, bool bM, bool bU>
	typename hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::iterator
	hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::DoInsertKey(false_type, const key_type& key, const hash_code_t c) // false_type means bUniqueKeys is false.
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename A, typename EK, typename Eq,
				typename H1, typename H2, typename H, typename RP, bool bC, bool bM, bool bU>
	template <class... Args>
	typename hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::insert_return_type
	hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::emplace(Args&&... args)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename K, typename V, typename A, typename EK, typename Eq,
				typename H1, typename H2, typename H, typename RP, bool bC, bool bM, bool bU>
	template <class... Args>
	typename hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::iterator
	hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::emplace_hint(const_iterator, Args&&... args)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename K, typename V, typename A, typename EK, typename Eq,
			  typename H1, typename H2, typename H, typename RP, bool bC, bool bM, bool bU>
	typename hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::insert_return_type
	hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::insert(value_type&& otherValue)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename A, typename EK, typename Eq,
			  typename H1, typename H2, typename H, typename RP, bool bC, bool bM, bool bU>
	template <class P>
	typename hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::insert_return_type
	hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::insert(hash_code_t c, node_type* pNodeNew, P&& otherValue)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename A, typename EK, typename Eq,
			  typename H1, typename H2, typename H, typename RP, bool bC, bool bM, bool bU>
	typename hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::iterator
	hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::insert(const_iterator, value_type&& value)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename A, typename EK, typename Eq,
			  typename H1, typename H2, typename H, typename RP, bool bC, bool bM, bool bU>
	typename hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::insert_return_type
	hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::insert(const value_type& value) 
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename A, typename EK, typename Eq,
			  typename H1, typename H2, typename H, typename RP, bool bC, bool bM, bool bU>
	typename hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::insert_return_type
	hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::insert(hash_code_t c, node_type* pNodeNew, const value_type& value) 
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename A, typename EK, typename Eq,
			  typename H1, typename H2, typename H, typename RP, bool bC, bool bM, bool bU>
	typename hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::iterator
	hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::insert(const_iterator, const value_type& value)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename A, typename EK, typename Eq,
			  typename H1, typename H2, typename H, typename RP, bool bC, bool bM, bool bU>
	void hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::insert(std::initializer_list<value_type> ilist)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename A, typename EK, typename Eq,
			  typename H1, typename H2, typename H, typename RP, bool bC, bool bM, bool bU>
	template <typename InputIterator>
	void
	hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::insert(InputIterator first, InputIterator last)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename K, typename V, typename A, typename EK, typename Eq,
			  typename H1, typename H2, typename H, typename RP, bool bC, bool bM, bool bU>
	template <class KX, class M>
	eastl::pair<typename hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::iterator, bool>
	hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::DoInsertOrAssign(KX&& k, M&& obj)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename K, typename V, typename A, typename EK, typename Eq,
			  typename H1, typename H2, typename H, typename RP, bool bC, bool bM, bool bU>
	template <class KX, class M>
	typename hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::iterator 
	hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::DoInsertOrAssign(const_iterator, KX&& k, M&& obj)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename A, typename EK, typename Eq,
			  typename H1, typename H2, typename H, typename RP, bool bC, bool bM, bool bU>
	typename hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::iterator
	hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::erase(const_iterator i)
	{
    __builtin_trap() /* STUB: not implemented */;
}



	template <typename K, typename V, typename A, typename EK, typename Eq,
			  typename H1, typename H2, typename H, typename RP, bool bC, bool bM, bool bU>
	inline typename hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::iterator
	hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::erase(const_iterator first, const_iterator last)
	{
    __builtin_trap() /* STUB: not implemented */;
}



	template <typename K, typename V, typename A, typename EK, typename Eq,
			  typename H1, typename H2, typename H, typename RP, bool bC, bool bM, bool bU>
	template <typename KX>
	typename hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::size_type 
	hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::DoErase(KX&& k)
	{
    __builtin_trap() /* STUB: not implemented */;
}



	template <typename K, typename V, typename A, typename EK, typename Eq,
			  typename H1, typename H2, typename H, typename RP, bool bC, bool bM, bool bU>
	inline void hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::clear()
	{
    __builtin_trap() /* STUB: not implemented */;
}



	template <typename K, typename V, typename A, typename EK, typename Eq,
			  typename H1, typename H2, typename H, typename RP, bool bC, bool bM, bool bU>
	inline void hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::clear(bool clearBuckets)
	{
    __builtin_trap() /* STUB: not implemented */;
}



	template <typename K, typename V, typename A, typename EK, typename Eq,
			  typename H1, typename H2, typename H, typename RP, bool bC, bool bM, bool bU>
	inline void hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::reset_lose_memory() EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename A, typename EK, typename Eq,
			  typename H1, typename H2, typename H, typename RP, bool bC, bool bM, bool bU>
	inline void hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::reserve(size_type nElementCount)
	{
    __builtin_trap() /* STUB: not implemented */;
}



	template <typename K, typename V, typename A, typename EK, typename Eq,
			  typename H1, typename H2, typename H, typename RP, bool bC, bool bM, bool bU>
	inline void hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::rehash(size_type nBucketCount)
	{
    __builtin_trap() /* STUB: not implemented */;
}



	template <typename K, typename V, typename A, typename EK, typename Eq,
			  typename H1, typename H2, typename H, typename RP, bool bC, bool bM, bool bU>
	void hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::DoRehash(size_type nNewBucketCount)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename A, typename EK, typename Eq,
			  typename H1, typename H2, typename H, typename RP, bool bC, bool bM, bool bU>
	inline bool hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::validate() const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename K, typename V, typename A, typename EK, typename Eq,
			  typename H1, typename H2, typename H, typename RP, bool bC, bool bM, bool bU>
	int hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>::validate_iterator(const_iterator i) const
	{
    __builtin_trap() /* STUB: not implemented */;
}



	///////////////////////////////////////////////////////////////////////
	// global operators
	///////////////////////////////////////////////////////////////////////

	// operator==, != have been moved to the specific container subclasses (e.g. hash_map).

	template <typename K, typename V, typename A, typename EK, typename Eq,
			  typename H1, typename H2, typename H, typename RP, bool bC, bool bM, bool bU>
	inline void swap(const hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>& a, 
					 const hashtable<K, V, A, EK, Eq, H1, H2, H, RP, bC, bM, bU>& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}


} // namespace eastl


EA_RESTORE_VC_WARNING();


#endif // Header include guard
