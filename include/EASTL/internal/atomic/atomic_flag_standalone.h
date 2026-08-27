/////////////////////////////////////////////////////////////////////////////////
// Copyright (c) Electronic Arts Inc. All rights reserved.
/////////////////////////////////////////////////////////////////////////////////


#ifndef EASTL_ATOMIC_INTERNAL_FLAG_STANDALONE_H
#define EASTL_ATOMIC_INTERNAL_FLAG_STANDALONE_H

#if defined(EA_PRAGMA_ONCE_SUPPORTED)
	#pragma once
#endif


namespace eastl
{


////////////////////////////////////////////////////////////////////////////////
//
// bool atomic_flag_test_and_set(eastl::atomic<T>*)
//
EASTL_FORCE_INLINE bool atomic_flag_test_and_set(eastl::atomic_flag* atomicObj) EA_NOEXCEPT
{ __builtin_trap() /* STUB: not implemented */; }

template <typename Order>
EASTL_FORCE_INLINE bool atomic_flag_test_and_set_explicit(eastl::atomic_flag* atomicObj, Order order)
{ __builtin_trap() /* STUB: not implemented */; }


////////////////////////////////////////////////////////////////////////////////
//
// bool atomic_flag_clear(eastl::atomic<T>*)
//
EASTL_FORCE_INLINE void atomic_flag_clear(eastl::atomic_flag* atomicObj)
{ __builtin_trap() /* STUB: not implemented */; }

template <typename Order>
EASTL_FORCE_INLINE void atomic_flag_clear_explicit(eastl::atomic_flag* atomicObj, Order order)
{ __builtin_trap() /* STUB: not implemented */; }


////////////////////////////////////////////////////////////////////////////////
//
// bool atomic_flag_test(eastl::atomic<T>*)
//
EASTL_FORCE_INLINE bool atomic_flag_test(eastl::atomic_flag* atomicObj)
{ __builtin_trap() /* STUB: not implemented */; }

template <typename Order>
EASTL_FORCE_INLINE bool atomic_flag_test_explicit(eastl::atomic_flag* atomicObj, Order order)
{ __builtin_trap() /* STUB: not implemented */; }


} // namespace eastl


#endif /* EASTL_ATOMIC_INTERNAL_FLAG_STANDALONE_H */
