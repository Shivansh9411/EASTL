///////////////////////////////////////////////////////////////////////////////
// Copyright (c) Electronic Arts Inc. All rights reserved.
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Implements a basic_string class, much like the C++ std::basic_string.
// The primary distinctions between basic_string and std::basic_string are:
//    - basic_string has a few extension functions that allow for increased performance.
//    - basic_string has a few extension functions that make use easier,
//      such as a member sprintf function and member tolower/toupper functions.
//    - basic_string supports debug memory naming natively.
//    - basic_string is easier to read, debug, and visualize.
//    - basic_string internally manually expands basic functions such as begin(),
//      size(), etc. in order to improve debug performance and optimizer success.
//    - basic_string is savvy to an environment that doesn't have exception handling,
//      as is sometimes the case with console or embedded environments.
//    - basic_string has less deeply nested function calls and allows the user to
//      enable forced inlining in debug builds in order to reduce bloat.
//    - basic_string doesn't use char traits. As a result, EASTL assumes that
//      strings will hold characters and not exotic things like widgets. At the
//      very least, basic_string assumes that the value_type is a POD.
//    - basic_string::size_type is defined as eastl_size_t instead of size_t in
//      order to save memory and run faster on 64 bit systems.
//    - basic_string data is guaranteed to be contiguous.
//    - basic_string data is guaranteed to be 0-terminated, and the c_str() function
//      is guaranteed to return the same pointer as the data() which is guaranteed
//      to be the same value as &string[0].
//    - basic_string has a set_capacity() function which frees excess capacity.
//      The only way to do this with std::basic_string is via the cryptic non-obvious
//      trick of using: basic_string<char>(x).swap(x);
//    - basic_string has a force_size() function, which unilaterally moves the string
//      end position (mpEnd) to the given location. Useful for when the user writes
//      into the string via some external means such as C strcpy or sprintf.
//    - basic_string substr() deviates from the standard and returns a string with
//		a copy of this->get_allocator()
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Copy on Write (cow)
//
// This string implementation does not do copy on write (cow). This is by design,
// as cow penalizes 95% of string uses for the benefit of only 5% of the uses
// (these percentages are qualitative, not quantitative). The primary benefit of
// cow is that it allows for the sharing of string data between two string objects.
// Thus if you say this:
//    string a("hello");
//    string b(a);
// the "hello" will be shared between a and b. If you then say this:
//    a = "world";
// then a will release its reference to "hello" and leave b with the only reference
// to it. Normally this functionality is accomplished via reference counting and
// with atomic operations or mutexes.
//
// The C++ standard does not say anything about basic_string and cow. However,
// for a basic_string implementation to be standards-conforming, a number of
// issues arise which dictate some things about how one would have to implement
// a cow string. The discussion of these issues will not be rehashed here, as you
// can read the references below for better detail than can be provided in the
// space we have here. However, we can say that the C++ standard is sensible and
// that anything we try to do here to allow for an efficient cow implementation
// would result in a generally unacceptable string interface.
//
// The disadvantages of cow strings are:
//    - A reference count needs to exist with the string, which increases string memory usage.
//    - With thread safety, atomic operations and mutex locks are expensive, especially
//      on weaker memory systems such as console gaming platforms.
//    - All non-const string accessor functions need to do a sharing check then the
//      first such check needs to detach the string. Similarly, all string assignments
//      need to do a sharing check as well. If you access the string before doing an
//      assignment, the assignment doesn't result in a shared string, because the string
//      has already been detached.
//    - String sharing doesn't happen the large majority of the time. In some cases,
//      the total sum of the reference count memory can exceed any memory savings
//      gained by the strings that share representations.
//
// The addition of a string_cow class is under consideration for this library.
// There are conceivably some systems which have string usage patterns which would
// benefit from cow sharing. Such functionality is best saved for a separate string
// implementation so that the other string uses aren't penalized.
//
// References:
//    This is a good starting HTML reference on the topic:
//       http://www.gotw.ca/publications/optimizations.htm
//    Here is a Usenet discussion on the topic:
//       http://groups-beta.google.com/group/comp.lang.c++.moderated/browse_thread/thread/3dc6af5198d0bf7/886c8642cb06e03d
//
///////////////////////////////////////////////////////////////////////////////


#ifndef EASTL_STRING_H
#define EASTL_STRING_H

#include <EASTL/internal/config.h>
#include <EASTL/allocator.h>
#include <EASTL/iterator.h>
#include <EASTL/algorithm.h>
#include <EASTL/initializer_list.h>
#include <EASTL/bonus/compressed_pair.h>

EA_DISABLE_ALL_VC_WARNINGS()
#include <stddef.h>             // size_t, ptrdiff_t, etc.
#include <stdarg.h>             // vararg functionality.

#include <stdlib.h>             // malloc, free.
#include <stdio.h>              // snprintf, etc.
#include <ctype.h>              // toupper, etc.

EA_DISABLE_GCC_WARNING(-Wtype-limits)
#include <wchar.h>
EA_RESTORE_GCC_WARNING()

#include <string.h> // strlen, etc.

#if EASTL_EXCEPTIONS_ENABLED
	#include <stdexcept> // std::out_of_range, std::length_error, std::logic_error.
#endif
EA_RESTORE_ALL_VC_WARNINGS()


// 4530 - C++ exception handler used, but unwind semantics are not enabled. Specify /EHsc
// 4480 - nonstandard extension used: specifying underlying type for enum
// 4571 - catch(...) semantics changed since Visual C++ 7.1; structured exceptions (SEH) are no longer caught.
// 4267 - 'argument' : conversion from 'size_t' to 'const uint32_t', possible loss of data. This is a bogus warning resulting from a bug in VC++.
// 4702 - unreachable code
EA_DISABLE_VC_WARNING(4530 4480 4571 4267 4702);


#if defined(EA_PRAGMA_ONCE_SUPPORTED)
	#pragma once // Some compilers (e.g. VC++) benefit significantly from using this. We've measured 3-4% build speed improvements in apps as a result.
#endif


#include <EASTL/internal/char_traits.h>
#include <EASTL/string_view.h>

///////////////////////////////////////////////////////////////////////////////
// EASTL_STRING_EXPLICIT
//
// See EASTL_STRING_OPT_EXPLICIT_CTORS for documentation.
//
#if EASTL_STRING_OPT_EXPLICIT_CTORS
	#define EASTL_STRING_EXPLICIT explicit
#else
	#define EASTL_STRING_EXPLICIT
#endif
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
// Vsnprintf
//
// The user is expected to supply these functions one way or another. Note that
// these functions are expected to accept parameters as per the C99 standard.
// These functions can deal with C99 standard return values or Microsoft non-standard
// return values but act more efficiently if implemented via the C99 style.
//
// In the case of EASTL_EASTDC_VSNPRINTF == 1, the user is expected to either
// link EAStdC or provide the functions below that act the same. In the case of
// EASTL_EASTDC_VSNPRINTF == 0, the user is expected to provide the function
// implementations, and may simply use C vsnprintf if desired, though it's not
// completely portable between compilers.
//
#if EASTL_EASTDC_VSNPRINTF
	namespace EA
	{
		namespace StdC
		{
			// Provided by the EAStdC package or by the user.
			EASTL_EASTDC_API int Vsnprintf(char*  EA_RESTRICT pDestination, size_t n, const char*  EA_RESTRICT pFormat, va_list arguments);
			EASTL_EASTDC_API int Vsnprintf(char16_t* EA_RESTRICT pDestination, size_t n, const char16_t* EA_RESTRICT pFormat, va_list arguments);
			EASTL_EASTDC_API int Vsnprintf(char32_t* EA_RESTRICT pDestination, size_t n, const char32_t* EA_RESTRICT pFormat, va_list arguments);
			#if EA_CHAR8_UNIQUE
				EASTL_EASTDC_API int Vsnprintf(char8_t*  EA_RESTRICT pDestination, size_t n, const char8_t*  EA_RESTRICT pFormat, va_list arguments);
			#endif
			#if defined(EA_WCHAR_UNIQUE) && EA_WCHAR_UNIQUE
				EASTL_EASTDC_API int Vsnprintf(wchar_t* EA_RESTRICT pDestination, size_t n, const wchar_t* EA_RESTRICT pFormat, va_list arguments);
			#endif
		}
	}

	namespace eastl
	{
		inline int Vsnprintf(char* EA_RESTRICT pDestination, size_t n, const char* EA_RESTRICT pFormat, va_list arguments)
			{
    __builtin_trap() /* STUB: not implemented */;
}

		inline int Vsnprintf(char16_t* EA_RESTRICT pDestination, size_t n, const char16_t* EA_RESTRICT pFormat, va_list arguments)
			{
    __builtin_trap() /* STUB: not implemented */;
}

		inline int Vsnprintf(char32_t* EA_RESTRICT pDestination, size_t n, const char32_t* EA_RESTRICT pFormat, va_list arguments)
			{
    __builtin_trap() /* STUB: not implemented */;
}

		#if EA_CHAR8_UNIQUE
			inline int Vsnprintf(char8_t* EA_RESTRICT pDestination, size_t n, const char8_t* EA_RESTRICT pFormat, va_list arguments)
				{ return EA::StdC::Vsnprintf((char*)pDestination, n, (const char*)pFormat, arguments); }
		#endif

		#if defined(EA_WCHAR_UNIQUE) && EA_WCHAR_UNIQUE
			inline int Vsnprintf(wchar_t* EA_RESTRICT pDestination, size_t n, const wchar_t* EA_RESTRICT pFormat, va_list arguments)
			{
    __builtin_trap() /* STUB: not implemented */;
}
		#endif
	}
