#include <cstdlib>
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) Electronic Arts Inc. All rights reserved.
/////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// This file implements a bitset much like the C++ std::bitset class. 
// The primary distinctions between this bitset and std::bitset are:
//    - bitset is more efficient than some other std::bitset implementations,
//      notably the bitset that comes with Microsoft and other 1st party platforms.
//    - bitset is savvy to an environment that doesn't have exception handling,
//      as is sometimes the case with console or embedded environments.
//    - bitset is savvy to environments in which 'unsigned long' is not the 
//      most efficient integral data type. std::bitset implementations use
//      unsigned long, even if it is an inefficient integer type.
//    - bitset removes as much function calls as practical, in order to allow
//      debug builds to run closer in speed and code footprint to release builds.
//    - bitset doesn't support string functionality. We can add this if 
//      it is deemed useful.
//
///////////////////////////////////////////////////////////////////////////////


#ifndef EASTL_BITSET_H
#define EASTL_BITSET_H


#include <EASTL/internal/config.h>
#include <EASTL/algorithm.h>
#include <EASTL/bit.h>

EA_DISABLE_ALL_VC_WARNINGS();

#include <stddef.h>
#include <string.h>

EA_RESTORE_ALL_VC_WARNINGS();

#if EASTL_EXCEPTIONS_ENABLED
	EA_DISABLE_ALL_VC_WARNINGS();

	#include <stdexcept> // std::out_of_range, std::length_error.

	EA_RESTORE_ALL_VC_WARNINGS();
#endif

EA_DISABLE_VC_WARNING(4127); // Conditional expression is constant

#if defined(EA_PRAGMA_ONCE_SUPPORTED)
	#pragma once // Some compilers (e.g. VC++) benefit significantly from using this. We've measured 3-4% build speed improvements in apps as a result.
#endif



namespace eastl
{
	// To consider: Enable this for backwards compatibility with any user code that might be using BitsetWordType:
	// #define BitsetWordType EASTL_BITSET_WORD_TYPE_DEFAULT


	/// BITSET_WORD_COUNT
	///
	/// Defines the number of words we use, based on the number of bits.
	/// nBitCount refers to the number of bits in a bitset.
	/// WordType refers to the type of integer word which stores bitet data. By default it is BitsetWordType.
	///
	/// Note: for nBitCount == 0, returns 1!
	#if !defined(__GNUC__) || (__GNUC__ >= 3) // GCC 2.x can't handle the simpler declaration below.
		#define BITSET_WORD_COUNT(nBitCount, WordType) (nBitCount == 0 ? 1 : ((nBitCount - 1) / (8 * sizeof(WordType)) + 1))
	#else
		#define BITSET_WORD_COUNT(nBitCount, WordType) ((nBitCount - 1) / (8 * sizeof(WordType)) + 1)
	#endif


	/// EASTL_DISABLE_BITSET_ARRAYBOUNDS_WARNING
	/// Before GCC 4.7 the '-Warray-bounds' buggy and was very likely to issue false positives for loops that are
	/// difficult to evaluate.
	/// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=45978
	///
	#if defined(__GNUC__) && (EA_COMPILER_VERSION > 4007) && defined(EA_PLATFORM_ANDROID) // Earlier than GCC 4.7 
		#define EASTL_DISABLE_BITSET_ARRAYBOUNDS_WARNING 1
	#else
		#define EASTL_DISABLE_BITSET_ARRAYBOUNDS_WARNING 0
	#endif

	template <size_t N, typename WordType = EASTL_BITSET_WORD_TYPE_DEFAULT>
	class bitset;

	namespace detail
	{
		template<typename T>
		struct is_word_type : bool_constant<!is_const_v<T> && !is_volatile_v<T> && !is_same_v<T, bool> && is_integral_v<T> && is_unsigned_v<T>> {};

		template<typename T>
		constexpr bool is_word_type_v = is_word_type<T>::value;

		// slices the min(N, UInt) lowest significant bits from value.
		template<size_t N, typename WordType, typename UInt>
		eastl::enable_if_t<is_word_type_v<UInt>> from_unsigned_integral(bitset<N, WordType>& bs, UInt value)
		{
    __builtin_trap() /* STUB: not implemented */;
}

		// This is here to work around the lack of `if constexpr` in C++14, so that calling
		// (WordType(1) << (CHAR_BIT * sizeof(UInt))) doesn't trigger warnings/errors when
		// sizeof(UInt) >= sizeof(WordType)
		template<typename UInt, typename WordType, size_t NumWords, bool bAssertOnOverflow, bool bDoPartialCopy = (sizeof(UInt) < sizeof(WordType))>
		struct to_unsigned_integral_helper
		{};

		template<typename UInt, typename WordType, size_t NumWords, bool bAssertOnOverflow>
		struct to_unsigned_integral_helper<UInt, WordType, NumWords, bAssertOnOverflow, true>
		{
			static size_t copyWords(const WordType* data, UInt* result)
			{
    __builtin_trap() /* STUB: not implemented */;
}
		};

