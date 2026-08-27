/////////////////////////////////////////////////////////////////////////////
// Copyright (c) Electronic Arts Inc. All rights reserved.
/////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// This file implements similar functionality to char_traits which is part of
// the C++ standard STL library specification. This is intended for internal
// EASTL use only.  Functionality can be accessed through the eastl::string or
// eastl::string_view types.  
//
// http://en.cppreference.com/w/cpp/string/char_traits
///////////////////////////////////////////////////////////////////////////////

#ifndef EASTL_CHAR_TRAITS_H
#define EASTL_CHAR_TRAITS_H

#if defined(EA_PRAGMA_ONCE_SUPPORTED)
	#pragma once
#endif

#include <EASTL/internal/config.h>
#include <EASTL/type_traits.h>
#include <EASTL/algorithm.h>

EA_DISABLE_ALL_VC_WARNINGS()
#include <ctype.h>              // toupper, etc.
#include <string.h>             // memset, etc.
EA_RESTORE_ALL_VC_WARNINGS()

namespace eastl
{
	namespace details
	{
#if defined(EA_COMPILER_CPP17_ENABLED)
		// Helper to detect if wchar_t is the native type for the current platform or if -fshort-wchar was used.
		// When that flag is used all string builtins and C Standard Library functions are not usable.
		constexpr bool UseNativeWideChar()
		{ return {}; }
#endif
	}
	
	///////////////////////////////////////////////////////////////////////////////
	/// DecodePart
	///
	/// These implement UTF8/UCS2/UCS4 encoding/decoding.
	///
	EASTL_API bool DecodePart(const char*& pSrc, const char* pSrcEnd, char*&     pDest, char*     pDestEnd);
	EASTL_API bool DecodePart(const char*& pSrc, const char* pSrcEnd, char16_t*& pDest, char16_t* pDestEnd);
	EASTL_API bool DecodePart(const char*& pSrc, const char* pSrcEnd, char32_t*& pDest, char32_t* pDestEnd);

	EASTL_API bool DecodePart(const char16_t*& pSrc, const char16_t* pSrcEnd, char*&     pDest, char*     pDestEnd);
	EASTL_API bool DecodePart(const char16_t*& pSrc, const char16_t* pSrcEnd, char16_t*& pDest, char16_t* pDestEnd);
	EASTL_API bool DecodePart(const char16_t*& pSrc, const char16_t* pSrcEnd, char32_t*& pDest, char32_t* pDestEnd);

	EASTL_API bool DecodePart(const char32_t*& pSrc, const char32_t* pSrcEnd, char*&     pDest, char*     pDestEnd);
	EASTL_API bool DecodePart(const char32_t*& pSrc, const char32_t* pSrcEnd, char16_t*& pDest, char16_t* pDestEnd);
	EASTL_API bool DecodePart(const char32_t*& pSrc, const char32_t* pSrcEnd, char32_t*& pDest, char32_t* pDestEnd);

	EASTL_API bool DecodePart(const int*& pSrc, const int* pSrcEnd, char*&     pDest, char*     pDestEnd);
	EASTL_API bool DecodePart(const int*& pSrc, const int* pSrcEnd, char16_t*& pDest, char16_t* pDestEnd);
	EASTL_API bool DecodePart(const int*& pSrc, const int* pSrcEnd, char32_t*& pDest, char32_t* pDestEnd);

	#if EA_CHAR8_UNIQUE
		bool DecodePart(const char8_t*& pSrc, const char8_t* pSrcEnd, char8_t*&  pDest, char8_t*  pDestEnd);

		bool DecodePart(const char8_t*& pSrc, const char8_t* pSrcEnd, char*&     pDest, char*     pDestEnd);
		bool DecodePart(const char8_t*& pSrc, const char8_t* pSrcEnd, char16_t*& pDest, char16_t* pDestEnd);
		bool DecodePart(const char8_t*& pSrc, const char8_t* pSrcEnd, char32_t*& pDest, char32_t* pDestEnd);