#else
	// User-provided functions.
	extern int Vsnprintf8 (char*  pDestination, size_t n, const char*  pFormat, va_list arguments);
	extern int Vsnprintf16(char16_t* pDestination, size_t n, const char16_t* pFormat, va_list arguments);
	extern int Vsnprintf32(char32_t* pDestination, size_t n, const char32_t* pFormat, va_list arguments);
	#if EA_CHAR8_UNIQUE
		extern int Vsnprintf8 (char8_t*  pDestination, size_t n, const char8_t*  pFormat, va_list arguments);
	#endif
	#if defined(EA_WCHAR_UNIQUE) && EA_WCHAR_UNIQUE
		extern int VsnprintfW(wchar_t* pDestination, size_t n, const wchar_t* pFormat, va_list arguments);
	#endif

	namespace eastl
	{
		inline int Vsnprintf(char* pDestination, size_t n, const char* pFormat, va_list arguments)
			{ return Vsnprintf8(pDestination, n, pFormat, arguments); }

		inline int Vsnprintf(char16_t* pDestination, size_t n, const char16_t* pFormat, va_list arguments)
			{ return Vsnprintf16(pDestination, n, pFormat, arguments); }

		inline int Vsnprintf(char32_t* pDestination, size_t n, const char32_t* pFormat, va_list arguments)
			{ return Vsnprintf32(pDestination, n, pFormat, arguments); }

		#if EA_CHAR8_UNIQUE
			inline int Vsnprintf(char8_t* pDestination, size_t n, const char8_t* pFormat, va_list arguments)
				{ return Vsnprintf8(pDestination, n, pFormat, arguments); }
		#endif

		#if defined(EA_WCHAR_UNIQUE) && EA_WCHAR_UNIQUE
			inline int Vsnprintf(wchar_t* pDestination, size_t n, const wchar_t* pFormat, va_list arguments)
				{ return VsnprintfW(pDestination, n, pFormat, arguments); }
		#endif
	}
#endif
///////////////////////////////////////////////////////////////////////////////



namespace eastl
{

	/// EASTL_BASIC_STRING_DEFAULT_NAME
	///
	/// Defines a default container name in the absence of a user-provided name.
	///
	#ifndef EASTL_BASIC_STRING_DEFAULT_NAME
		#define EASTL_BASIC_STRING_DEFAULT_NAME EASTL_DEFAULT_NAME_PREFIX " basic_string" // Unless the user overrides something, this is "EASTL basic_string".
	#endif


	/// EASTL_BASIC_STRING_DEFAULT_ALLOCATOR
	///
	#ifndef EASTL_BASIC_STRING_DEFAULT_ALLOCATOR
		#define EASTL_BASIC_STRING_DEFAULT_ALLOCATOR allocator_type(EASTL_BASIC_STRING_DEFAULT_NAME)
	#endif


	///////////////////////////////////////////////////////////////////////////////
	/// basic_string
	///
	/// Implements a templated string class, somewhat like C++ std::basic_string.
	///
	/// Notes:
	///     As of this writing, an insert of a string into itself necessarily
	///     triggers a reallocation, even if there is enough capacity in self
	///     to handle the increase in size. This is due to the slightly tricky
	///     nature of the operation of modifying one's self with one's self,
	///     and thus the source and destination are being modified during the
	///     operation. It might be useful to rectify this to the extent possible.
	///
	///     Our usage of noexcept specifiers is a little different from the
	///     requirements specified by std::basic_string in C++11. This is because
	///     our allocators are instances and not types and thus can be non-equal
	///     and result in exceptions during assignments that theoretically can't
	///     occur with std containers.
	///
	template <typename T, typename Allocator = EASTLAllocatorType>
	class basic_string
	{
	public:
		typedef basic_string<T, Allocator>                      this_type;
		typedef basic_string_view<T>                            view_type;
		typedef T                                               value_type;
		typedef T*                                              pointer;
		typedef const T*                                        const_pointer;
		typedef T&                                              reference;
		typedef const T&                                        const_reference;
		typedef T*                                              iterator;           // Maintainer note: We want to leave iterator defined as T* -- at least in release builds -- as this gives some algorithms an advantage that optimizers cannot get around.
		typedef const T*                                        const_iterator;
		typedef eastl::reverse_iterator<iterator>               reverse_iterator;
		typedef eastl::reverse_iterator<const_iterator>         const_reverse_iterator;
		typedef eastl_size_t                                    size_type;          // See config.h for the definition of eastl_size_t, which defaults to size_t.
		typedef ptrdiff_t                                       difference_type;
		typedef Allocator                                       allocator_type;

	static const EA_CONSTEXPR size_type npos     = (size_type)-1;      /// 'npos' means non-valid position or simply non-position.

	public:
		// CtorDoNotInitialize exists so that we can create a constructor that allocates but doesn't
		// initialize and also doesn't collide with any other constructor declaration.
		struct CtorDoNotInitialize{};

		// CtorSprintf exists so that we can create a constructor that accepts printf-style
		// arguments but also doesn't collide with any other constructor declaration.
		#ifdef EA_PLATFORM_MINGW
			// Workaround for MinGW compiler bug: variadic arguments are corrupted if empty object is passed before it
			struct CtorSprintf{ int dummy; };
		#else
			struct CtorSprintf{};
		#endif

		// CtorConvert exists so that we can have a constructor that implements string encoding
		// conversion, such as between UCS2 char16_t and UTF8 char8_t.
		struct CtorConvert{};

	protected:
		// Masks used to determine if we are in SSO or Heap
		#ifdef EA_SYSTEM_BIG_ENDIAN
			// Big Endian use LSB, unless we want to reorder struct layouts on endianness, Bit is set when we are in Heap
			static EA_CONSTEXPR_OR_CONST size_type kHeapMask = 0x1;
			static EA_CONSTEXPR_OR_CONST size_type kSSOMask  = 0x1;
		#else
			// Little Endian use MSB
			static EA_CONSTEXPR_OR_CONST size_type kHeapMask = ~(size_type(~size_type(0)) >> 1);
			static EA_CONSTEXPR_OR_CONST size_type kSSOMask  = 0x80;
		#endif

	public:
		#ifdef EA_SYSTEM_BIG_ENDIAN
			static EA_CONSTEXPR_OR_CONST size_type kMaxSize = (~kHeapMask) >> 1;
		#else
			static EA_CONSTEXPR_OR_CONST size_type kMaxSize = ~kHeapMask;
		#endif

	protected:
		// The view of memory when the string data is obtained from the allocator.
		struct HeapLayout
		{
			value_type* mpBegin;  // Begin of string.
			size_type mnSize;     // Size of the string. Number of characters currently in the string, not including the trailing '0'
			size_type mnCapacity; // Capacity of the string. Number of characters string can hold, not including the trailing '0'
		};

		template <typename CharT, size_t = sizeof(CharT)>
		struct SSOPadding
		{
			char padding[sizeof(CharT) - sizeof(char)];
		};

		template <typename CharT>
		struct SSOPadding<CharT, 1>
		{
			// template specialization to remove the padding structure to avoid warnings on zero length arrays
			// also, this allows us to take advantage of the empty-base-class optimization.
		};

		// The view of memory when the string data is able to store the string data locally (without a heap allocation).
		struct SSOLayout
		{
			static EA_CONSTEXPR_OR_CONST size_type SSO_CAPACITY = (sizeof(HeapLayout) - sizeof(char)) / sizeof(value_type);

			// mnSize must correspond to the last byte of HeapLayout.mnCapacity, so we don't want the compiler to insert
			// padding after mnSize if sizeof(value_type) != 1; Also ensures both layouts are the same size.
			struct SSOSize : SSOPadding<value_type>
			{
				char mnRemainingSize;
			};

			value_type mData[SSO_CAPACITY]; // Local buffer for string data.
			SSOSize mRemainingSizeField;
		};

		// This view of memory is a utility structure for easy copying of the string data.
		struct RawLayout
		{
			char mBuffer[sizeof(HeapLayout)];
		};

		static_assert(sizeof(SSOLayout)  == sizeof(HeapLayout), "heap and sso layout structures must be the same size");
		static_assert(sizeof(HeapLayout) == sizeof(RawLayout),  "heap and raw layout structures must be the same size");

		// This implements the 'short string optimization' or SSO. SSO reuses the existing storage of string class to
		// hold string data short enough to fit therefore avoiding a heap allocation. The number of characters stored in
		// the string SSO buffer is variable and depends on the string character width. This implementation favors a
		// consistent string size than increasing the size of the string local data to accommodate a consistent number
		// of characters despite character width.
		struct Layout
		{
			union
			{
				HeapLayout heap;
				SSOLayout sso;
				RawLayout raw;
			};

			Layout()                                                  {
    __builtin_trap() /* STUB: not implemented */;
} // start as SSO by default
			Layout(const Layout& other)                               {
    __builtin_trap() /* STUB: not implemented */;
}
			Layout(Layout&& other)                                    { Move(*this, other); }
			Layout& operator=(const Layout& other)                    {
    __builtin_trap() /* STUB: not implemented */;
}
			Layout& operator=(Layout&& other)                         {
    __builtin_trap() /* STUB: not implemented */;
}

			// We are using Heap when the bit is set, easier to conceptualize checking IsHeap instead of IsSSO
			inline bool IsHeap() const EA_NOEXCEPT                    {
    __builtin_trap() /* STUB: not implemented */;
}
			inline bool IsSSO() const EA_NOEXCEPT                     {
    __builtin_trap() /* STUB: not implemented */;
}
			inline value_type* SSOBufferPtr() EA_NOEXCEPT             {
    __builtin_trap() /* STUB: not implemented */;
}
			inline const value_type* SSOBufferPtr() const EA_NOEXCEPT {
    __builtin_trap() /* STUB: not implemented */;
}

			// Largest value for SSO.mnSize == 23, which has two LSB bits set, but on big-endian (BE)
			// use least significant bit (LSB) to denote heap so shift.
			inline size_type GetSSOSize() const EA_NOEXCEPT
			{
    __builtin_trap() /* STUB: not implemented */;
}
			inline size_type GetHeapSize() const EA_NOEXCEPT {
    __builtin_trap() /* STUB: not implemented */;
}
			inline size_type GetSize() const EA_NOEXCEPT     {
    __builtin_trap() /* STUB: not implemented */;
}

			inline void SetSSOSize(size_type size) EA_NOEXCEPT
			{
    __builtin_trap() /* STUB: not implemented */;
}