		template<typename UInt, typename WordType, size_t NumWords, bool bAssertOnOverflow>
		struct to_unsigned_integral_helper<UInt, WordType, NumWords, bAssertOnOverflow, false>
		{
			static size_t copyWords(const WordType* data, UInt* result)
			{
    __builtin_trap() /* STUB: not implemented */;
}
		};



		template<typename UInt, bool bAssertOnOverflow, size_t N, typename WordType>
		eastl::enable_if_t<is_word_type_v<UInt>, UInt> to_unsigned_integral(const bitset<N, WordType>& bs)
		{
    __builtin_trap() /* STUB: not implemented */;
}
	} // namespace detail

	/// BitsetBase
	///
	/// This is a default implementation that works for any number of words.
	///
	template <size_t NW, typename WordType> // Templated on the number of words used to hold the bitset and the word type.
	struct BitsetBase
	{
		typedef WordType                 word_type;
		typedef BitsetBase<NW, WordType> this_type;
	  #if EASTL_BITSET_SIZE_T
		typedef size_t                   size_type;
	  #else
		typedef eastl_size_t             size_type;
	  #endif

		enum {
			kBitsPerWord      = (8 * sizeof(word_type)),
			kBitsPerWordMask  = (kBitsPerWord - 1),
			kBitsPerWordShift = ((kBitsPerWord == 8) ? 3 : ((kBitsPerWord == 16) ? 4 : ((kBitsPerWord == 32) ? 5 : (((kBitsPerWord == 64) ? 6 : 7)))))
		};

	public:
		// invariant: we keep any high bits in the last word that are unneeded set to 0
		// so that our to_ulong() conversion can simply copy the words into the target type.
		word_type mWord[NW];

	public:
		void operator&=(const this_type& x);
		void operator|=(const this_type& x);
		void operator^=(const this_type& x);

		void operator<<=(size_type n);
		void operator>>=(size_type n);

		void flip();
		void set();
		void set(size_type i, bool value);
		void reset();

		bool operator==(const this_type& x) const;

		bool      any() const;
		size_type count() const;

		word_type& DoGetWord(size_type i);
		word_type  DoGetWord(size_type i) const;

		size_type DoFindFirst() const;
		size_type DoFindNext(size_type last_find) const;

		size_type DoFindLast() const;                       // Returns NW * kBitsPerWord (the bit count) if no bits are set.
		size_type DoFindPrev(size_type last_find) const;    // Returns NW * kBitsPerWord (the bit count) if no bits are set.

	}; // class BitsetBase



	/// BitsetBase<1, WordType>
	/// 
	/// This is a specialization for a bitset that fits within one word.
	///
	template <typename WordType>
	struct BitsetBase<1, WordType>
	{
		typedef WordType                word_type;
		typedef BitsetBase<1, WordType> this_type;
	  #if EASTL_BITSET_SIZE_T
		typedef size_t                  size_type;
	  #else
		typedef eastl_size_t            size_type;
	  #endif

		enum {
			kBitsPerWord      = (8 * sizeof(word_type)),
			kBitsPerWordMask  = (kBitsPerWord - 1),
			kBitsPerWordShift = ((kBitsPerWord == 8) ? 3 : ((kBitsPerWord == 16) ? 4 : ((kBitsPerWord == 32) ? 5 : (((kBitsPerWord == 64) ? 6 : 7)))))
		};

	public:
		word_type mWord[1]; // Defined as an array of 1 so that bitset can treat this BitsetBase like others.

	public:
		void operator&=(const this_type& x);
		void operator|=(const this_type& x);
		void operator^=(const this_type& x);

		void operator<<=(size_type n);
		void operator>>=(size_type n);

		void flip();
		void set();
		void set(size_type i, bool value);
		void reset();

		bool operator==(const this_type& x) const;

		bool      any() const;
		size_type count() const;

		word_type& DoGetWord(size_type);
		word_type  DoGetWord(size_type) const;

		size_type DoFindFirst() const;
		size_type DoFindNext(size_type last_find) const;

		size_type DoFindLast() const;                       // Returns 1 * kBitsPerWord (the bit count) if no bits are set.
		size_type DoFindPrev(size_type last_find) const;    // Returns 1 * kBitsPerWord (the bit count) if no bits are set.

	}; // BitsetBase<1, WordType>



	/// BitsetBase<2, WordType>
	/// 
	/// This is a specialization for a bitset that fits within two words.
	/// The difference here is that we avoid branching (ifs and loops).
	///
	template <typename WordType>
	struct BitsetBase<2, WordType>
	{
		typedef WordType                 word_type;
		typedef BitsetBase<2, WordType>  this_type;
	  #if EASTL_BITSET_SIZE_T
		typedef size_t                   size_type;
	  #else
		typedef eastl_size_t             size_type;
	  #endif

		enum {
			kBitsPerWord      = (8 * sizeof(word_type)),
			kBitsPerWordMask  = (kBitsPerWord - 1),
			kBitsPerWordShift = ((kBitsPerWord == 8) ? 3 : ((kBitsPerWord == 16) ? 4 : ((kBitsPerWord == 32) ? 5 : (((kBitsPerWord == 64) ? 6 : 7)))))
		};