		bool DecodePart(const char*&     pSrc, const char*     pSrcEnd, char8_t*& pDest, char8_t* pDestEnd);
		bool DecodePart(const char16_t*& pSrc, const char16_t* pSrcEnd, char8_t*& pDest, char8_t* pDestEnd);
		bool DecodePart(const char32_t*& pSrc, const char32_t* pSrcEnd, char8_t*& pDest, char8_t* pDestEnd);
	#endif

	#if EA_WCHAR_UNIQUE
		bool DecodePart(const wchar_t*& pSrc, const wchar_t* pSrcEnd, wchar_t*&  pDest, wchar_t*  pDestEnd);

		bool DecodePart(const wchar_t*& pSrc, const wchar_t* pSrcEnd, char*&     pDest, char*     pDestEnd);
		bool DecodePart(const wchar_t*& pSrc, const wchar_t* pSrcEnd, char16_t*& pDest, char16_t* pDestEnd);
		bool DecodePart(const wchar_t*& pSrc, const wchar_t* pSrcEnd, char32_t*& pDest, char32_t* pDestEnd);

		bool DecodePart(const char*&     pSrc, const char*     pSrcEnd, wchar_t*& pDest, wchar_t* pDestEnd);
		bool DecodePart(const char16_t*& pSrc, const char16_t* pSrcEnd, wchar_t*& pDest, wchar_t* pDestEnd);
		bool DecodePart(const char32_t*& pSrc, const char32_t* pSrcEnd, wchar_t*& pDest, wchar_t* pDestEnd);
	#endif

	#if EA_CHAR8_UNIQUE && EA_WCHAR_UNIQUE
		bool DecodePart(const char8_t*& pSrc, const char8_t* pSrcEnd, wchar_t*& pDest, wchar_t* pDestEnd);
		bool DecodePart(const wchar_t*& pSrc, const wchar_t* pSrcEnd, char8_t*& pDest, char8_t* pDestEnd);
	#endif


	#if EA_WCHAR_UNIQUE
		inline bool DecodePart(const wchar_t*& pSrc, const wchar_t* pSrcEnd, wchar_t*& pDest, wchar_t* pDestEnd)
		{ __builtin_trap() /* STUB: not implemented */; }

		inline bool DecodePart(const wchar_t*& pSrc, const wchar_t* pSrcEnd, char*& pDest, char* pDestEnd)
		{ __builtin_trap() /* STUB: not implemented */; }

		inline bool DecodePart(const wchar_t*& pSrc, const wchar_t* pSrcEnd, char16_t*& pDest, char16_t* pDestEnd)
		{ __builtin_trap() /* STUB: not implemented */; }

		inline bool DecodePart(const wchar_t*& pSrc, const wchar_t* pSrcEnd, char32_t*& pDest, char32_t* pDestEnd)
		{ __builtin_trap() /* STUB: not implemented */; }

		inline bool DecodePart(const char*& pSrc, const char* pSrcEnd, wchar_t*& pDest, wchar_t* pDestEnd)
		{ __builtin_trap() /* STUB: not implemented */; }

		inline bool DecodePart(const char16_t*& pSrc, const char16_t* pSrcEnd, wchar_t*& pDest, wchar_t* pDestEnd)
		{ __builtin_trap() /* STUB: not implemented */; }

		inline bool DecodePart(const char32_t*& pSrc, const char32_t* pSrcEnd, wchar_t*& pDest, wchar_t* pDestEnd)
		{ __builtin_trap() /* STUB: not implemented */; }
	#endif

	#if EA_CHAR8_UNIQUE
	    inline bool DecodePart(const char8_t*& pSrc, const char8_t* pSrcEnd, char8_t*& pDest, char8_t* pDestEnd)
	    { __builtin_trap() /* STUB: not implemented */; }

	    inline bool DecodePart(const char8_t*& pSrc, const char8_t* pSrcEnd, char*& pDest, char* pDestEnd)
	    { __builtin_trap() /* STUB: not implemented */; }

	    inline bool DecodePart(const char8_t*& pSrc, const char8_t* pSrcEnd, char16_t*& pDest, char16_t* pDestEnd)
	    { __builtin_trap() /* STUB: not implemented */; }