			inline void SetHeapSize(size_type size) EA_NOEXCEPT          {
    __builtin_trap() /* STUB: not implemented */;
}
			inline void SetSize(size_type size) EA_NOEXCEPT              {
    __builtin_trap() /* STUB: not implemented */;
}

			inline size_type GetRemainingCapacity() const EA_NOEXCEPT    {
    __builtin_trap() /* STUB: not implemented */;
}

			inline value_type* HeapBeginPtr() EA_NOEXCEPT                {
    __builtin_trap() /* STUB: not implemented */;
};
			inline const value_type* HeapBeginPtr() const EA_NOEXCEPT    {
    __builtin_trap() /* STUB: not implemented */;
};

			inline value_type* SSOBeginPtr() EA_NOEXCEPT                 {
    __builtin_trap() /* STUB: not implemented */;
}
			inline const value_type* SSOBeginPtr() const EA_NOEXCEPT     {
    __builtin_trap() /* STUB: not implemented */;
}

			inline value_type* BeginPtr() EA_NOEXCEPT                    {
    __builtin_trap() /* STUB: not implemented */;
}
			inline const value_type* BeginPtr() const EA_NOEXCEPT        {
    __builtin_trap() /* STUB: not implemented */;
}

			inline value_type* HeapEndPtr() EA_NOEXCEPT                  {
    __builtin_trap() /* STUB: not implemented */;
}
			inline const value_type* HeapEndPtr() const EA_NOEXCEPT      {
    __builtin_trap() /* STUB: not implemented */;
}

			inline value_type* SSOEndPtr() EA_NOEXCEPT                   {
    __builtin_trap() /* STUB: not implemented */;
}
			inline const value_type* SSOEndPtr() const EA_NOEXCEPT       {
    __builtin_trap() /* STUB: not implemented */;
}

			// Points to end of character stream, *ptr == '0'
			inline value_type* EndPtr() EA_NOEXCEPT                      {
    __builtin_trap() /* STUB: not implemented */;
}
			inline const value_type* EndPtr() const EA_NOEXCEPT          {
    __builtin_trap() /* STUB: not implemented */;
}

			inline value_type* HeapCapacityPtr() EA_NOEXCEPT             {
    __builtin_trap() /* STUB: not implemented */;
}
			inline const value_type* HeapCapacityPtr() const EA_NOEXCEPT {
    __builtin_trap() /* STUB: not implemented */;
}

			inline value_type* SSOCapacityPtr() EA_NOEXCEPT               {
    __builtin_trap() /* STUB: not implemented */;
}
			inline const value_type* SSOCapacityPtr() const EA_NOEXCEPT   {
    __builtin_trap() /* STUB: not implemented */;
}

			// Points to end of the buffer at the terminating '0', *ptr == '0' <- only true when size() == capacity()
			inline value_type* CapacityPtr() EA_NOEXCEPT                 {
    __builtin_trap() /* STUB: not implemented */;
}
			inline const value_type* CapacityPtr() const EA_NOEXCEPT     {
    __builtin_trap() /* STUB: not implemented */;
}

			inline void SetHeapBeginPtr(value_type* pBegin) EA_NOEXCEPT  {
    __builtin_trap() /* STUB: not implemented */;
}

			inline void SetHeapCapacity(size_type cap) EA_NOEXCEPT
			{
    __builtin_trap() /* STUB: not implemented */;
}

			inline size_type GetHeapCapacity() const EA_NOEXCEPT
			{
    __builtin_trap() /* STUB: not implemented */;
}

			inline void Copy(Layout& dst, const Layout& src) EA_NOEXCEPT {
    __builtin_trap() /* STUB: not implemented */;
}
			inline void Move(Layout& dst, Layout& src) EA_NOEXCEPT       {
    __builtin_trap() /* STUB: not implemented */;
}
			inline void Swap(Layout& a, Layout& b) EA_NOEXCEPT           {
    __builtin_trap() /* STUB: not implemented */;
}

			inline void ResetToSSO() EA_NOEXCEPT {
    __builtin_trap() /* STUB: not implemented */;
}
		};

		eastl::compressed_pair<Layout, allocator_type> mPair;

		inline Layout& internalLayout() EA_NOEXCEPT                        {
    __builtin_trap() /* STUB: not implemented */;
}
		inline const Layout& internalLayout() const EA_NOEXCEPT            {
    __builtin_trap() /* STUB: not implemented */;
}
		inline allocator_type& internalAllocator() EA_NOEXCEPT             {
    __builtin_trap() /* STUB: not implemented */;
}
		inline const allocator_type& internalAllocator() const EA_NOEXCEPT {
    __builtin_trap() /* STUB: not implemented */;
}

	public:
		// Constructor, destructor
		basic_string() EA_NOEXCEPT_IF(EA_NOEXCEPT_EXPR(EASTL_BASIC_STRING_DEFAULT_ALLOCATOR));
		explicit basic_string(const allocator_type& allocator) EA_NOEXCEPT;
		basic_string(const this_type& x, size_type position, size_type n = npos);
		basic_string(const value_type* p, size_type n, const allocator_type& allocator = EASTL_BASIC_STRING_DEFAULT_ALLOCATOR);
		EASTL_STRING_EXPLICIT basic_string(const value_type* p, const allocator_type& allocator = EASTL_BASIC_STRING_DEFAULT_ALLOCATOR);
		basic_string(size_type n, value_type c, const allocator_type& allocator = EASTL_BASIC_STRING_DEFAULT_ALLOCATOR);
		basic_string(const this_type& x);
	    basic_string(const this_type& x, const allocator_type& allocator);
		basic_string(const value_type* pBegin, const value_type* pEnd, const allocator_type& allocator = EASTL_BASIC_STRING_DEFAULT_ALLOCATOR);
		basic_string(CtorDoNotInitialize, size_type n, const allocator_type& allocator = EASTL_BASIC_STRING_DEFAULT_ALLOCATOR);
		basic_string(CtorSprintf, const value_type* pFormat, ...);
		basic_string(std::initializer_list<value_type> init, const allocator_type& allocator = EASTL_BASIC_STRING_DEFAULT_ALLOCATOR);

		basic_string(this_type&& x) EA_NOEXCEPT;
		basic_string(this_type&& x, const allocator_type& allocator);

		explicit basic_string(const view_type& sv, const allocator_type& allocator = EASTL_BASIC_STRING_DEFAULT_ALLOCATOR);
		basic_string(const view_type& sv, size_type position, size_type n, const allocator_type& allocator = EASTL_BASIC_STRING_DEFAULT_ALLOCATOR);

		template <typename OtherCharType>
		basic_string(CtorConvert, const OtherCharType* p, const allocator_type& allocator = EASTL_BASIC_STRING_DEFAULT_ALLOCATOR);

		template <typename OtherCharType>
		basic_string(CtorConvert, const OtherCharType* p, size_type n, const allocator_type& allocator = EASTL_BASIC_STRING_DEFAULT_ALLOCATOR);

		template <typename OtherStringType> // Unfortunately we need the CtorConvert here because otherwise this function would collide with the value_type* constructor.
		basic_string(CtorConvert, const OtherStringType& x);

	   ~basic_string();

		// Allocator
		const allocator_type& get_allocator() const EA_NOEXCEPT;
		allocator_type&       get_allocator() EA_NOEXCEPT;
		void                  set_allocator(const allocator_type& allocator);

		// Implicit conversion operator
		operator basic_string_view<T>() const EA_NOEXCEPT;

		// Operator=
		this_type& operator=(const this_type& x);
		this_type& operator=(const value_type* p);
		this_type& operator=(value_type c);
		this_type& operator=(std::initializer_list<value_type> ilist);
		this_type& operator=(view_type v);
		this_type& operator=(this_type&& x); // TODO(c++17): noexcept(allocator_traits<Allocator>::propagate_on_container_move_assignment::value || allocator_traits<Allocator>::is_always_equal::value);

		#if EASTL_OPERATOR_EQUALS_OTHER_ENABLED
			this_type& operator=(value_type* p) { return operator=((const value_type*)p); } // We need this because otherwise the const value_type* version can collide with the const OtherStringType& version below.

			template <typename OtherCharType>
			this_type& operator=(const OtherCharType* p);

			template <typename OtherStringType>
			this_type& operator=(const OtherStringType& x);
		#endif

		void swap(this_type& x); // TODO(c++17): noexcept(allocator_traits<Allocator>::propagate_on_container_swap::value || allocator_traits<Allocator>::is_always_equal::value);

		// Assignment operations
		this_type& assign(const this_type& x);
		this_type& assign(const this_type& x, size_type position, size_type n = npos);
		this_type& assign(const value_type* p, size_type n);
		this_type& assign(const value_type* p);
		this_type& assign(size_type n, value_type c);
		this_type& assign(const value_type* pBegin, const value_type* pEnd);
		this_type& assign(this_type&& x); // TODO(c++17): noexcept(allocator_traits<Allocator>::propagate_on_container_move_assignment::value || allocator_traits<Allocator>::is_always_equal::value);
		this_type& assign(std::initializer_list<value_type>);

		template <typename OtherCharType>
		this_type& assign_convert(const OtherCharType* p);

		template <typename OtherCharType>
		this_type& assign_convert(const OtherCharType* p, size_type n);

		template <typename OtherStringType>
		this_type& assign_convert(const OtherStringType& x);

		// Iterators.
		iterator       begin() EA_NOEXCEPT;                 // Expanded in source code as: mpBegin
		const_iterator begin() const EA_NOEXCEPT;           // Expanded in source code as: mpBegin
		const_iterator cbegin() const EA_NOEXCEPT;

		iterator       end() EA_NOEXCEPT;                   // Expanded in source code as: mpEnd
		const_iterator end() const EA_NOEXCEPT;             // Expanded in source code as: mpEnd
		const_iterator cend() const EA_NOEXCEPT;

		reverse_iterator       rbegin() EA_NOEXCEPT;
		const_reverse_iterator rbegin() const EA_NOEXCEPT;
		const_reverse_iterator crbegin() const EA_NOEXCEPT;