	public:
		word_type mWord[2];

	public:
		void operator&=(const this_type& x);
		void operator|=(const this_type& x);
		void operator^=(const this_type& x);

		void operator<<=(size_type n);
		void operator>>=(size_type n);

		void flip();
		void set();
		void set(size_type i, bool value);
		void reset();

		bool operator==(const this_type& x) const;

		bool      any() const;
		size_type count() const;

		word_type& DoGetWord(size_type);
		word_type  DoGetWord(size_type) const;

		size_type DoFindFirst() const;
		size_type DoFindNext(size_type last_find) const;

		size_type DoFindLast() const;                       // Returns 2 * kBitsPerWord (the bit count) if no bits are set.
		size_type DoFindPrev(size_type last_find) const;    // Returns 2 * kBitsPerWord (the bit count) if no bits are set.

	}; // BitsetBase<2, WordType>




	/// bitset
	///
	/// Implements a bitset much like the C++ std::bitset.
	///
	/// As of this writing we don't implement a specialization of bitset<0>,
	/// as it is deemed an academic exercise that nobody would actually
	/// use and it would increase code space and provide little practical
	/// benefit. Note that this doesn't mean bitset<0> isn't supported; 
	/// it means that our version of it isn't as efficient as it would be 
	/// if a specialization was made for it.
	///
	/// - N can be any unsigned (non-zero) value, though memory usage is 
	///   linear with respect to N, so large values of N use large amounts of memory.
	/// - WordType must be a non-cv qualified unsigned integral other than bool.
	///   By default the WordType is the largest native register type that the
	///   target platform supports.
	///
	template <size_t N, typename WordType>
	class bitset : private BitsetBase<BITSET_WORD_COUNT(N, WordType), WordType>
	{
	public:
		static_assert(detail::is_word_type_v<WordType>, "Word type must be a non-cv qualified, unsigned integral other than bool.");

		typedef BitsetBase<BITSET_WORD_COUNT(N, WordType), WordType>  base_type;
		typedef bitset<N, WordType>                                   this_type;
		typedef WordType                                              word_type;
		typedef typename base_type::size_type                         size_type;

		enum
		{
			kBitsPerWord      = (8 * sizeof(word_type)),
			kBitsPerWordMask  = (kBitsPerWord - 1),
			kBitsPerWordShift = ((kBitsPerWord == 8) ? 3 : ((kBitsPerWord == 16) ? 4 : ((kBitsPerWord == 32) ? 5 : (((kBitsPerWord == 64) ? 6 : 7))))),
			kSize             = N,                               // The number of bits the bitset holds
			kWordSize         = sizeof(word_type),               // The size of individual words the bitset uses to hold the bits.
			kWordCount        = BITSET_WORD_COUNT(N, WordType)   // The number of words the bitset uses to hold the bits. sizeof(bitset<N, WordType>) == kWordSize * kWordCount.
		};

		// internal implementation details. do not use.
		using base_type::mWord;
		using base_type::DoGetWord;
		using base_type::DoFindFirst;
		using base_type::DoFindNext;
		using base_type::DoFindLast;
		using base_type::DoFindPrev;

		using base_type::count;
		using base_type::any;

	public:
		/// reference
		///
		/// A reference is a reference to a specific bit in the bitset.
		/// The C++ standard specifies that this be a nested class, 
		/// though it is not clear if a non-nested reference implementation
		/// would be non-conforming.
		///
		class reference
		{
		protected:
			friend class bitset<N, WordType>;

			word_type* mpBitWord;
			size_type  mnBitIndex;
		
			reference(){
    __builtin_trap() /* STUB: not implemented */;
} // The C++ standard specifies that this is private.
	
		public:
			reference(const bitset& x, size_type i);

			reference& operator=(bool value);
			reference& operator=(const reference& x);

			bool operator~() const;
			operator bool() const // Defined inline because CodeWarrior fails to be able to compile it outside.
			   {
    __builtin_trap() /* STUB: not implemented */;
}

			reference& flip();
		};

	public:
		friend class reference;

		bitset();

#if EA_IS_ENABLED(EASTL_DEPRECATIONS_FOR_2024_SEPT)
		// note: this constructor will only copy the minimum of N or unsigned long long's size least significant bits.
		bitset(unsigned long long value);
#else
		bitset(uint32_t value);
#endif

		// We don't define copy constructor and operator= because 
		// the compiler-generated versions will suffice.

		this_type& operator&=(const this_type& x);
		this_type& operator|=(const this_type& x);
		this_type& operator^=(const this_type& x);

		this_type& operator<<=(size_type n);
		this_type& operator>>=(size_type n);

		this_type& set();
		this_type& set(size_type i, bool value = true);

		this_type& reset();
		this_type& reset(size_type i);
			
		this_type& flip();
		this_type& flip(size_type i);
		this_type  operator~() const;

		reference operator[](size_type i);
		bool      operator[](size_type i) const;

		const word_type* data() const;
		word_type*       data();