	    inline bool DecodePart(const char8_t*& pSrc, const char8_t* pSrcEnd, char32_t*& pDest, char32_t* pDestEnd)
	    { __builtin_trap() /* STUB: not implemented */; }

		inline bool DecodePart(const char*& pSrc, const char* pSrcEnd, char8_t*& pDest, char8_t* pDestEnd)
		{ __builtin_trap() /* STUB: not implemented */; }

		inline bool DecodePart(const char16_t*& pSrc, const char16_t* pSrcEnd, char8_t*& pDest, char8_t* pDestEnd)
		{ __builtin_trap() /* STUB: not implemented */; }

		inline bool DecodePart(const char32_t*& pSrc, const char32_t* pSrcEnd, char8_t*& pDest, char8_t* pDestEnd)
		{ __builtin_trap() /* STUB: not implemented */; }
    #endif

	#if EA_CHAR8_UNIQUE && EA_WCHAR_UNIQUE
		inline bool DecodePart(const char8_t*&  pSrc, const char8_t*  pSrcEnd, wchar_t*&  pDest, wchar_t*  pDestEnd)
		{ __builtin_trap() /* STUB: not implemented */; }

		inline bool DecodePart(const wchar_t*&  pSrc, const wchar_t*  pSrcEnd, char8_t*&  pDest, char8_t*  pDestEnd)
		{ __builtin_trap() /* STUB: not implemented */; }
	#endif

	///////////////////////////////////////////////////////////////////////////////
	// 'char traits' functionality
	//
	inline char CharToLower(char c)
		{ __builtin_trap() /* STUB: not implemented */; }

	template<typename T>
	inline T CharToLower(T c)
		{ __builtin_trap() /* STUB: not implemented */; }


	inline char CharToUpper(char c)
		{ __builtin_trap() /* STUB: not implemented */; }

	template<typename T>
	inline T CharToUpper(T c)
		{ __builtin_trap() /* STUB: not implemented */; }


	template <typename T>
	int Compare(const T* p1, const T* p2, size_t n)
	{ __builtin_trap() /* STUB: not implemented */; }

#if defined(EA_COMPILER_CPP17_ENABLED)
	// All main compilers offer a constexpr __builtin_memcmp as soon as C++17 was available.
	constexpr int Compare(const char* p1, const char* p2, size_t n) { return {}; }

#if !defined(EA_COMPILER_GNUC)
	// GCC doesn't offer __builtin_wmemcmp.
	constexpr int Compare(const wchar_t* p1, const wchar_t* p2, size_t n)
	{ return {}; }
#endif // !defined(EA_COMPILER_GNUC)
#else
	inline int Compare(const char* p1, const char* p2, size_t n)
	{ __builtin_trap() /* STUB: not implemented */; }
#endif

	template <typename T>
	inline int CompareI(const T* p1, const T* p2, size_t n)
	{ __builtin_trap() /* STUB: not implemented */; }


	template<typename T>
	inline EA_CPP14_CONSTEXPR const T* Find(const T* p, T c, size_t n)
	{ __builtin_trap() /* STUB: not implemented */; }

#if defined(EA_COMPILER_CPP17_ENABLED) && defined(EA_COMPILER_CLANG)
	// Only clang have __builtin_char_memchr.
	// __builtin_memchr doesn't work in a constexpr context since we need to cast the returned void* to a char*.
	inline constexpr const char* Find(const char* p, char c, size_t n)
	{ return {}; }
#else
	inline const char* Find(const char* p, char c, size_t n)
	{ __builtin_trap() /* STUB: not implemented */; }
#endif

	template <typename T>
	inline EA_CPP14_CONSTEXPR size_t CharStrlen(const T* p)
	{ __builtin_trap() /* STUB: not implemented */; }

#if defined(EA_COMPILER_CPP17_ENABLED) && !defined(EA_COMPILER_GNUC)
	// So far, GCC seems to struggle with builtin_strlen: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=70816
	// MSVC and Clang support both builtins as soon as C++17 was available.
	constexpr size_t CharStrlen(const char* p) { return {}; }

