/////////////////////////////////////////////////////////////////////////////////
// Copyright (c) Electronic Arts Inc. All rights reserved.
/////////////////////////////////////////////////////////////////////////////////


#ifndef EASTL_ATOMIC_INTERNAL_STANDALONE_H
#define EASTL_ATOMIC_INTERNAL_STANDALONE_H

#if defined(EA_PRAGMA_ONCE_SUPPORTED)
	#pragma once
#endif


namespace eastl
{


////////////////////////////////////////////////////////////////////////////////
//
// bool atomic_compare_exchange_strong(eastl::atomic<T>*, T* expected, T desired)
//
template <typename T>
EASTL_FORCE_INLINE bool atomic_compare_exchange_strong(eastl::atomic<T>* atomicObj,
													   typename eastl::atomic<T>::value_type* expected,
													   typename eastl::atomic<T>::value_type desired) EA_NOEXCEPT
{ __builtin_trap() /* STUB: not implemented */; }

template <typename T, typename OrderSuccess, typename OrderFailure>
EASTL_FORCE_INLINE bool atomic_compare_exchange_strong_explicit(eastl::atomic<T>* atomicObj,
																typename eastl::atomic<T>::value_type* expected,
																typename eastl::atomic<T>::value_type desired,
																OrderSuccess orderSuccess, OrderFailure orderFailure) EA_NOEXCEPT
{ __builtin_trap() /* STUB: not implemented */; }


////////////////////////////////////////////////////////////////////////////////
//
// bool atomic_compare_exchange_weak(eastl::atomic<T>*, T* expected, T desired)
//
template <typename T>
EASTL_FORCE_INLINE bool atomic_compare_exchange_weak(eastl::atomic<T>* atomicObj,
													 typename eastl::atomic<T>::value_type* expected,
													 typename eastl::atomic<T>::value_type desired) EA_NOEXCEPT
{ __builtin_trap() /* STUB: not implemented */; }

template <typename T, typename OrderSuccess, typename OrderFailure>
EASTL_FORCE_INLINE bool atomic_compare_exchange_weak_explicit(eastl::atomic<T>* atomicObj,
															  typename eastl::atomic<T>::value_type* expected,
															  typename eastl::atomic<T>::value_type desired,
															  OrderSuccess orderSuccess, OrderFailure orderFailure) EA_NOEXCEPT
{ __builtin_trap() /* STUB: not implemented */; }


////////////////////////////////////////////////////////////////////////////////
//
// T atomic_fetch_xor(eastl::atomic<T>*, T arg)
//
template <typename T>
EASTL_FORCE_INLINE typename eastl::atomic<T>::value_type atomic_fetch_xor(eastl::atomic<T>* atomicObj,
																		  typename eastl::atomic<T>::value_type arg) EA_NOEXCEPT
{ __builtin_trap() /* STUB: not implemented */; }

template <typename T, typename Order>
EASTL_FORCE_INLINE typename eastl::atomic<T>::value_type atomic_fetch_xor_explicit(eastl::atomic<T>* atomicObj,
																				   typename eastl::atomic<T>::value_type arg,
																				   Order order) EA_NOEXCEPT
{ __builtin_trap() /* STUB: not implemented */; }


////////////////////////////////////////////////////////////////////////////////
//
// T atomic_xor_fetch(eastl::atomic<T>*, T arg)
//
template <typename T>
EASTL_FORCE_INLINE typename eastl::atomic<T>::value_type atomic_xor_fetch(eastl::atomic<T>* atomicObj,
																		  typename eastl::atomic<T>::value_type arg) EA_NOEXCEPT
{ __builtin_trap() /* STUB: not implemented */; }

template <typename T, typename Order>
EASTL_FORCE_INLINE typename eastl::atomic<T>::value_type atomic_xor_fetch_explicit(eastl::atomic<T>* atomicObj,
																				   typename eastl::atomic<T>::value_type arg,
																				   Order order) EA_NOEXCEPT
{ __builtin_trap() /* STUB: not implemented */; }


////////////////////////////////////////////////////////////////////////////////
//
// T atomic_fetch_or(eastl::atomic<T>*, T arg)
//
template <typename T>
EASTL_FORCE_INLINE typename eastl::atomic<T>::value_type atomic_fetch_or(eastl::atomic<T>* atomicObj,
																		 typename eastl::atomic<T>::value_type arg) EA_NOEXCEPT
{ __builtin_trap() /* STUB: not implemented */; }

template <typename T, typename Order>
EASTL_FORCE_INLINE typename eastl::atomic<T>::value_type atomic_fetch_or_explicit(eastl::atomic<T>* atomicObj,
																				  typename eastl::atomic<T>::value_type arg,
																				  Order order) EA_NOEXCEPT
{ __builtin_trap() /* STUB: not implemented */; }


////////////////////////////////////////////////////////////////////////////////
//
// T atomic_or_fetch(eastl::atomic<T>*, T arg)
//
template <typename T>
EASTL_FORCE_INLINE typename eastl::atomic<T>::value_type atomic_or_fetch(eastl::atomic<T>* atomicObj,
																		 typename eastl::atomic<T>::value_type arg) EA_NOEXCEPT
{ __builtin_trap() /* STUB: not implemented */; }

template <typename T, typename Order>
EASTL_FORCE_INLINE typename eastl::atomic<T>::value_type atomic_or_fetch_explicit(eastl::atomic<T>* atomicObj,
																				  typename eastl::atomic<T>::value_type arg,
																				  Order order) EA_NOEXCEPT
{ __builtin_trap() /* STUB: not implemented */; }


////////////////////////////////////////////////////////////////////////////////
//
// T atomic_fetch_and(eastl::atomic<T>*, T arg)
//
template <typename T>
EASTL_FORCE_INLINE typename eastl::atomic<T>::value_type atomic_fetch_and(eastl::atomic<T>* atomicObj,
																		  typename eastl::atomic<T>::value_type arg) EA_NOEXCEPT
{ __builtin_trap() /* STUB: not implemented */; }

template <typename T, typename Order>
EASTL_FORCE_INLINE typename eastl::atomic<T>::value_type atomic_fetch_and_explicit(eastl::atomic<T>* atomicObj,
																				   typename eastl::atomic<T>::value_type arg,
																				   Order order) EA_NOEXCEPT
{ __builtin_trap() /* STUB: not implemented */; }


////////////////////////////////////////////////////////////////////////////////
//
// T atomic_and_fetch(eastl::atomic<T>*, T arg)
//
template <typename T>
EASTL_FORCE_INLINE typename eastl::atomic<T>::value_type atomic_and_fetch(eastl::atomic<T>* atomicObj,
																		  typename eastl::atomic<T>::value_type arg) EA_NOEXCEPT
{ __builtin_trap() /* STUB: not implemented */; }

template <typename T, typename Order>
EASTL_FORCE_INLINE typename eastl::atomic<T>::value_type atomic_and_fetch_explicit(eastl::atomic<T>* atomicObj,
																				   typename eastl::atomic<T>::value_type arg,
																				   Order order) EA_NOEXCEPT
{ __builtin_trap() /* STUB: not implemented */; }


/////////////////////////////////////////////////////////////////////////////////
//
// T atomic_fetch_sub(eastl::atomic<T>*, T arg)
//
template <typename T>
EASTL_FORCE_INLINE typename eastl::atomic<T>::value_type atomic_fetch_sub(eastl::atomic<T>* atomicObj,
																		  typename eastl::atomic<T>::difference_type arg) EA_NOEXCEPT
{ __builtin_trap() /* STUB: not implemented */; }

template <typename T, typename Order>
EASTL_FORCE_INLINE typename eastl::atomic<T>::value_type atomic_fetch_sub_explicit(eastl::atomic<T>* atomicObj,
																				   typename eastl::atomic<T>::difference_type arg,
																				   Order order) EA_NOEXCEPT
{ __builtin_trap() /* STUB: not implemented */; }


/////////////////////////////////////////////////////////////////////////////////
//
// T atomic_sub_fetch(eastl::atomic<T>*, T arg)
//
template <typename T>
EASTL_FORCE_INLINE typename eastl::atomic<T>::value_type atomic_sub_fetch(eastl::atomic<T>* atomicObj,
																		  typename eastl::atomic<T>::difference_type arg) EA_NOEXCEPT
{ __builtin_trap() /* STUB: not implemented */; }

template <typename T, typename Order>
EASTL_FORCE_INLINE typename eastl::atomic<T>::value_type atomic_sub_fetch_explicit(eastl::atomic<T>* atomicObj,
																				   typename eastl::atomic<T>::difference_type arg,
																				   Order order) EA_NOEXCEPT
{ __builtin_trap() /* STUB: not implemented */; }


/////////////////////////////////////////////////////////////////////////////////
//
// T atomic_fetch_add(eastl::atomic<T>*, T arg)
//
template <typename T>
EASTL_FORCE_INLINE typename eastl::atomic<T>::value_type atomic_fetch_add(eastl::atomic<T>* atomicObj,
																		  typename eastl::atomic<T>::difference_type arg) EA_NOEXCEPT
{ __builtin_trap() /* STUB: not implemented */; }

template <typename T, typename Order>
EASTL_FORCE_INLINE typename eastl::atomic<T>::value_type atomic_fetch_add_explicit(eastl::atomic<T>* atomicObj,
																				   typename eastl::atomic<T>::difference_type arg,
																				   Order order) EA_NOEXCEPT
{ __builtin_trap() /* STUB: not implemented */; }


/////////////////////////////////////////////////////////////////////////////////
//
// T atomic_add_fetch(eastl::atomic<T>*, T arg)
//
template <typename T>
EASTL_FORCE_INLINE typename eastl::atomic<T>::value_type atomic_add_fetch(eastl::atomic<T>* atomicObj,
																		  typename eastl::atomic<T>::difference_type arg) EA_NOEXCEPT
{ __builtin_trap() /* STUB: not implemented */; }

template <typename T, typename Order>
EASTL_FORCE_INLINE typename eastl::atomic<T>::value_type atomic_add_fetch_explicit(eastl::atomic<T>* atomicObj,
																				   typename eastl::atomic<T>::difference_type arg,
																				   Order order) EA_NOEXCEPT
{ __builtin_trap() /* STUB: not implemented */; }


/////////////////////////////////////////////////////////////////////////////////
//
// T atomic_exchange(eastl::atomic<T>*, T desired)
//
template <typename T>
EASTL_FORCE_INLINE typename eastl::atomic<T>::value_type atomic_exchange(eastl::atomic<T>* atomicObj,
																		 typename eastl::atomic<T>::value_type desired) EA_NOEXCEPT
{ __builtin_trap() /* STUB: not implemented */; }

template <typename T, typename Order>
EASTL_FORCE_INLINE typename eastl::atomic<T>::value_type atomic_exchange_explicit(eastl::atomic<T>* atomicObj,
																				  typename eastl::atomic<T>::value_type desired,
																				  Order order) EA_NOEXCEPT
{ __builtin_trap() /* STUB: not implemented */; }


/////////////////////////////////////////////////////////////////////////////////
//
// T atomic_load(const eastl::atomic<T>*)
//
template <typename T>
EASTL_FORCE_INLINE typename eastl::atomic<T>::value_type atomic_load(const eastl::atomic<T>* atomicObj) EA_NOEXCEPT
{ __builtin_trap() /* STUB: not implemented */; }

template <typename T, typename Order>
EASTL_FORCE_INLINE typename eastl::atomic<T>::value_type atomic_load_explicit(const eastl::atomic<T>* atomicObj, Order order) EA_NOEXCEPT
{ __builtin_trap() /* STUB: not implemented */; }


/////////////////////////////////////////////////////////////////////////////////
//
// T atomic_load_cond(const eastl::atomic<T>*)
//
template <typename T, typename Predicate>
EASTL_FORCE_INLINE typename eastl::atomic<T>::value_type atomic_load_cond(const eastl::atomic<T>* atomicObj, Predicate pred) EA_NOEXCEPT
{ __builtin_trap() /* STUB: not implemented */; }

template <typename T, typename Predicate, typename Order>
EASTL_FORCE_INLINE typename eastl::atomic<T>::value_type atomic_load_cond_explicit(const eastl::atomic<T>* atomicObj, Predicate pred, Order order) EA_NOEXCEPT
{ __builtin_trap() /* STUB: not implemented */; }


/////////////////////////////////////////////////////////////////////////////////
//
// void atomic_store(eastl::atomic<T>*, T)
//
template <typename T>
EASTL_FORCE_INLINE void atomic_store(eastl::atomic<T>* atomicObj, typename eastl::atomic<T>::value_type desired) EA_NOEXCEPT
{ __builtin_trap() /* STUB: not implemented */; }

template <typename T, typename Order>
EASTL_FORCE_INLINE void atomic_store_explicit(eastl::atomic<T>* atomicObj, typename eastl::atomic<T>::value_type desired, Order order) EA_NOEXCEPT
{ __builtin_trap() /* STUB: not implemented */; }


/////////////////////////////////////////////////////////////////////////////////
//
// void eastl::atomic_thread_fence(Order)
//
template <typename Order>
EASTL_FORCE_INLINE void atomic_thread_fence(Order) EA_NOEXCEPT
{ __builtin_trap() /* STUB: not implemented */; }

EASTL_FORCE_INLINE void atomic_thread_fence(eastl::internal::memory_order_relaxed_s) EA_NOEXCEPT
{ __builtin_trap() /* STUB: not implemented */; }

EASTL_FORCE_INLINE void atomic_thread_fence(eastl::internal::memory_order_acquire_s) EA_NOEXCEPT
{ __builtin_trap() /* STUB: not implemented */; }

EASTL_FORCE_INLINE void atomic_thread_fence(eastl::internal::memory_order_release_s) EA_NOEXCEPT
{ __builtin_trap() /* STUB: not implemented */; }

EASTL_FORCE_INLINE void atomic_thread_fence(eastl::internal::memory_order_acq_rel_s) EA_NOEXCEPT
{ __builtin_trap() /* STUB: not implemented */; }

EASTL_FORCE_INLINE void atomic_thread_fence(eastl::internal::memory_order_seq_cst_s) EA_NOEXCEPT
{ __builtin_trap() /* STUB: not implemented */; }


/////////////////////////////////////////////////////////////////////////////////
//
// void eastl::atomic_signal_fence(Order)
//
template <typename Order>
EASTL_FORCE_INLINE void atomic_signal_fence(Order) EA_NOEXCEPT
{ __builtin_trap() /* STUB: not implemented */; }

EASTL_FORCE_INLINE void atomic_signal_fence(eastl::internal::memory_order_relaxed_s) EA_NOEXCEPT
{ __builtin_trap() /* STUB: not implemented */; }

EASTL_FORCE_INLINE void atomic_signal_fence(eastl::internal::memory_order_acquire_s) EA_NOEXCEPT
{ __builtin_trap() /* STUB: not implemented */; }

EASTL_FORCE_INLINE void atomic_signal_fence(eastl::internal::memory_order_release_s) EA_NOEXCEPT
{ __builtin_trap() /* STUB: not implemented */; }

EASTL_FORCE_INLINE void atomic_signal_fence(eastl::internal::memory_order_acq_rel_s) EA_NOEXCEPT
{ __builtin_trap() /* STUB: not implemented */; }

EASTL_FORCE_INLINE void atomic_signal_fence(eastl::internal::memory_order_seq_cst_s) EA_NOEXCEPT
{ __builtin_trap() /* STUB: not implemented */; }


/////////////////////////////////////////////////////////////////////////////////
//
// void eastl::compiler_barrier()
//
EASTL_FORCE_INLINE void compiler_barrier() EA_NOEXCEPT
{ __builtin_trap() /* STUB: not implemented */; }


/////////////////////////////////////////////////////////////////////////////////
//
// void eastl::compiler_barrier_data_dependency(const T&)
//
template <typename T>
EASTL_FORCE_INLINE void compiler_barrier_data_dependency(const T& val) EA_NOEXCEPT
{ __builtin_trap() /* STUB: not implemented */; }


/////////////////////////////////////////////////////////////////////////////////
//
// void eastl::cpu_pause()
//
EASTL_FORCE_INLINE void cpu_pause() EA_NOEXCEPT
{ __builtin_trap() /* STUB: not implemented */; }


/////////////////////////////////////////////////////////////////////////////////
//
// bool eastl::atomic_is_lock_free(eastl::atomic<T>*)
//
template <typename T>
EASTL_FORCE_INLINE bool atomic_is_lock_free(const eastl::atomic<T>* atomicObj) EA_NOEXCEPT
{ __builtin_trap() /* STUB: not implemented */; }


} // namespace eastl


#endif /* EASTL_ATOMIC_INTERNAL_STANDALONE_H */