		// Deprecated: use the bitset(unsigned long long) constructor instead.
		// this was a workaround for when our constructor was defined as bitset(uint32_t) and could cause a narrowing conversion.
		EASTL_REMOVE_AT_2024_SEPT void          from_uint32(uint32_t value);
		EASTL_REMOVE_AT_2024_SEPT void          from_uint64(uint64_t value);

		/// to_xxx()
		/// 
		/// Not recommended: Use one of
		///   as_xxx() which is a compile time error if the target type cannot represent the entire bitset, or
		///   to_xxx_assert_convertible() which is the standard conformant version of this function, or
		///   to_xxx_no_assert_convertible() which has the same behaviour, explicit naming
		/// 
		/// Different from the standard:
		/// Does *NOT* assert that the bitset can be represented as the target integer type (has bits set outside the target type).
		/// However, if exceptions are enabled, it does throw an exception if the bitset cannot be represented as the target integer type.
		unsigned long to_ulong()  const;
		uint32_t      to_uint32() const;
		uint64_t      to_uint64() const;

		/// to_xxx_assert_convertible()
		///
		/// Equivalent to the standard library's to_ulong() / to_ullong().
		/// Asserts / throws an exception if the bitset cannot be represented as the target integer type.
		uint32_t			to_uint32_assert_convertible() const {
    __builtin_trap() /* STUB: not implemented */;
}
		uint64_t			to_uint64_assert_convertible() const {
    __builtin_trap() /* STUB: not implemented */;
}
		unsigned long		to_ulong_assert_convertible()  const {
    __builtin_trap() /* STUB: not implemented */;
}
		unsigned long long	to_ullong_assert_convertible() const {
    __builtin_trap() /* STUB: not implemented */;
}

		/// to_xxx_no_assert_convertible()
		///
		/// Prefer to_xxx_assert_convertible() instead of these functions.
		/// 
		/// Different from the standard:
		/// Does *NOT* assert that the bitset can be represented as the target integer type (has bits set outside the target type).
		/// However, if exceptions are enabled, it does throw an exception if the bitset cannot be represented as the target integer type.
		uint32_t			to_uint32_no_assert_convertible() const {
    __builtin_trap() /* STUB: not implemented */;
}
		uint64_t			to_uint64_no_assert_convertible() const {
    __builtin_trap() /* STUB: not implemented */;
}
		unsigned long		to_ulong_no_assert_convertible()  const {
    __builtin_trap() /* STUB: not implemented */;
}
		unsigned long long	to_ullong_no_assert_convertible() const {
    __builtin_trap() /* STUB: not implemented */;
}

		/// as_uint<UInt>() / as_xxx()
		/// 
		/// Extension to the standard: Cast to a unsigned integral that can represent the entire bitset.
		/// If the target type cannot represent the entire bitset, then issue a compile error (overload does not exist).
		/// Never throws / asserts.
		template<typename UInt>
		eastl::enable_if_t<detail::is_word_type_v<UInt> && N <= (CHAR_BIT * sizeof(UInt)), UInt>	as_uint() const noexcept {
    __builtin_trap() /* STUB: not implemented */;
}

		template<size_t NumBits = N>
		eastl::enable_if_t<NumBits <= (CHAR_BIT * sizeof(uint32_t)), uint32_t>						as_uint32() const noexcept {
    __builtin_trap() /* STUB: not implemented */;
}
		template<size_t NumBits = N>
		eastl::enable_if_t<NumBits <= (CHAR_BIT * sizeof(uint64_t)), uint64_t>						as_uint64() const noexcept {
    __builtin_trap() /* STUB: not implemented */;
}
		template<size_t NumBits = N>
		eastl::enable_if_t<NumBits <= (CHAR_BIT * sizeof(unsigned long)), unsigned long>			as_ulong() const noexcept {
    __builtin_trap() /* STUB: not implemented */;
}
		template<size_t NumBits = N>
		eastl::enable_if_t<NumBits <= (CHAR_BIT * sizeof(unsigned long long)), unsigned long long>	as_ullong() const noexcept {
    __builtin_trap() /* STUB: not implemented */;
}

	  //size_type count() const;            // We inherit this from the base class.
		size_type size() const;

		bool operator==(const this_type& x) const;
#if !defined(EA_COMPILER_HAS_THREE_WAY_COMPARISON)
		bool operator!=(const this_type& x) const;
#endif

		bool test(size_type i) const;
	  //bool any() const;                   // We inherit this from the base class.
		bool all() const;
		bool none() const;

		this_type operator<<(size_type n) const;
		this_type operator>>(size_type n) const;

		// Finds the index of the first "on" bit, returns kSize if none are set.
		size_type find_first() const;

		// Finds the index of the next "on" bit after last_find, returns kSize if none are set.
		size_type find_next(size_type last_find) const;

		// Finds the index of the last "on" bit, returns kSize if none are set.
		size_type find_last() const;

		// Finds the index of the last "on" bit before last_find, returns kSize if none are set.
		size_type find_prev(size_type last_find) const;

	}; // bitset