		reverse_iterator       rend() EA_NOEXCEPT;
		const_reverse_iterator rend() const EA_NOEXCEPT;
		const_reverse_iterator crend() const EA_NOEXCEPT;


		// Size-related functionality
		bool      empty() const EA_NOEXCEPT;
		size_type size() const EA_NOEXCEPT;
		size_type length() const EA_NOEXCEPT;
		size_type max_size() const EA_NOEXCEPT;
		size_type capacity() const EA_NOEXCEPT;
		void      resize(size_type n, value_type c);
		void      resize(size_type n);
		void      reserve(size_type = 0);
		void      set_capacity(size_type n = npos); // Revises the capacity to the user-specified value. Resizes the container to match the capacity if the requested capacity n is less than the current size. If n == npos then the capacity is reallocated (if necessary) such that capacity == size.
		void      force_size(size_type n);          // Unilaterally moves the string end position (mpEnd) to the given location. Useful for when the user writes into the string via some extenal means such as C strcpy or sprintf. This allows for more efficient use than using resize to achieve this.
		void shrink_to_fit();

		// Raw access
		const value_type* data() const  EA_NOEXCEPT;
		      value_type* data()        EA_NOEXCEPT;
		const value_type* c_str() const EA_NOEXCEPT;

		// Element access
		reference       operator[](size_type n);
		const_reference operator[](size_type n) const;
		reference       at(size_type n);
		const_reference at(size_type n) const;
		reference       front();
		const_reference front() const;
		reference       back();
		const_reference back() const;

		// Append operations
		this_type& operator+=(const this_type& x);
		this_type& operator+=(const value_type* p);
		this_type& operator+=(value_type c);

		this_type& append(const this_type& x);
		this_type& append(const this_type& x,  size_type position, size_type n = npos);
		this_type& append(const value_type* p, size_type n);
		this_type& append(const value_type* p);
		this_type& append(size_type n, value_type c);
		this_type& append(const value_type* pBegin, const value_type* pEnd);

		this_type& append_sprintf_va_list(const value_type* pFormat, va_list arguments);
		this_type& append_sprintf(const value_type* pFormat, ...);

		template <typename OtherCharType>
		this_type& append_convert(const OtherCharType* p);

		template <typename OtherCharType>
		this_type& append_convert(const OtherCharType* p, size_type n);

		template <typename OtherStringType>
		this_type& append_convert(const OtherStringType& x);

		void push_back(value_type c);
		void pop_back();

		// Insertion operations
		this_type& insert(size_type position, const this_type& x);
		this_type& insert(size_type position, const this_type& x, size_type beg, size_type n);
		this_type& insert(size_type position, const value_type* p, size_type n);
		this_type& insert(size_type position, const value_type* p);
		this_type& insert(size_type position, size_type n, value_type c);
		iterator   insert(const_iterator p, value_type c);
		iterator   insert(const_iterator p, size_type n, value_type c);
		iterator   insert(const_iterator p, const value_type* pBegin, const value_type* pEnd);
		iterator   insert(const_iterator p, std::initializer_list<value_type>);

		// Erase operations
		this_type&       erase(size_type position = 0, size_type n = npos);
		iterator         erase(const_iterator p);
		iterator         erase(const_iterator pBegin, const_iterator pEnd);
		reverse_iterator erase(reverse_iterator position);
		reverse_iterator erase(reverse_iterator first, reverse_iterator last);
		void             clear() EA_NOEXCEPT;

		// Detach memory
		pointer detach() EA_NOEXCEPT;

		// Replacement operations
		this_type&  replace(size_type position, size_type n,  const this_type& x);
		this_type&  replace(size_type pos1,     size_type n1, const this_type& x,  size_type pos2, size_type n2 = npos);
		this_type&  replace(size_type position, size_type n1, const value_type* p, size_type n2);
		this_type&  replace(size_type position, size_type n1, const value_type* p);
		this_type&  replace(size_type position, size_type n1, size_type n2, value_type c);
		this_type&  replace(const_iterator first, const_iterator last, const this_type& x);
		this_type&  replace(const_iterator first, const_iterator last, const value_type* p, size_type n);
		this_type&  replace(const_iterator first, const_iterator last, const value_type* p);
		this_type&  replace(const_iterator first, const_iterator last, size_type n, value_type c);
		this_type&  replace(const_iterator first, const_iterator last, const value_type* pBegin, const value_type* pEnd);
		size_type   copy(value_type* p, size_type n, size_type position = 0) const;

		// Find operations
		size_type find(const this_type& x,  size_type position = 0) const EA_NOEXCEPT;
		size_type find(const value_type* p, size_type position = 0) const;
		size_type find(const value_type* p, size_type position, size_type n) const;
		size_type find(value_type c, size_type position = 0) const EA_NOEXCEPT;

		// Reverse find operations
		size_type rfind(const this_type& x,  size_type position = npos) const EA_NOEXCEPT;
		size_type rfind(const value_type* p, size_type position = npos) const;
		size_type rfind(const value_type* p, size_type position, size_type n) const;
		size_type rfind(value_type c, size_type position = npos) const EA_NOEXCEPT;

		// Find first-of operations
		size_type find_first_of(const this_type& x, size_type position = 0) const EA_NOEXCEPT;
		size_type find_first_of(const value_type* p, size_type position = 0) const;
		size_type find_first_of(const value_type* p, size_type position, size_type n) const;
		size_type find_first_of(value_type c, size_type position = 0) const EA_NOEXCEPT;

		// Find last-of operations
		size_type find_last_of(const this_type& x, size_type position = npos) const EA_NOEXCEPT;
		size_type find_last_of(const value_type* p, size_type position = npos) const;
		size_type find_last_of(const value_type* p, size_type position, size_type n) const;
		size_type find_last_of(value_type c, size_type position = npos) const EA_NOEXCEPT;

		// Find first not-of operations
		size_type find_first_not_of(const this_type& x, size_type position = 0) const EA_NOEXCEPT;
		size_type find_first_not_of(const value_type* p, size_type position = 0) const;
		size_type find_first_not_of(const value_type* p, size_type position, size_type n) const;
		size_type find_first_not_of(value_type c, size_type position = 0) const EA_NOEXCEPT;

		// Find last not-of operations
		size_type find_last_not_of(const this_type& x,  size_type position = npos) const EA_NOEXCEPT;
		size_type find_last_not_of(const value_type* p, size_type position = npos) const;
		size_type find_last_not_of(const value_type* p, size_type position, size_type n) const;
		size_type find_last_not_of(value_type c, size_type position = npos) const EA_NOEXCEPT;

		// Substring functionality
		this_type substr(size_type position = 0, size_type n = npos) const;

		// Comparison operations
		int        compare(const this_type& x) const EA_NOEXCEPT;
		int        compare(size_type pos1, size_type n1, const this_type& x) const;
		int        compare(size_type pos1, size_type n1, const this_type& x, size_type pos2, size_type n2) const;
		int        compare(const value_type* p) const;
		int        compare(size_type pos1, size_type n1, const value_type* p) const;
		int        compare(size_type pos1, size_type n1, const value_type* p, size_type n2) const;
		static int compare(const value_type* pBegin1, const value_type* pEnd1, const value_type* pBegin2, const value_type* pEnd2);

		// Case-insensitive comparison functions. Not part of C++ this_type. Only ASCII-level locale functionality is supported. Thus this is not suitable for localization purposes.
		int        comparei(const this_type& x) const EA_NOEXCEPT;
		int        comparei(const value_type* p) const;
		static int comparei(const value_type* pBegin1, const value_type* pEnd1, const value_type* pBegin2, const value_type* pEnd2);

		// Misc functionality, not part of C++ this_type.
		void         make_lower();
		void         make_upper();
		void         ltrim();
		void         rtrim();
		void         trim();
		void         ltrim(const value_type* p);
		void         rtrim(const value_type* p);
		void         trim(const value_type* p);
		this_type    left(size_type n) const;
		this_type    right(size_type n) const;
		this_type&   sprintf_va_list(const value_type* pFormat, va_list arguments);
		this_type&   sprintf(const value_type* pFormat, ...);

		bool validate() const EA_NOEXCEPT;
		int  validate_iterator(const_iterator i) const EA_NOEXCEPT;


	protected:
		// Helper functions for initialization/insertion operations.
		value_type* DoAllocate(size_type n);
		void        DoFree(value_type* p, size_type n);
		size_type   GetNewCapacity(size_type currentCapacity);
		size_type   GetNewCapacity(size_type currentCapacity, size_type minimumGrowSize);
		void        AllocateSelf();
		void        AllocateSelf(size_type n);
		void        DeallocateSelf();
		iterator    InsertInternal(const_iterator p, value_type c);
		void        RangeInitialize(const value_type* pBegin, const value_type* pEnd);
		void        RangeInitialize(const value_type* pBegin);
		void        SizeInitialize(size_type n, value_type c);

		bool        IsSSO() const EA_NOEXCEPT;

		void        ThrowLengthException() const;
		void        ThrowRangeException() const;
		void        ThrowInvalidArgumentException() const;

		#if EASTL_OPERATOR_EQUALS_OTHER_ENABLED
			template <typename CharType>
			void DoAssignConvert(CharType c, true_type);

			template <typename StringType>
			void DoAssignConvert(const StringType& x, false_type);
		#endif
	}; // basic_string





	///////////////////////////////////////////////////////////////////////////////
	// basic_string
	///////////////////////////////////////////////////////////////////////////////

