#include <stdexcept>
#include <cstdlib>
///////////////////////////////////////////////////////////////////////////////
// Copyright (c) Electronic Arts Inc. All rights reserved.
///////////////////////////////////////////////////////////////////////////////


#include <EASTL/internal/config.h>
#include <EASTL/string.h>
#include <EABase/eabase.h>
#include <string.h>


namespace eastl
{
	///////////////////////////////////////////////////////////////////////////////
	// Converters for DecodePart
	//
	// For some decent documentation about conversions, see:
	//     http://tidy.sourceforge.net/cgi-bin/lxr/source/src/utf8.c
	//     
	///////////////////////////////////////////////////////////////////////////////

	// Requires that pDest have a capacity of at least 6 chars.
	// Sets pResult to '\1' in the case that c is an invalid UCS4 char.
	inline bool UCS4ToUTF8(uint32_t c, char*& pResult)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	// Requires that pResult have a capacity of at least 3 chars.
	// Sets pResult to '\1' in the case that c is an invalid UCS4 char.
	inline bool UCS2ToUTF8(uint16_t c, char*& pResult)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	// Sets result to 0xffff in the case that the input UTF8 sequence is bad.
	// 32 bit 0xffffffff is an invalid UCS4 code point, so we can't use that as an error return value.
	inline bool UTF8ToUCS4(const char*& p, const char* pEnd, uint32_t& result)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	// Sets result to 0xffff in the case that the input UTF8 sequence is bad.
	// The effect of converting UTF8 codepoints > 0xffff to UCS2 (char16_t) is to set all
	// such codepoints to 0xffff. EASTL doesn't have a concept of setting or maintaining 
	// error state for string conversions, though it does have a policy of converting 
	// impossible values to something without generating invalid strings or throwing exceptions.
	inline bool UTF8ToUCS2(const char*& p, const char* pEnd, uint16_t& result)
	{
    __builtin_trap() /* STUB: not implemented */;
}



	///////////////////////////////////////////////////////////////////////////
	// DecodePart
	///////////////////////////////////////////////////////////////////////////

	EASTL_API bool DecodePart(const char*& pSrc, const char* pSrcEnd, char*& pDest, char* pDestEnd)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	EASTL_API bool DecodePart(const char*& pSrc, const char* pSrcEnd, char16_t*& pDest, char16_t* pDestEnd)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	EASTL_API bool DecodePart(const char*& pSrc, const char* pSrcEnd, char32_t*& pDest, char32_t* pDestEnd)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	EASTL_API bool DecodePart(const char16_t*& pSrc, const char16_t* pSrcEnd, char*& pDest, char* pDestEnd)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	EASTL_API bool DecodePart(const char16_t*& pSrc, const char16_t* pSrcEnd, char16_t*& pDest, char16_t* pDestEnd)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	EASTL_API bool DecodePart(const char16_t*& pSrc, const char16_t* pSrcEnd, char32_t*& pDest, char32_t* pDestEnd)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	EASTL_API bool DecodePart(const char32_t*& pSrc, const char32_t* pSrcEnd, char*& pDest, char* pDestEnd)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	EASTL_API bool DecodePart(const char32_t*& pSrc, const char32_t* pSrcEnd, char16_t*& pDest, char16_t* pDestEnd)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	EASTL_API bool DecodePart(const char32_t*& pSrc, const char32_t* pSrcEnd, char32_t*& pDest, char32_t* pDestEnd)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	EASTL_API bool DecodePart(const int*& pSrc, const int* pSrcEnd, char*&  pDest, char* pDestEnd)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	EASTL_API bool DecodePart(const int*& pSrc, const int* pSrcEnd, char16_t*& pDest, char16_t* pDestEnd)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	EASTL_API bool DecodePart(const int*& pSrc, const int* pSrcEnd, char32_t*& pDest, char32_t* pDestEnd)
	{
    __builtin_trap() /* STUB: not implemented */;
}



} // namespace eastl