	/// BitsetCountBits
	///
	/// This is a fast trick way to count bits without branches nor memory accesses.
	///
	/// todo: Use bit.h's popcount instead?
	template<typename UInt64>
	eastl::enable_if_t<detail::is_word_type_v<UInt64> && sizeof(UInt64) == 8, uint32_t> BitsetCountBits(UInt64 x)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template<typename UInt32>
	eastl::enable_if_t<detail::is_word_type_v<UInt32> && sizeof(UInt32) == 4, uint32_t> BitsetCountBits(UInt32 x)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template<typename SmallUInt>
	eastl::enable_if_t< detail::is_word_type_v<SmallUInt> && sizeof(SmallUInt) < 4, uint32_t> BitsetCountBits(SmallUInt x)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	// const static char kBitsPerUint16[16] = { 0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4 };
	#define EASTL_BITSET_COUNT_STRING "\0\1\1\2\1\2\2\3\1\2\2\3\2\3\3\4"


	template<typename UInt8>
	eastl::enable_if_t<detail::is_word_type_v<UInt8> && sizeof(UInt8) == 1, uint32_t> GetFirstBit(UInt8 x)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	// To do: Update this to use VC++ _BitScanForward, _BitScanForward64;
	// GCC __builtin_ctz, __builtin_ctzl.
	// VC++ __lzcnt16, __lzcnt, __lzcnt64 requires recent CPUs (2013+) and probably can't be used.
	// http://en.wikipedia.org/wiki/Haswell_%28microarchitecture%29#New_features
	template<typename UInt16>
	eastl::enable_if_t<detail::is_word_type_v<UInt16> && sizeof(UInt16) == 2, uint32_t> GetFirstBit(UInt16 x)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template<typename UInt32>
	eastl::enable_if_t<detail::is_word_type_v<UInt32> && sizeof(UInt32) == 4, uint32_t> GetFirstBit(UInt32 x)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template<typename UInt64>
	eastl::enable_if_t<detail::is_word_type_v<UInt64> && sizeof(UInt64) == 8, uint32_t> GetFirstBit(UInt64 x)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	#if EASTL_INT128_SUPPORTED
		inline uint32_t GetFirstBit(eastl_uint128_t x)
		{
			if(x)
			{
				uint32_t n = 1;

				if((x & UINT64_C(0xFFFFFFFFFFFFFFFF)) == 0) { n += 64; x >>= 64; }
				if((x & 0xFFFFFFFF) == 0)                   { n += 32; x >>= 32; }
				if((x & 0x0000FFFF) == 0)                   { n += 16; x >>= 16; }
				if((x & 0x000000FF) == 0)                   { n +=  8; x >>=  8; }
				if((x & 0x0000000F) == 0)                   { n +=  4; x >>=  4; }
				if((x & 0x00000003) == 0)                   { n +=  2; x >>=  2; }

				return (n - ((uint32_t)x & 1));
			}

			return 128;
		}
	#endif

	template<typename UInt8>
	eastl::enable_if_t<detail::is_word_type_v<UInt8> && sizeof(UInt8) == 1, uint32_t> GetLastBit(UInt8 x)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template<typename UInt16>
	eastl::enable_if_t<detail::is_word_type_v<UInt16> && sizeof(UInt16) == 2, uint32_t> GetLastBit(UInt16 x)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template<typename UInt32>
	eastl::enable_if_t<detail::is_word_type_v<UInt32> && sizeof(UInt32) == 4, uint32_t> GetLastBit(UInt32 x)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template<typename UInt64>
	eastl::enable_if_t<detail::is_word_type_v<UInt64> && sizeof(UInt64) == 8, uint32_t> GetLastBit(UInt64 x)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	#if EASTL_INT128_SUPPORTED
		inline uint32_t GetLastBit(eastl_uint128_t x)
		{
			if(x)
			{
				uint32_t n = 0;
				
				eastl_uint128_t mask(UINT64_C(0xFFFFFFFFFFFFFFFF)); // There doesn't seem to exist compiler support for INT128_C() by any compiler. EAStdC's int128_t supports it though.
				mask <<= 64;

				if(x & mask)                         { n += 64; x >>= 64; }
				if(x & UINT64_C(0xFFFFFFFF00000000)) { n += 32; x >>= 32; }
				if(x & UINT64_C(0x00000000FFFF0000)) { n += 16; x >>= 16; }
				if(x & UINT64_C(0x00000000FFFFFF00)) { n +=  8; x >>=  8; }
				if(x & UINT64_C(0x00000000FFFFFFF0)) { n +=  4; x >>=  4; }
				if(x & UINT64_C(0x00000000FFFFFFFC)) { n +=  2; x >>=  2; }
				if(x & UINT64_C(0x00000000FFFFFFFE)) { n +=  1;           }

				return n;
			}

			return 128;
		}
	#endif




	///////////////////////////////////////////////////////////////////////////
	// BitsetBase
	//
	// We tried two forms of array access here:
	//     for(word_type *pWord(mWord), *pWordEnd(mWord + NW); pWord < pWordEnd; ++pWord)
	//         *pWord = ...
	// and
	//     for(size_t i = 0; i < NW; i++)
	//         mWord[i] = ...
	//
	// For our tests (~NW < 16), the latter (using []) access resulted in faster code. 
	///////////////////////////////////////////////////////////////////////////