	template <typename T, typename Allocator>
	inline basic_string<T, Allocator>::basic_string() EA_NOEXCEPT_IF(EA_NOEXCEPT_EXPR(EASTL_BASIC_STRING_DEFAULT_ALLOCATOR))
	    : mPair(allocator_type(EASTL_BASIC_STRING_DEFAULT_NAME))
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline basic_string<T, Allocator>::basic_string(const allocator_type& allocator) EA_NOEXCEPT
	    : mPair(allocator)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline basic_string<T, Allocator>::basic_string(const this_type& x)
	    : mPair(x.get_allocator())
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	basic_string<T, Allocator>::basic_string(const this_type& x, const allocator_type& allocator)
		: mPair(allocator)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	template <typename OtherStringType>
	inline basic_string<T, Allocator>::basic_string(CtorConvert, const OtherStringType& x)
	    : mPair(x.get_allocator())
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	basic_string<T, Allocator>::basic_string(const this_type& x, size_type position, size_type n)
		: mPair(x.get_allocator())
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline basic_string<T, Allocator>::basic_string(const value_type* p, size_type n, const allocator_type& allocator)
		: mPair(allocator)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline basic_string<T, Allocator>::basic_string(const view_type& sv, const allocator_type& allocator)
	    : basic_string(sv.data(), static_cast<size_type>(sv.size()), allocator)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline basic_string<T, Allocator>::basic_string(const view_type& sv, size_type position, size_type n, const allocator_type& allocator)
	    : basic_string(sv.substr(position, n), allocator)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	template <typename OtherCharType>
	inline basic_string<T, Allocator>::basic_string(CtorConvert, const OtherCharType* p, const allocator_type& allocator)
		: mPair(allocator)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	template <typename OtherCharType>
	inline basic_string<T, Allocator>::basic_string(CtorConvert, const OtherCharType* p, size_type n, const allocator_type& allocator)
		: mPair(allocator)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline basic_string<T, Allocator>::basic_string(const value_type* p, const allocator_type& allocator)
		: mPair(allocator)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline basic_string<T, Allocator>::basic_string(size_type n, value_type c, const allocator_type& allocator)
		: mPair(allocator)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline basic_string<T, Allocator>::basic_string(const value_type* pBegin, const value_type* pEnd, const allocator_type& allocator)
		: mPair(allocator)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	// CtorDoNotInitialize exists so that we can create a version that allocates but doesn't
	// initialize but also doesn't collide with any other constructor declaration.
	template <typename T, typename Allocator>
	basic_string<T, Allocator>::basic_string(CtorDoNotInitialize /*unused*/, size_type n, const allocator_type& allocator)
		: mPair(allocator)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	// CtorSprintf exists so that we can create a version that does a variable argument
	// sprintf but also doesn't collide with any other constructor declaration.
	template <typename T, typename Allocator>
	basic_string<T, Allocator>::basic_string(CtorSprintf /*unused*/, const value_type* pFormat, ...)
		: mPair()
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	basic_string<T, Allocator>::basic_string(std::initializer_list<value_type> init, const allocator_type& allocator)
		: mPair(allocator)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	basic_string<T, Allocator>::basic_string(this_type&& x) EA_NOEXCEPT
		: mPair(x.get_allocator())
	{
		internalLayout() = eastl::move(x.internalLayout());
		x.AllocateSelf();
	}


	template <typename T, typename Allocator>
	basic_string<T, Allocator>::basic_string(this_type&& x, const allocator_type& allocator)
	: mPair(allocator)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline basic_string<T, Allocator>::~basic_string()
	{
		DeallocateSelf();
	}


	template <typename T, typename Allocator>
	inline const typename basic_string<T, Allocator>::allocator_type&
	basic_string<T, Allocator>::get_allocator() const EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename basic_string<T, Allocator>::allocator_type&
	basic_string<T, Allocator>::get_allocator() EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline void basic_string<T, Allocator>::set_allocator(const allocator_type& allocator)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline const typename basic_string<T, Allocator>::value_type*
	basic_string<T, Allocator>::data()  const EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline const typename basic_string<T, Allocator>::value_type*
	basic_string<T, Allocator>::c_str() const EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T, typename Allocator>
	inline typename basic_string<T, Allocator>::value_type*
	basic_string<T, Allocator>::data() EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T, typename Allocator>
	inline typename basic_string<T, Allocator>::iterator
	basic_string<T, Allocator>::begin() EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename basic_string<T, Allocator>::iterator
	basic_string<T, Allocator>::end() EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename basic_string<T, Allocator>::const_iterator
	basic_string<T, Allocator>::begin() const EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename basic_string<T, Allocator>::const_iterator
	basic_string<T, Allocator>::cbegin() const EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename basic_string<T, Allocator>::const_iterator
	basic_string<T, Allocator>::end() const EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename basic_string<T, Allocator>::const_iterator
	basic_string<T, Allocator>::cend() const EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename basic_string<T, Allocator>::reverse_iterator
	basic_string<T, Allocator>::rbegin() EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename basic_string<T, Allocator>::reverse_iterator
	basic_string<T, Allocator>::rend() EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename basic_string<T, Allocator>::const_reverse_iterator
	basic_string<T, Allocator>::rbegin() const EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename basic_string<T, Allocator>::const_reverse_iterator
	basic_string<T, Allocator>::crbegin() const EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename basic_string<T, Allocator>::const_reverse_iterator
	basic_string<T, Allocator>::rend() const EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename basic_string<T, Allocator>::const_reverse_iterator
	basic_string<T, Allocator>::crend() const EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline bool basic_string<T, Allocator>::empty() const EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline bool basic_string<T, Allocator>::IsSSO() const EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename basic_string<T, Allocator>::size_type
	basic_string<T, Allocator>::size() const EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename basic_string<T, Allocator>::size_type
	basic_string<T, Allocator>::length() const EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename basic_string<T, Allocator>::size_type
	basic_string<T, Allocator>::max_size() const EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename basic_string<T, Allocator>::size_type
	basic_string<T, Allocator>::capacity() const EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename basic_string<T, Allocator>::const_reference
	basic_string<T, Allocator>::operator[](size_type n) const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename basic_string<T, Allocator>::reference
	basic_string<T, Allocator>::operator[](size_type n)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	basic_string<T,Allocator>::operator basic_string_view<T>() const EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename basic_string<T, Allocator>::this_type& basic_string<T, Allocator>::operator=(const this_type& x)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	#if EASTL_OPERATOR_EQUALS_OTHER_ENABLED
		template <typename T, typename Allocator>
		template <typename CharType>
		inline void basic_string<T, Allocator>::DoAssignConvert(CharType c, true_type)
		{
			assign_convert(&c, 1); // Call this version of append because it will result in the encoding-converting append being used.
		}


		template <typename T, typename Allocator>
		template <typename StringType>
		inline void basic_string<T, Allocator>::DoAssignConvert(const StringType& x, false_type)
		{
			//if(&x != this) // Unnecessary because &x cannot possibly equal this.
			{
				#if EASTL_ALLOCATOR_COPY_ENABLED
					get_allocator() = x.get_allocator();
				#endif

				assign_convert(x.c_str(), x.length());
			}
		}


		template <typename T, typename Allocator>
		template <typename OtherStringType>
		inline typename basic_string<T, Allocator>::this_type& basic_string<T, Allocator>::operator=(const OtherStringType& x)
		{
			clear();
			DoAssignConvert(x, is_integral<OtherStringType>());
			return *this;
		}


		template <typename T, typename Allocator>
		template <typename OtherCharType>
		inline typename basic_string<T, Allocator>::this_type& basic_string<T, Allocator>::operator=(const OtherCharType* p)
		{
			return assign_convert(p);
		}
	#endif