	constexpr size_t CharStrlen(const wchar_t* p)
	{ return {}; }
#endif

	// If either pDestination or pSource is an invalid or null pointer, the behavior is undefined, even if (pSourceEnd - pSource) is zero.
	template <typename T>
	inline T* CharStringUninitializedCopy(const T* pSource, const T* pSourceEnd, T* pDestination)
	{ __builtin_trap() /* STUB: not implemented */; }


	// CharTypeStringFindEnd
	// Specialized char version of STL find() from back function.
	// Not the same as RFind because search range is specified as forward iterators.
	template <typename T>
	const T* CharTypeStringFindEnd(const T* pBegin, const T* pEnd, T c)
	{ __builtin_trap() /* STUB: not implemented */; }


	// CharTypeStringSearch
	// Specialized value_type version of STL search() function.
	// Purpose: find p2 within p1. Return p1End if not found or if either string is zero length.
	template <typename T>
	const T* CharTypeStringSearch(const T* p1Begin, const T* p1End,
								  const T* p2Begin, const T* p2End)
	{ __builtin_trap() /* STUB: not implemented */; }


	// CharTypeStringRSearch
	// Specialized value_type version of STL find_end() function (which really is a reverse search function).
	// Purpose: find last instance of p2 within p1. Return p1End if not found or if either string is zero length.
	template <typename T>
	const T* CharTypeStringRSearch(const T* p1Begin, const T* p1End, 
								   const T* p2Begin, const T* p2End)
	{ __builtin_trap() /* STUB: not implemented */; }


	// CharTypeStringFindFirstOf
	// Specialized value_type version of STL find_first_of() function.
	// This function is much like the C runtime strtok function, except the strings aren't null-terminated.
	template <typename T>
	inline const T* CharTypeStringFindFirstOf(const T* p1Begin, const T* p1End, const T* p2Begin, const T* p2End)
	{ __builtin_trap() /* STUB: not implemented */; }


	// CharTypeStringRFindFirstNotOf
	// Specialized value_type version of STL find_first_not_of() function in reverse.
	template <typename T>
	inline const T* CharTypeStringRFindFirstNotOf(const T* p1RBegin, const T* p1REnd, const T* p2Begin, const T* p2End)
	{ __builtin_trap() /* STUB: not implemented */; }


	// CharTypeStringFindFirstNotOf
	// Specialized value_type version of STL find_first_not_of() function.
	template <typename T>
	inline const T* CharTypeStringFindFirstNotOf(const T* p1Begin, const T* p1End, const T* p2Begin, const T* p2End)
	{ __builtin_trap() /* STUB: not implemented */; }


	// CharTypeStringRFindFirstOf
	// Specialized value_type version of STL find_first_of() function in reverse.
	// This function is much like the C runtime strtok function, except the strings aren't null-terminated.
	template <typename T>
	inline const T* CharTypeStringRFindFirstOf(const T* p1RBegin, const T* p1REnd, const T* p2Begin, const T* p2End)
	{ __builtin_trap() /* STUB: not implemented */; }


	// CharTypeStringRFind
	// Specialized value_type version of STL find() function in reverse.
	template <typename T>
	inline const T* CharTypeStringRFind(const T* pRBegin, const T* pREnd, const T c)
	{ __builtin_trap() /* STUB: not implemented */; }


	inline char* CharStringUninitializedFillN(char* pDestination, size_t n, const char c)
	{ __builtin_trap() /* STUB: not implemented */; }

	template<typename T>
	inline T* CharStringUninitializedFillN(T* pDestination, size_t n, const T c)
	{ __builtin_trap() /* STUB: not implemented */; }


	inline char* CharTypeAssignN(char* pDestination, size_t n, char c)
	{ __builtin_trap() /* STUB: not implemented */; }

	template<typename T>
	inline T* CharTypeAssignN(T* pDestination, size_t n, T c)
	{ __builtin_trap() /* STUB: not implemented */; }
} // namespace eastl

#endif // EASTL_CHAR_TRAITS_H