	template <size_t NW, typename WordType>
	inline void BitsetBase<NW, WordType>::operator&=(const this_type& x)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <size_t NW, typename WordType>
	inline void BitsetBase<NW, WordType>::operator|=(const this_type& x)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <size_t NW, typename WordType>
	inline void BitsetBase<NW, WordType>::operator^=(const this_type& x)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <size_t NW, typename WordType>
	inline void BitsetBase<NW, WordType>::operator<<=(size_type n)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <size_t NW, typename WordType>
	inline void BitsetBase<NW, WordType>::operator>>=(size_type n)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <size_t NW, typename WordType>
	inline void BitsetBase<NW, WordType>::flip()
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <size_t NW, typename WordType>
	inline void BitsetBase<NW, WordType>::set()
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <size_t NW, typename WordType>
	inline void BitsetBase<NW, WordType>::set(size_type i, bool value)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <size_t NW, typename WordType>
	inline void BitsetBase<NW, WordType>::reset()
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <size_t NW, typename WordType>
	inline bool BitsetBase<NW, WordType>::operator==(const this_type& x) const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <size_t NW, typename WordType>
	inline bool BitsetBase<NW, WordType>::any() const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <size_t NW, typename WordType>
	inline typename BitsetBase<NW, WordType>::size_type
	BitsetBase<NW, WordType>::count() const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <size_t NW, typename WordType>
	inline typename BitsetBase<NW, WordType>::word_type&
	BitsetBase<NW, WordType>::DoGetWord(size_type i)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <size_t NW, typename WordType>
	inline typename BitsetBase<NW, WordType>::word_type
	BitsetBase<NW, WordType>::DoGetWord(size_type i) const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <size_t NW, typename WordType>
	inline typename BitsetBase<NW, WordType>::size_type 
	BitsetBase<NW, WordType>::DoFindFirst() const
	{
    __builtin_trap() /* STUB: not implemented */;
}


#if EASTL_DISABLE_BITSET_ARRAYBOUNDS_WARNING
EA_DISABLE_GCC_WARNING(-Warray-bounds)
#endif

	template <size_t NW, typename WordType>
	inline typename BitsetBase<NW, WordType>::size_type 
	BitsetBase<NW, WordType>::DoFindNext(size_type last_find) const
	{
    __builtin_trap() /* STUB: not implemented */;
}

#if EASTL_DISABLE_BITSET_ARRAYBOUNDS_WARNING
EA_RESTORE_GCC_WARNING()
#endif



	template <size_t NW, typename WordType>
	inline typename BitsetBase<NW, WordType>::size_type 
	BitsetBase<NW, WordType>::DoFindLast() const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <size_t NW, typename WordType>
	inline typename BitsetBase<NW, WordType>::size_type 
	BitsetBase<NW, WordType>::DoFindPrev(size_type last_find) const
	{
    __builtin_trap() /* STUB: not implemented */;
}



	///////////////////////////////////////////////////////////////////////////
	// BitsetBase<1, WordType>
	///////////////////////////////////////////////////////////////////////////


	template <typename WordType>
	inline void BitsetBase<1, WordType>::operator&=(const this_type& x)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename WordType>
	inline void BitsetBase<1, WordType>::operator|=(const this_type& x)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename WordType>
	inline void BitsetBase<1, WordType>::operator^=(const this_type& x)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename WordType>
	inline void BitsetBase<1, WordType>::operator<<=(size_type n)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename WordType>
	inline void BitsetBase<1, WordType>::operator>>=(size_type n)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename WordType>
	inline void BitsetBase<1, WordType>::flip()
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename WordType>
	inline void BitsetBase<1, WordType>::set()
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename WordType>
	inline void BitsetBase<1, WordType>::set(size_type i, bool value)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename WordType>
	inline void BitsetBase<1, WordType>::reset()
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename WordType>
	inline bool BitsetBase<1, WordType>::operator==(const this_type& x) const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename WordType>
	inline bool BitsetBase<1, WordType>::any() const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename WordType>
	inline typename BitsetBase<1, WordType>::size_type
	BitsetBase<1, WordType>::count() const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename WordType>
	inline typename BitsetBase<1, WordType>::word_type&
	BitsetBase<1, WordType>::DoGetWord(size_type)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename WordType>
	inline typename BitsetBase<1, WordType>::word_type
	BitsetBase<1, WordType>::DoGetWord(size_type) const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename WordType>
	inline typename BitsetBase<1, WordType>::size_type
	BitsetBase<1, WordType>::DoFindFirst() const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename WordType>
	inline typename BitsetBase<1, WordType>::size_type 
	BitsetBase<1, WordType>::DoFindNext(size_type last_find) const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename WordType>
	inline typename BitsetBase<1, WordType>::size_type 
	BitsetBase<1, WordType>::DoFindLast() const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename WordType>
	inline typename BitsetBase<1, WordType>::size_type 
	BitsetBase<1, WordType>::DoFindPrev(size_type last_find) const
	{
    __builtin_trap() /* STUB: not implemented */;
}




