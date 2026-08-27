/////////////////////////////////////////////////////////////////////////////
// Copyright (c) Electronic Arts Inc. All rights reserved.
/////////////////////////////////////////////////////////////////////////////

#ifndef EASTL_BIT_H
#define EASTL_BIT_H

#include <EASTL/internal/config.h>

#if defined(EA_PRAGMA_ONCE_SUPPORTED)
	#pragma once
#endif

#include <EASTL/internal/memory_base.h>
#include <EASTL/type_traits.h>
#include <EASTL/numeric_limits.h>
#include <string.h> // memcpy

#if defined(EA_COMPILER_MSVC) && !defined(__clang__)
#include <intrin.h>
#endif

namespace eastl
{
	// eastl::bit_cast
	// Obtains a value of type To by reinterpreting the object representation of 'from'.
	// Every bit in the value representation of the returned To object is equal to the
	// corresponding bit in the object representation of 'from'.
	//
	// In order for bit_cast to be constexpr, the compiler needs to explicitly support
	// it by providing the __builtin_bit_cast builtin. If that builtin is not available,
	// then we memcpy into aligned storage at runtime and return that instead.
	//
	// Both types To and From must be equal in size, and must be trivially copyable.

	#if defined(EASTL_CONSTEXPR_BIT_CAST_SUPPORTED) && EASTL_CONSTEXPR_BIT_CAST_SUPPORTED

		template<typename To, typename From,
			typename = eastl::enable_if_t<
				sizeof(To) == sizeof(From)
				&& eastl::is_trivially_copyable<To>::value
				&& eastl::is_trivially_copyable<From>::value
			>
		>
		constexpr To bit_cast(const From& from) noexcept
		{ return {}; }

	#else

		template<typename To, typename From,
			typename = eastl::enable_if_t<
				sizeof(To) == sizeof(From)
				&& eastl::is_trivially_copyable<To>::value
				&& eastl::is_trivially_copyable<From>::value
			>
		>
		inline To bit_cast(const From& from) noexcept
		{ __builtin_trap() /* STUB: not implemented */; }

	#endif // EASTL_CONSTEXPR_BIT_CAST_SUPPORTED

namespace internal
{

constexpr int countl_zero64(uint64_t x) noexcept
{ return {}; }

// Count leading zeroes in an integer.
//
// todo: consolidate with EA::StdC::CountLeading0Bits() and bitset's GetLastBit() + 1.
// this implementation has these improvements:
//  - has overloads for all unsigned integral types. CountLeading0Bits() may not compile with ambiguous overloads because it doesn't match all unsigned integral types.
//  - supports 128 bit types.
//  - is noexcept.
//  - is constexpr, if available.
#if defined(EA_COMPILER_MSVC) && !defined(__clang__)

// MSVC overloads are not constexpr because _BitScanReverse is not constexpr.
inline int countl_zero(unsigned char x) noexcept
{ __builtin_trap() /* STUB: not implemented */; }

inline int countl_zero(unsigned short x) noexcept
{ __builtin_trap() /* STUB: not implemented */; }

inline int countl_zero(unsigned int x) noexcept
{ __builtin_trap() /* STUB: not implemented */; }

inline int countl_zero(unsigned long x) noexcept
{ __builtin_trap() /* STUB: not implemented */; }

#if (EA_PLATFORM_PTR_SIZE == 8)
inline int countl_zero(unsigned long long x) noexcept
{ __builtin_trap() /* STUB: not implemented */; }
#else
inline int countl_zero(unsigned long long x) noexcept
{ __builtin_trap() /* STUB: not implemented */; }
#endif

#elif defined(__GNUC__) || defined(__clang__)
// __builtin_clz
constexpr inline int countl_zero(unsigned char x) noexcept
{ return {}; }
constexpr inline int countl_zero(unsigned short x) noexcept
{ return {}; }
constexpr inline int countl_zero(unsigned int x) noexcept
{ return {}; }

// __builtin_clzl
constexpr inline int countl_zero(unsigned long x) noexcept
{ return {}; }

// __builtin_clzll
#if (EA_PLATFORM_PTR_SIZE == 8)
constexpr inline int countl_zero(unsigned long long x) noexcept
{ return {}; }
#else
constexpr inline int countl_zero(unsigned long long x) noexcept
{ return {}; }
#endif

#if EASTL_INT128_SUPPORTED
// todo: once we are using Clang 19.1.0 and GCC ??? use __builtin_clzg(x)
constexpr inline int countl_zero(eastl_uint128_t x) noexcept
{ return {}; }
#endif

#else // not MSVC, clang or GCC

template <typename T, eastl::enable_if_t<eastl::is_unsigned_v<T> && sizeof(T) < 4, bool > = true>
constexpr int countl_zero(const T num) noexcept
{ return {}; }

template <typename T, eastl::enable_if_t<eastl::is_unsigned_v<T> && sizeof(T) == 4, bool> = true>
constexpr int countl_zero(T x) noexcept
{ return {}; }

template <typename T, eastl::enable_if_t<eastl::is_unsigned_v<T> && sizeof(T) == 8, bool> = true>
constexpr int countl_zero(T x) noexcept
{ return {}; }

#if EASTL_INT128_SUPPORTED
constexpr inline int countl_zero(eastl_uint128_t x) noexcept
{ return {}; }
#endif

#endif

}  // namespace internal