	template <typename T, typename Allocator>
	inline typename basic_string<T, Allocator>::this_type& basic_string<T, Allocator>::operator=(const value_type* p)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T, typename Allocator>
	inline typename basic_string<T, Allocator>::this_type& basic_string<T, Allocator>::operator=(value_type c)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename basic_string<T, Allocator>::this_type& basic_string<T, Allocator>::operator=(this_type&& x)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename basic_string<T, Allocator>::this_type& basic_string<T, Allocator>::operator=(std::initializer_list<value_type> ilist)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename basic_string<T, Allocator>::this_type& basic_string<T, Allocator>::operator=(view_type v)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	void basic_string<T, Allocator>::resize(size_type n, value_type c)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	void basic_string<T, Allocator>::resize(size_type n)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	void basic_string<T, Allocator>::reserve(size_type n)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline void basic_string<T, Allocator>::shrink_to_fit()
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline void basic_string<T, Allocator>::set_capacity(size_type n)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline void basic_string<T, Allocator>::force_size(size_type n)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline void basic_string<T, Allocator>::clear() EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename basic_string<T, Allocator>::pointer
	basic_string<T, Allocator>::detach() EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename basic_string<T, Allocator>::const_reference
	basic_string<T, Allocator>::at(size_type n) const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename basic_string<T, Allocator>::reference
	basic_string<T, Allocator>::at(size_type n)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename basic_string<T, Allocator>::reference
	basic_string<T, Allocator>::front()
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename basic_string<T, Allocator>::const_reference
	basic_string<T, Allocator>::front() const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename basic_string<T, Allocator>::reference
	basic_string<T, Allocator>::back()
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename basic_string<T, Allocator>::const_reference
	basic_string<T, Allocator>::back() const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline basic_string<T, Allocator>& basic_string<T, Allocator>::operator+=(const this_type& x)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline basic_string<T, Allocator>& basic_string<T, Allocator>::operator+=(const value_type* p)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline basic_string<T, Allocator>& basic_string<T, Allocator>::operator+=(value_type c)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline basic_string<T, Allocator>& basic_string<T, Allocator>::append(const this_type& x)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline basic_string<T, Allocator>& basic_string<T, Allocator>::append(const this_type& x, size_type position, size_type n)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline basic_string<T, Allocator>& basic_string<T, Allocator>::append(const value_type* p, size_type n)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline basic_string<T, Allocator>& basic_string<T, Allocator>::append(const value_type* p)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	template <typename OtherCharType>
	basic_string<T, Allocator>& basic_string<T, Allocator>::append_convert(const OtherCharType* pOther)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	template <typename OtherStringType>
	basic_string<T, Allocator>& basic_string<T, Allocator>::append_convert(const OtherStringType& x)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	template <typename OtherCharType>
	basic_string<T, Allocator>& basic_string<T, Allocator>::append_convert(const OtherCharType* pOther, size_type n)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	basic_string<T, Allocator>& basic_string<T, Allocator>::append(size_type n, value_type c)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	basic_string<T, Allocator>& basic_string<T, Allocator>::append(const value_type* pBegin, const value_type* pEnd)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	basic_string<T, Allocator>& basic_string<T, Allocator>::append_sprintf_va_list(const value_type* pFormat, va_list arguments)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T, typename Allocator>
	basic_string<T, Allocator>& basic_string<T, Allocator>::append_sprintf(const value_type* pFormat, ...)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline void basic_string<T, Allocator>::push_back(value_type c)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline void basic_string<T, Allocator>::pop_back()
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline basic_string<T, Allocator>& basic_string<T, Allocator>::assign(const this_type& x)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline basic_string<T, Allocator>& basic_string<T, Allocator>::assign(const this_type& x, size_type position, size_type n)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline basic_string<T, Allocator>& basic_string<T, Allocator>::assign(const value_type* p, size_type n)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline basic_string<T, Allocator>& basic_string<T, Allocator>::assign(const value_type* p)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	basic_string<T, Allocator>& basic_string<T, Allocator>::assign(size_type n, value_type c)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	basic_string<T, Allocator>& basic_string<T, Allocator>::assign(const value_type* pBegin, const value_type* pEnd)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline basic_string<T, Allocator>& basic_string<T, Allocator>::assign(std::initializer_list<value_type> ilist)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline basic_string<T, Allocator>& basic_string<T, Allocator>::assign(this_type&& x)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	template <typename OtherCharType>
	basic_string<T, Allocator>& basic_string<T, Allocator>::assign_convert(const OtherCharType* p)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	template <typename OtherCharType>
	basic_string<T, Allocator>& basic_string<T, Allocator>::assign_convert(const OtherCharType* p, size_type n)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	template <typename OtherStringType>
	basic_string<T, Allocator>& basic_string<T, Allocator>::assign_convert(const OtherStringType& x)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	basic_string<T, Allocator>& basic_string<T, Allocator>::insert(size_type position, const this_type& x)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	basic_string<T, Allocator>& basic_string<T, Allocator>::insert(size_type position, const this_type& x, size_type beg, size_type n)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	basic_string<T, Allocator>& basic_string<T, Allocator>::insert(size_type position, const value_type* p, size_type n)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	basic_string<T, Allocator>& basic_string<T, Allocator>::insert(size_type position, const value_type* p)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	basic_string<T, Allocator>& basic_string<T, Allocator>::insert(size_type position, size_type n, value_type c)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename basic_string<T, Allocator>::iterator
	basic_string<T, Allocator>::insert(const_iterator p, value_type c)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	typename basic_string<T, Allocator>::iterator
	basic_string<T, Allocator>::insert(const_iterator p, size_type n, value_type c)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	typename basic_string<T, Allocator>::iterator
	basic_string<T, Allocator>::insert(const_iterator p, const value_type* pBegin, const value_type* pEnd)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	typename basic_string<T, Allocator>::iterator
	basic_string<T, Allocator>::insert(const_iterator p, std::initializer_list<value_type> ilist)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline basic_string<T, Allocator>& basic_string<T, Allocator>::erase(size_type position, size_type n)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename basic_string<T, Allocator>::iterator
	basic_string<T, Allocator>::erase(const_iterator p)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	typename basic_string<T, Allocator>::iterator
	basic_string<T, Allocator>::erase(const_iterator pBegin, const_iterator pEnd)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename basic_string<T, Allocator>::reverse_iterator
	basic_string<T, Allocator>::erase(reverse_iterator position)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	typename basic_string<T, Allocator>::reverse_iterator
	basic_string<T, Allocator>::erase(reverse_iterator first, reverse_iterator last)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	basic_string<T, Allocator>& basic_string<T, Allocator>::replace(size_type position, size_type n, const this_type& x)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	basic_string<T, Allocator>& basic_string<T, Allocator>::replace(size_type pos1, size_type n1, const this_type& x, size_type pos2, size_type n2)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	basic_string<T, Allocator>& basic_string<T, Allocator>::replace(size_type position, size_type n1, const value_type* p, size_type n2)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	basic_string<T, Allocator>& basic_string<T, Allocator>::replace(size_type position, size_type n1, const value_type* p)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	basic_string<T, Allocator>& basic_string<T, Allocator>::replace(size_type position, size_type n1, size_type n2, value_type c)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline basic_string<T, Allocator>& basic_string<T, Allocator>::replace(const_iterator pBegin, const_iterator pEnd, const this_type& x)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline basic_string<T, Allocator>& basic_string<T, Allocator>::replace(const_iterator pBegin, const_iterator pEnd, const value_type* p, size_type n)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline basic_string<T, Allocator>& basic_string<T, Allocator>::replace(const_iterator pBegin, const_iterator pEnd, const value_type* p)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	basic_string<T, Allocator>& basic_string<T, Allocator>::replace(const_iterator pBegin, const_iterator pEnd, size_type n, value_type c)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	basic_string<T, Allocator>& basic_string<T, Allocator>::replace(const_iterator pBegin1, const_iterator pEnd1, const value_type* pBegin2, const value_type* pEnd2)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	typename basic_string<T, Allocator>::size_type
	basic_string<T, Allocator>::copy(value_type* p, size_type n, size_type position) const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	void basic_string<T, Allocator>::swap(this_type& x)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename basic_string<T, Allocator>::size_type
	basic_string<T, Allocator>::find(const this_type& x, size_type position) const EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename basic_string<T, Allocator>::size_type
	basic_string<T, Allocator>::find(const value_type* p, size_type position) const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	typename basic_string<T, Allocator>::size_type
	basic_string<T, Allocator>::find(const value_type* p, size_type position, size_type n) const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	typename basic_string<T, Allocator>::size_type
	basic_string<T, Allocator>::find(value_type c, size_type position) const EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename basic_string<T, Allocator>::size_type
	basic_string<T, Allocator>::rfind(const this_type& x, size_type position) const EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename basic_string<T, Allocator>::size_type
	basic_string<T, Allocator>::rfind(const value_type* p, size_type position) const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	typename basic_string<T, Allocator>::size_type
	basic_string<T, Allocator>::rfind(const value_type* p, size_type position, size_type n) const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	typename basic_string<T, Allocator>::size_type
	basic_string<T, Allocator>::rfind(value_type c, size_type position) const EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename basic_string<T, Allocator>::size_type
	basic_string<T, Allocator>::find_first_of(const this_type& x, size_type position) const EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename basic_string<T, Allocator>::size_type
	basic_string<T, Allocator>::find_first_of(const value_type* p, size_type position) const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	typename basic_string<T, Allocator>::size_type
	basic_string<T, Allocator>::find_first_of(const value_type* p, size_type position, size_type n) const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename basic_string<T, Allocator>::size_type
	basic_string<T, Allocator>::find_first_of(value_type c, size_type position) const EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename basic_string<T, Allocator>::size_type
	basic_string<T, Allocator>::find_last_of(const this_type& x, size_type position) const EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename basic_string<T, Allocator>::size_type
	basic_string<T, Allocator>::find_last_of(const value_type* p, size_type position) const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	typename basic_string<T, Allocator>::size_type
	basic_string<T, Allocator>::find_last_of(const value_type* p, size_type position, size_type n) const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename basic_string<T, Allocator>::size_type
	basic_string<T, Allocator>::find_last_of(value_type c, size_type position) const EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename basic_string<T, Allocator>::size_type
	basic_string<T, Allocator>::find_first_not_of(const this_type& x, size_type position) const EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename basic_string<T, Allocator>::size_type
	basic_string<T, Allocator>::find_first_not_of(const value_type* p, size_type position) const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	typename basic_string<T, Allocator>::size_type
	basic_string<T, Allocator>::find_first_not_of(const value_type* p, size_type position, size_type n) const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	typename basic_string<T, Allocator>::size_type
	basic_string<T, Allocator>::find_first_not_of(value_type c, size_type position) const EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename basic_string<T, Allocator>::size_type
	basic_string<T, Allocator>::find_last_not_of(const this_type& x, size_type position) const EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename basic_string<T, Allocator>::size_type
	basic_string<T, Allocator>::find_last_not_of(const value_type* p, size_type position) const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	typename basic_string<T, Allocator>::size_type
	basic_string<T, Allocator>::find_last_not_of(const value_type* p, size_type position, size_type n) const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	typename basic_string<T, Allocator>::size_type
	basic_string<T, Allocator>::find_last_not_of(value_type c, size_type position) const EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline basic_string<T, Allocator> basic_string<T, Allocator>::substr(size_type position, size_type n) const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline int basic_string<T, Allocator>::compare(const this_type& x) const EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline int basic_string<T, Allocator>::compare(size_type pos1, size_type n1, const this_type& x) const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline int basic_string<T, Allocator>::compare(size_type pos1, size_type n1, const this_type& x, size_type pos2, size_type n2) const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline int basic_string<T, Allocator>::compare(const value_type* p) const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline int basic_string<T, Allocator>::compare(size_type pos1, size_type n1, const value_type* p) const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline int basic_string<T, Allocator>::compare(size_type pos1, size_type n1, const value_type* p, size_type n2) const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	// make_lower
	// This is a very simple ASCII-only case conversion function
	// Anything more complicated should use a more powerful separate library.
	template <typename T, typename Allocator>
	inline void basic_string<T, Allocator>::make_lower()
	{
    __builtin_trap() /* STUB: not implemented */;
}