	///////////////////////////////////////////////////////////////////////////
	// BitsetBase<2, WordType>
	///////////////////////////////////////////////////////////////////////////


	template <typename WordType>
	inline void BitsetBase<2, WordType>::operator&=(const this_type& x)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename WordType>
	inline void BitsetBase<2, WordType>::operator|=(const this_type& x)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename WordType>
	inline void BitsetBase<2, WordType>::operator^=(const this_type& x)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename WordType>
	inline void BitsetBase<2, WordType>::operator<<=(size_type n)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename WordType>
	inline void BitsetBase<2, WordType>::operator>>=(size_type n)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename WordType>
	inline void BitsetBase<2, WordType>::flip()
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename WordType>
	inline void BitsetBase<2, WordType>::set()
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename WordType>
	inline void BitsetBase<2, WordType>::set(size_type i, bool value)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename WordType>
	inline void BitsetBase<2, WordType>::reset()
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename WordType>
	inline bool BitsetBase<2, WordType>::operator==(const this_type& x) const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename WordType>
	inline bool BitsetBase<2, WordType>::any() const
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename WordType>
	inline typename BitsetBase<2, WordType>::size_type
	BitsetBase<2, WordType>::count() const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename WordType>
	inline typename BitsetBase<2, WordType>::word_type&
	BitsetBase<2, WordType>::DoGetWord(size_type i)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename WordType>
	inline typename BitsetBase<2, WordType>::word_type
	BitsetBase<2, WordType>::DoGetWord(size_type i) const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename WordType>
	inline typename BitsetBase<2, WordType>::size_type 
	BitsetBase<2, WordType>::DoFindFirst() const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename WordType>
	inline typename BitsetBase<2, WordType>::size_type 
	BitsetBase<2, WordType>::DoFindNext(size_type last_find) const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename WordType>
	inline typename BitsetBase<2, WordType>::size_type 
	BitsetBase<2, WordType>::DoFindLast() const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename WordType>
	inline typename BitsetBase<2, WordType>::size_type 
	BitsetBase<2, WordType>::DoFindPrev(size_type last_find) const
	{
    __builtin_trap() /* STUB: not implemented */;
}



	///////////////////////////////////////////////////////////////////////////
	// bitset::reference
	///////////////////////////////////////////////////////////////////////////

	template <size_t N, typename WordType>
	inline bitset<N, WordType>::reference::reference(const bitset& x, size_type i)
		: mpBitWord(&const_cast<bitset&>(x).DoGetWord(i)),
		  mnBitIndex(i & kBitsPerWordMask)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <size_t N, typename WordType>
	inline typename bitset<N, WordType>::reference&
	bitset<N, WordType>::reference::operator=(bool value)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <size_t N, typename WordType>
	inline typename bitset<N, WordType>::reference&
	bitset<N, WordType>::reference::operator=(const reference& x)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <size_t N, typename WordType>
	inline bool bitset<N, WordType>::reference::operator~() const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	//Defined inline in the class because Metrowerks fails to be able to compile it here.
	//template <size_t N, typename WordType>
	//inline bitset<N, WordType>::reference::operator bool() const
	//{
	//    return (*mpBitWord & (static_cast<word_type>(1) << (mnBitIndex & kBitsPerWordMask))) != 0;
	//}


	template <size_t N, typename WordType>
	inline typename bitset<N, WordType>::reference&
	bitset<N, WordType>::reference::flip()
	{
    __builtin_trap() /* STUB: not implemented */;
}




	///////////////////////////////////////////////////////////////////////////
	// bitset
	///////////////////////////////////////////////////////////////////////////

	template <size_t N, typename WordType>
	inline bitset<N, WordType>::bitset()
	{
    __builtin_trap() /* STUB: not implemented */;
}

	EA_DISABLE_VC_WARNING(6313)
#if EA_IS_ENABLED(EASTL_DEPRECATIONS_FOR_2024_SEPT)
	template <size_t N, typename WordType>
	inline bitset<N, WordType>::bitset(unsigned long long value)
	{
    __builtin_trap() /* STUB: not implemented */;
}
#else
	template <size_t N, typename WordType>
	inline bitset<N, WordType>::bitset(uint32_t value)
	{
		detail::from_unsigned_integral(*this, value);
	}
#endif
	EA_RESTORE_VC_WARNING()