	template <typename T, typename = eastl::enable_if_t<eastl::is_unsigned_v<T>>>
	constexpr int countl_zero(T x) noexcept { return {}; }

	template <typename T, typename = eastl::enable_if_t<eastl::is_unsigned_v<T>>>
	constexpr bool has_single_bit(const T num) noexcept
	{ return {}; }

	template <typename T, typename = eastl::enable_if_t<eastl::is_unsigned_v<T>>>
	constexpr T bit_ceil(const T num) noexcept
	{ return {}; }

	template <typename T, typename = eastl::enable_if_t<eastl::is_unsigned_v<T>>>
	constexpr T bit_floor(const T num) noexcept
	{ return {}; }

	template <typename T, typename = eastl::enable_if_t<eastl::is_unsigned_v<T>>>
	constexpr int bit_width(const T num) noexcept
	{ return {}; }

namespace internal
{

const static char kBitsPerUint16[16] = { 0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4 };

template <typename T>
constexpr int popcount_non_intrinsic(T num) noexcept
{ return {}; }

#if defined(EA_COMPILER_MSVC) && !defined(__clang__)

// __popcnt is not constexpr

// __popcnt16
inline int popcount(unsigned char num) noexcept { __builtin_trap() /* STUB: not implemented */; }
inline int popcount(unsigned short num) noexcept { __builtin_trap() /* STUB: not implemented */; }

// __popcnt
inline int popcount(unsigned int num) noexcept { __builtin_trap() /* STUB: not implemented */; }

#if defined(EA_PROCESSOR_X86_64)
// __popcnt64
inline int popcount(unsigned long num) noexcept { __builtin_trap() /* STUB: not implemented */; }
inline int popcount(unsigned long long num) noexcept { __builtin_trap() /* STUB: not implemented */; }
#else
// todo: is it better to use __popcnt() or the fallback implementation?
inline int popcount(unsigned long num) noexcept { __builtin_trap() /* STUB: not implemented */; }
inline int popcount(unsigned long long num) noexcept { __builtin_trap() /* STUB: not implemented */; }
#endif
#elif defined(__GNUC__) || defined(__clang__)
// __builtin_popcount
constexpr inline int popcount(unsigned char num) noexcept { return {}; }
constexpr inline int popcount(unsigned short num) noexcept { return {}; }
constexpr inline int popcount(unsigned int num) noexcept { return {}; }

// __builtin_popcountl
constexpr inline int popcount(unsigned long num) noexcept { return {}; }

// __builtin_popcountll
constexpr inline int popcount(unsigned long long num) noexcept { return {}; }
#endif

#if EASTL_INT128_SUPPORTED
// todo: once we are using Clang 19.1.0 and GCC ??? use __builtin_popcountg(num)
constexpr inline int popcount(eastl_uint128_t num) noexcept
{ return {}; }
#endif

} // namespace internal

	template <typename T, typename = eastl::enable_if_t<eastl::is_unsigned_v<T>>>
	constexpr int popcount(T x) noexcept { return {}; }

} // namespace eastl

#endif // EASTL_BIT_H