	// make_upper
	// This is a very simple ASCII-only case conversion function
	// Anything more complicated should use a more powerful separate library.
	template <typename T, typename Allocator>
	inline void basic_string<T, Allocator>::make_upper()
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline void basic_string<T, Allocator>::ltrim()
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline void basic_string<T, Allocator>::rtrim()
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline void basic_string<T, Allocator>::trim()
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline void basic_string<T, Allocator>::ltrim(const value_type* p)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline void basic_string<T, Allocator>::rtrim(const value_type* p)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline void basic_string<T, Allocator>::trim(const value_type* p)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline basic_string<T, Allocator> basic_string<T, Allocator>::left(size_type n) const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline basic_string<T, Allocator> basic_string<T, Allocator>::right(size_type n) const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline basic_string<T, Allocator>& basic_string<T, Allocator>::sprintf(const value_type* pFormat, ...)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	basic_string<T, Allocator>& basic_string<T, Allocator>::sprintf_va_list(const value_type* pFormat, va_list arguments)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	int basic_string<T, Allocator>::compare(const value_type* pBegin1, const value_type* pEnd1,
											const value_type* pBegin2, const value_type* pEnd2)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	int basic_string<T, Allocator>::comparei(const value_type* pBegin1, const value_type* pEnd1,
											 const value_type* pBegin2, const value_type* pEnd2)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline int basic_string<T, Allocator>::comparei(const this_type& x) const EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline int basic_string<T, Allocator>::comparei(const value_type* p) const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	typename basic_string<T, Allocator>::iterator
	basic_string<T, Allocator>::InsertInternal(const_iterator p, value_type c)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	void basic_string<T, Allocator>::SizeInitialize(size_type n, value_type c)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	void basic_string<T, Allocator>::RangeInitialize(const value_type* pBegin, const value_type* pEnd)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline void basic_string<T, Allocator>::RangeInitialize(const value_type* pBegin)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename basic_string<T, Allocator>::value_type*
	basic_string<T, Allocator>::DoAllocate(size_type n)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline void basic_string<T, Allocator>::DoFree(value_type* p, size_type n)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename basic_string<T, Allocator>::size_type
	basic_string<T, Allocator>::GetNewCapacity(size_type currentCapacity)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline typename basic_string<T, Allocator>::size_type
	basic_string<T, Allocator>::GetNewCapacity(size_type currentCapacity, size_type minimumGrowSize)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline void basic_string<T, Allocator>::AllocateSelf()
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	void basic_string<T, Allocator>::AllocateSelf(size_type n)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline void basic_string<T, Allocator>::DeallocateSelf()
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline void basic_string<T, Allocator>::ThrowLengthException() const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline void basic_string<T, Allocator>::ThrowRangeException() const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline void basic_string<T, Allocator>::ThrowInvalidArgumentException() const
	{
    __builtin_trap() /* STUB: not implemented */;
}