	template <size_t N, typename WordType>
	inline typename bitset<N, WordType>::this_type&
	bitset<N, WordType>::operator&=(const this_type& x)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <size_t N, typename WordType>
	inline typename bitset<N, WordType>::this_type&
	bitset<N, WordType>::operator|=(const this_type& x)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <size_t N, typename WordType>
	inline typename bitset<N, WordType>::this_type&
	bitset<N, WordType>::operator^=(const this_type& x)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <size_t N, typename WordType>
	inline typename bitset<N, WordType>::this_type&
	bitset<N, WordType>::operator<<=(size_type n)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <size_t N, typename WordType>
	inline typename bitset<N, WordType>::this_type&
	bitset<N, WordType>::operator>>=(size_type n)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <size_t N, typename WordType>
	inline typename bitset<N, WordType>::this_type&
	bitset<N, WordType>::set()
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <size_t N, typename WordType>
	inline typename bitset<N, WordType>::this_type&
	bitset<N, WordType>::set(size_type i, bool value)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <size_t N, typename WordType>
	inline typename bitset<N, WordType>::this_type&
	bitset<N, WordType>::reset()
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <size_t N, typename WordType>
	inline typename bitset<N, WordType>::this_type&
	bitset<N, WordType>::reset(size_type i)
	{
    __builtin_trap() /* STUB: not implemented */;
}

		
	template <size_t N, typename WordType>
	inline typename bitset<N, WordType>::this_type&
	bitset<N, WordType>::flip()
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <size_t N, typename WordType>
	inline typename bitset<N, WordType>::this_type&
	bitset<N, WordType>::flip(size_type i)
	{
    __builtin_trap() /* STUB: not implemented */;
}
		

	template <size_t N, typename WordType>
	inline typename bitset<N, WordType>::this_type
	bitset<N, WordType>::operator~() const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <size_t N, typename WordType>
	inline typename bitset<N, WordType>::reference
	bitset<N, WordType>::operator[](size_type i)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <size_t N, typename WordType>
	inline bool bitset<N, WordType>::operator[](size_type i) const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <size_t N, typename WordType>
	inline const typename bitset<N, WordType>::word_type* bitset<N, WordType>::data() const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <size_t N, typename WordType>
	inline typename bitset<N, WordType>::word_type* bitset<N, WordType>::data()
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <size_t N, typename WordType>
	inline void bitset<N, WordType>::from_uint32(uint32_t value)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <size_t N, typename WordType>
	inline void bitset<N, WordType>::from_uint64(uint64_t value)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <size_t N, typename WordType>
	inline unsigned long bitset<N, WordType>::to_ulong() const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <size_t N, typename WordType>
	inline uint32_t bitset<N, WordType>::to_uint32() const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <size_t N, typename WordType>
	inline uint64_t bitset<N, WordType>::to_uint64() const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	// template <size_t N, typename WordType>
	// inline typename bitset<N, WordType>::size_type
	// bitset<N, WordType>::count() const
	// {
	//     return base_type::count();
	// }


	template <size_t N, typename WordType>
	inline typename bitset<N, WordType>::size_type
	bitset<N, WordType>::size() const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <size_t N, typename WordType>
	inline bool bitset<N, WordType>::operator==(const this_type& x) const
	{
    __builtin_trap() /* STUB: not implemented */;
}

#if !defined(EA_COMPILER_HAS_THREE_WAY_COMPARISON)
	template <size_t N, typename WordType>
	inline bool bitset<N, WordType>::operator!=(const this_type& x) const
	{
    __builtin_trap() /* STUB: not implemented */;
}
#endif

	template <size_t N, typename WordType>
	inline bool bitset<N, WordType>::test(size_type i) const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	// template <size_t N, typename WordType>
	// inline bool bitset<N, WordType>::any() const
	// {
	//     return base_type::any();
	// }


	template <size_t N, typename WordType>
	inline bool bitset<N, WordType>::all() const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <size_t N, typename WordType>
	inline bool bitset<N, WordType>::none() const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <size_t N, typename WordType>
	inline typename bitset<N, WordType>::this_type
	bitset<N, WordType>::operator<<(size_type n) const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <size_t N, typename WordType>
	inline typename bitset<N, WordType>::this_type
	bitset<N, WordType>::operator>>(size_type n) const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <size_t N, typename WordType>
	inline typename bitset<N, WordType>::size_type
	bitset<N, WordType>::find_first() const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <size_t N, typename WordType>
	inline typename bitset<N, WordType>::size_type
	bitset<N, WordType>::find_next(size_type last_find) const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <size_t N, typename WordType>
	inline typename bitset<N, WordType>::size_type
	bitset<N, WordType>::find_last() const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <size_t N, typename WordType>
	inline typename bitset<N, WordType>::size_type
	bitset<N, WordType>::find_prev(size_type last_find) const
	{
    __builtin_trap() /* STUB: not implemented */;
}



	///////////////////////////////////////////////////////////////////////////
	// global operators
	///////////////////////////////////////////////////////////////////////////

	template <size_t N, typename WordType>
	inline bitset<N, WordType> operator&(const bitset<N, WordType>& a, const bitset<N, WordType>& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <size_t N, typename WordType>
	inline bitset<N, WordType> operator|(const bitset<N, WordType>& a, const bitset<N, WordType>& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <size_t N, typename WordType>
	inline bitset<N, WordType> operator^(const bitset<N, WordType>& a, const bitset<N, WordType>& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}


} // namespace eastl


EA_RESTORE_VC_WARNING();

#endif // Header include guard