	// iterator operators
	template <typename T, typename Allocator>
	inline bool operator==(const typename basic_string<T, Allocator>::reverse_iterator& r1,
						   const typename basic_string<T, Allocator>::reverse_iterator& r2)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline bool operator!=(const typename basic_string<T, Allocator>::reverse_iterator& r1,
						   const typename basic_string<T, Allocator>::reverse_iterator& r2)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	// Operator +
	template <typename T, typename Allocator>
	basic_string<T, Allocator> operator+(const basic_string<T, Allocator>& a, const basic_string<T, Allocator>& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	basic_string<T, Allocator> operator+(const typename basic_string<T, Allocator>::value_type* p, const basic_string<T, Allocator>& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	basic_string<T, Allocator> operator+(typename basic_string<T, Allocator>::value_type c, const basic_string<T, Allocator>& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	basic_string<T, Allocator> operator+(const basic_string<T, Allocator>& a, const typename basic_string<T, Allocator>::value_type* p)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	basic_string<T, Allocator> operator+(const basic_string<T, Allocator>& a, typename basic_string<T, Allocator>::value_type c)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	basic_string<T, Allocator> operator+(basic_string<T, Allocator>&& a, basic_string<T, Allocator>&& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T, typename Allocator>
	basic_string<T, Allocator> operator+(basic_string<T, Allocator>&& a, const basic_string<T, Allocator>& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T, typename Allocator>
	basic_string<T, Allocator> operator+(const typename basic_string<T, Allocator>::value_type* p, basic_string<T, Allocator>&& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T, typename Allocator>
	basic_string<T, Allocator> operator+(basic_string<T, Allocator>&& a, const typename basic_string<T, Allocator>::value_type* p)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T, typename Allocator>
	basic_string<T, Allocator> operator+(basic_string<T, Allocator>&& a, typename basic_string<T, Allocator>::value_type c)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline bool basic_string<T, Allocator>::validate() const EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline int basic_string<T, Allocator>::validate_iterator(const_iterator i) const EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	///////////////////////////////////////////////////////////////////////
	// global operators
	///////////////////////////////////////////////////////////////////////

	// Operator== and operator!=
	template <typename T, typename Allocator>
	inline bool operator==(const basic_string<T, Allocator>& a, const basic_string<T, Allocator>& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}

#if !defined(EA_COMPILER_HAS_THREE_WAY_COMPARISON)
	template <typename T, typename Allocator>
	inline bool operator==(const typename basic_string<T, Allocator>::value_type* p, const basic_string<T, Allocator>& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}
#endif

	template <typename T, typename Allocator>
	inline bool operator==(const basic_string<T, Allocator>& a, const typename basic_string<T, Allocator>::value_type* p)
	{
    __builtin_trap() /* STUB: not implemented */;
}
	
#if defined(EA_COMPILER_HAS_THREE_WAY_COMPARISON)
	template <typename T, typename Allocator>
	inline auto operator<=>(const basic_string<T, Allocator>& a, const basic_string<T, Allocator>& b)
	{
		return basic_string<T, Allocator>::compare(a.begin(), a.end(), b.begin(), b.end()) <=> 0;
	}

	template <typename T, typename Allocator>
	inline auto operator<=>(const basic_string<T, Allocator>& a, const typename basic_string<T, Allocator>::value_type* p)
	{
		typedef typename basic_string<T, Allocator>::size_type string_size_type;
		const string_size_type n = (string_size_type)CharStrlen(p);
		return basic_string<T, Allocator>::compare(a.begin(), a.end(), p, p + n) <=> 0;
	}
	
	template <typename T, typename Allocator>
	inline auto operator<=>(const basic_string<T, Allocator>& a, const typename basic_string<T, Allocator>::view_type v)
	{
		typedef typename basic_string<T, Allocator>::view_type view_type;
		return static_cast<view_type>(a) <=> v;
	}

#else

	template <typename T, typename Allocator>
	inline bool operator==(const typename basic_string<T, Allocator>::view_type v, const basic_string<T, Allocator>& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T, typename Allocator>
	inline bool operator==(const basic_string<T, Allocator>& a, const typename basic_string<T, Allocator>::view_type v)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline bool operator!=(const basic_string<T, Allocator>& a, const basic_string<T, Allocator>& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T, typename Allocator>
	inline bool operator!=(const typename basic_string<T, Allocator>::value_type* p, const basic_string<T, Allocator>& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline bool operator!=(const basic_string<T, Allocator>& a, const typename basic_string<T, Allocator>::value_type* p)
	{
    __builtin_trap() /* STUB: not implemented */;
}
	
	
	template <typename T, typename Allocator>
	inline bool operator!=(const typename basic_string<T, Allocator>::view_type v, const basic_string<T, Allocator>& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline bool operator!=(const basic_string<T, Allocator>& a, const typename basic_string<T, Allocator>::view_type v)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	// Operator< (and also >, <=, and >=).
	template <typename T, typename Allocator>
	inline bool operator<(const basic_string<T, Allocator>& a, const basic_string<T, Allocator>& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline bool operator<(const typename basic_string<T, Allocator>::value_type* p, const basic_string<T, Allocator>& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline bool operator<(const basic_string<T, Allocator>& a, const typename basic_string<T, Allocator>::value_type* p)
	{
    __builtin_trap() /* STUB: not implemented */;
}
	
	
	template <typename T, typename Allocator>
	inline bool operator<(const typename basic_string<T, Allocator>::view_type v, const basic_string<T, Allocator>& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline bool operator<(const basic_string<T, Allocator>& a, const typename basic_string<T, Allocator>::view_type v)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline bool operator>(const basic_string<T, Allocator>& a, const basic_string<T, Allocator>& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline bool operator>(const typename basic_string<T, Allocator>::value_type* p, const basic_string<T, Allocator>& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline bool operator>(const basic_string<T, Allocator>& a, const typename basic_string<T, Allocator>::value_type* p)
	{
    __builtin_trap() /* STUB: not implemented */;
}
	
	
	template <typename T, typename Allocator>
	inline bool operator>(const typename basic_string<T, Allocator>::view_type v, const basic_string<T, Allocator>& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline bool operator>(const basic_string<T, Allocator>& a, const typename basic_string<T, Allocator>::view_type v)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline bool operator<=(const basic_string<T, Allocator>& a, const basic_string<T, Allocator>& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline bool operator<=(const typename basic_string<T, Allocator>::value_type* p, const basic_string<T, Allocator>& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline bool operator<=(const basic_string<T, Allocator>& a, const typename basic_string<T, Allocator>::value_type* p)
	{
    __builtin_trap() /* STUB: not implemented */;
}
	
	
	template <typename T, typename Allocator>
	inline bool operator<=(const typename basic_string<T, Allocator>::view_type v, const basic_string<T, Allocator>& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline bool operator<=(const basic_string<T, Allocator>& a, const typename basic_string<T, Allocator>::view_type v)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline bool operator>=(const basic_string<T, Allocator>& a, const basic_string<T, Allocator>& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline bool operator>=(const typename basic_string<T, Allocator>::value_type* p, const basic_string<T, Allocator>& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline bool operator>=(const basic_string<T, Allocator>& a, const typename basic_string<T, Allocator>::value_type* p)
	{
    __builtin_trap() /* STUB: not implemented */;
}
	
	
	template <typename T, typename Allocator>
	inline bool operator>=(const typename basic_string<T, Allocator>::view_type v, const basic_string<T, Allocator>& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, typename Allocator>
	inline bool operator>=(const basic_string<T, Allocator>& a, const typename basic_string<T, Allocator>::view_type v)
	{
    __builtin_trap() /* STUB: not implemented */;
}
#endif

	template <typename T, typename Allocator>
	inline void swap(basic_string<T, Allocator>& a, basic_string<T, Allocator>& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	/// string / wstring
	typedef basic_string<char>    string;
	typedef basic_string<wchar_t> wstring;

	/// custom string8 / string16 / string32
	typedef basic_string<char>     string8;
	typedef basic_string<char16_t> string16;
	typedef basic_string<char32_t> string32;

	/// ISO mandated string types
	typedef basic_string<char8_t>  u8string;    // Actually not a C++11 type, but added for consistency.
	typedef basic_string<char16_t> u16string;
	typedef basic_string<char32_t> u32string;


	/// hash<string>
	///
	/// We provide EASTL hash function objects for use in hash table containers.
	///
	/// Example usage:
	///    #include <EASTL/hash_set.h>
	///    hash_set<string> stringHashSet;
	///
	template <typename T> struct hash;

	template <typename Allocator>
	struct hash<basic_string<char, Allocator>>
	{
		size_t operator()(const basic_string<char, Allocator>& x) const
		{
    __builtin_trap() /* STUB: not implemented */;
}
	};

	#if defined(EA_CHAR8_UNIQUE) && EA_CHAR8_UNIQUE
		template <typename Allocator>
		struct hash<basic_string<char8_t, Allocator>>
		{
			size_t operator()(const basic_string<char8_t, Allocator>& x) const
			{
				const char8_t* p = (const char8_t*)x.c_str();
				unsigned int c, result = 2166136261U;
				while((c = *p++) != 0)
					result = (result * 16777619) ^ c;
				return (size_t)result;
			}
		};
	#endif

	template <typename Allocator>
	struct hash<basic_string<char16_t, Allocator>>
	{
		size_t operator()(const basic_string<char16_t, Allocator>& x) const
		{
    __builtin_trap() /* STUB: not implemented */;
}
	};

	template <typename Allocator>
	struct hash<basic_string<char32_t, Allocator>>
	{
		size_t operator()(const basic_string<char32_t, Allocator>& x) const
		{
    __builtin_trap() /* STUB: not implemented */;
}
	};

	#if defined(EA_WCHAR_UNIQUE) && EA_WCHAR_UNIQUE
		template <typename Allocator>
		struct hash<basic_string<wchar_t, Allocator>>
		{
			size_t operator()(const basic_string<wchar_t, Allocator>& x) const
			{
    __builtin_trap() /* STUB: not implemented */;
}
		};
	#endif

	namespace internal {
	template<typename T>
	struct transparent_string_hash {
		size_t operator()(T* s) const {
    __builtin_trap() /* STUB: not implemented */;
}

		size_t operator()(const T* s) const {
    __builtin_trap() /* STUB: not implemented */;
}

		template <typename Allocator>
		size_t operator()(const basic_string<T, Allocator>& s) const {
    __builtin_trap() /* STUB: not implemented */;
}

		size_t operator()(const basic_string_view<T>& s) const {
    __builtin_trap() /* STUB: not implemented */;
}
	};
	} // namespace internal

	// extension to the standard.
	// transparent hash objects for string types.
	struct transparent_string_hash
		: public internal::transparent_string_hash<char>
#if EA_CHAR8_UNIQUE
		, public internal::transparent_string_hash<char8_t>
#endif
	{
		using is_transparent = int;
		using internal::transparent_string_hash<char>::operator();
#if EA_CHAR8_UNIQUE
		using internal::transparent_string_hash<char8_t>::operator();
#endif
	};

	struct transparent_string16_hash
		: public internal::transparent_string_hash<char16_t>
#if EA_WCHAR_UNIQUE == 1 && EA_WCHAR_SIZE == 2
		, public internal::transparent_string_hash<wchar_t>
#endif
	{
		using is_transparent = int;
		using internal::transparent_string_hash<char16_t>::operator();
#if EA_WCHAR_UNIQUE == 1 && EA_WCHAR_SIZE == 2
		using internal::transparent_string_hash<wchar_t>::operator();
#endif
	};

	struct transparent_string32_hash
		: public internal::transparent_string_hash<char32_t>
#if EA_WCHAR_UNIQUE == 1 && EA_WCHAR_SIZE == 4
		, public internal::transparent_string_hash<wchar_t>
#endif
	{
		using is_transparent = int;
		using internal::transparent_string_hash<char32_t>::operator();
#if EA_WCHAR_UNIQUE == 1 && EA_WCHAR_SIZE == 4
		using internal::transparent_string_hash<wchar_t>::operator();
#endif
	};

	struct transparent_wstring_hash
		: public internal::transparent_string_hash<wchar_t>
#if EA_WCHAR_UNIQUE == 1
#if EA_WCHAR_SIZE == 2
		, public internal::transparent_string_hash<char16_t>
#elif EA_WCHAR_SIZE == 4
		, public internal::transparent_string_hash<char32_t>
#endif
#endif
	{
		using is_transparent = int;
		using internal::transparent_string_hash<wchar_t>::operator();
#if EA_WCHAR_UNIQUE == 1
#if EA_WCHAR_SIZE == 2
		using internal::transparent_string_hash<char16_t>::operator();
#elif EA_WCHAR_SIZE == 4
		using internal::transparent_string_hash<char32_t>::operator();
#endif
#endif
	};

	/// to_string
	///
	/// Converts integral types to an eastl::string with the same content that sprintf produces.  The following
	/// implementation provides a type safe conversion mechanism which avoids the common bugs associated with sprintf
	/// style format strings.
	///
	/// http://en.cppreference.com/w/cpp/string/basic_string/to_string
	///
	inline string to_string(int value)
		{
    __builtin_trap() /* STUB: not implemented */;
}
	inline string to_string(long value)
		{
    __builtin_trap() /* STUB: not implemented */;
}
	inline string to_string(long long value)
		{
    __builtin_trap() /* STUB: not implemented */;
}
	inline string to_string(unsigned value)
		{
    __builtin_trap() /* STUB: not implemented */;
}
	inline string to_string(unsigned long value)
		{
    __builtin_trap() /* STUB: not implemented */;
}
	inline string to_string(unsigned long long value)
		{
    __builtin_trap() /* STUB: not implemented */;
}
	inline string to_string(float value)
		{
    __builtin_trap() /* STUB: not implemented */;
}
	inline string to_string(double value)
		{
    __builtin_trap() /* STUB: not implemented */;
}
	inline string to_string(long double value)
		{
    __builtin_trap() /* STUB: not implemented */;
}


	/// to_wstring
	///
	/// Converts integral types to an eastl::wstring with the same content that sprintf produces.  The following
	/// implementation provides a type safe conversion mechanism which avoids the common bugs associated with sprintf
	/// style format strings.
	///
	/// http://en.cppreference.com/w/cpp/string/basic_string/to_wstring
	///
	inline wstring to_wstring(int value)
		{
    __builtin_trap() /* STUB: not implemented */;
}
	inline wstring to_wstring(long value)
		{
    __builtin_trap() /* STUB: not implemented */;
}
	inline wstring to_wstring(long long value)
		{
    __builtin_trap() /* STUB: not implemented */;
}
	inline wstring to_wstring(unsigned value)
		{
    __builtin_trap() /* STUB: not implemented */;
}
	inline wstring to_wstring(unsigned long value)
		{
    __builtin_trap() /* STUB: not implemented */;
}
	inline wstring to_wstring(unsigned long long value)
		{
    __builtin_trap() /* STUB: not implemented */;
}
	inline wstring to_wstring(float value)
		{
    __builtin_trap() /* STUB: not implemented */;
}
	inline wstring to_wstring(double value)
		{
    __builtin_trap() /* STUB: not implemented */;
}
	inline wstring to_wstring(long double value)
		{
    __builtin_trap() /* STUB: not implemented */;
}


	/// user defined literals
	///
	/// Converts a character array literal to a basic_string.
	///
	/// Example:
	///   string s = "abcdef"s;
	///
	/// http://en.cppreference.com/w/cpp/string/basic_string/operator%22%22s
	///
	#if EASTL_USER_LITERALS_ENABLED && EASTL_INLINE_NAMESPACES_ENABLED
		// Disabling the Clang/GCC/MSVC warning about using user
		// defined literals without a leading '_' as they are reserved
		// for standard libary usage.
		EA_DISABLE_VC_WARNING(4455)
		EA_DISABLE_CLANG_WARNING(-Wuser-defined-literals)
		EA_DISABLE_GCC_WARNING(-Wliteral-suffix)
	    inline namespace literals
	    {
		    inline namespace string_literals
		    {
				inline string operator"" s(const char* str, size_t len) EA_NOEXCEPT {
    __builtin_trap() /* STUB: not implemented */;
}
				inline u16string operator"" s(const char16_t* str, size_t len) EA_NOEXCEPT {
    __builtin_trap() /* STUB: not implemented */;
}
				inline u32string operator"" s(const char32_t* str, size_t len) EA_NOEXCEPT {
    __builtin_trap() /* STUB: not implemented */;
}
				inline wstring operator"" s(const wchar_t* str, size_t len) EA_NOEXCEPT {
    __builtin_trap() /* STUB: not implemented */;
}

				// C++20 char8_t support.
				#if EA_CHAR8_UNIQUE
					inline u8string operator"" s(const char8_t* str, size_t len) EA_NOEXCEPT { return {str, u8string::size_type(len)}; }
				#endif
		    }
	    }
		EA_RESTORE_GCC_WARNING()	// -Wliteral-suffix
		EA_RESTORE_CLANG_WARNING()	// -Wuser-defined-literals
		EA_RESTORE_VC_WARNING()		// warning: 4455
	#endif


	/// erase / erase_if
	///
	/// https://en.cppreference.com/w/cpp/string/basic_string/erase2
	template <class CharT, class Allocator, class U>
	typename basic_string<CharT, Allocator>::size_type erase(basic_string<CharT, Allocator>& c, const U& value)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <class CharT, class Allocator, class Predicate>
	typename basic_string<CharT, Allocator>::size_type erase_if(basic_string<CharT, Allocator>& c, Predicate predicate)
	{
    __builtin_trap() /* STUB: not implemented */;
}
} // namespace eastl


EA_RESTORE_VC_WARNING();


#endif // Header include guard
