/////////////////////////////////////////////////////////////////////////////////
// Copyright (c) Electronic Arts Inc. All rights reserved.
/////////////////////////////////////////////////////////////////////////////////


#ifndef EASTL_ATOMIC_INTERNA_ATOMIC_FLAG_H
#define EASTL_ATOMIC_INTERNA_ATOMIC_FLAG_H

#if defined(EA_PRAGMA_ONCE_SUPPORTED)
	#pragma once
#endif


namespace eastl
{


// 'class' : multiple assignment operators specified
EA_DISABLE_VC_WARNING(4522);

// misaligned atomic operation may incur significant performance penalty
// The above warning is emitted in earlier versions of clang incorrectly.
// All eastl::atomic<T> objects are size aligned.
// This is static and runtime asserted.
// Thus we disable this warning.
EA_DISABLE_CLANG_WARNING(-Watomic-alignment);


class atomic_flag
{
public: /* ctors */

	EA_CONSTEXPR atomic_flag(bool desired) EA_NOEXCEPT
		: mFlag{ desired }
	{ __builtin_trap() /* STUB: not implemented */; }

	EA_CONSTEXPR atomic_flag() EA_NOEXCEPT
		: mFlag{ false }
	{ __builtin_trap() /* STUB: not implemented */; }

public: /* deleted ctors && assignment operators */

	atomic_flag(const atomic_flag&) EA_NOEXCEPT = delete;

	atomic_flag& operator=(const atomic_flag&)          EA_NOEXCEPT = delete;
	atomic_flag& operator=(const atomic_flag&) volatile EA_NOEXCEPT = delete;

public: /* clear */

	template <typename Order>
	void clear(Order /*order*/) volatile EA_NOEXCEPT
	{ __builtin_trap() /* STUB: not implemented */; }

	template <typename Order>
	void clear(Order /*order*/) EA_NOEXCEPT
	{ __builtin_trap() /* STUB: not implemented */; }

	void clear(eastl::internal::memory_order_relaxed_s) EA_NOEXCEPT
	{ __builtin_trap() /* STUB: not implemented */; }

	void clear(eastl::internal::memory_order_release_s) EA_NOEXCEPT
	{ __builtin_trap() /* STUB: not implemented */; }

	void clear(eastl::internal::memory_order_seq_cst_s) EA_NOEXCEPT
	{ __builtin_trap() /* STUB: not implemented */; }

	void clear() EA_NOEXCEPT
	{ __builtin_trap() /* STUB: not implemented */; }

public: /* test_and_set */

	template <typename Order>
	bool test_and_set(Order /*order*/) volatile EA_NOEXCEPT
	{ __builtin_trap() /* STUB: not implemented */; }

	template <typename Order>
	bool test_and_set(Order /*order*/) EA_NOEXCEPT
	{ __builtin_trap() /* STUB: not implemented */; }

	bool test_and_set(eastl::internal::memory_order_relaxed_s) EA_NOEXCEPT
	{ __builtin_trap() /* STUB: not implemented */; }

	bool test_and_set(eastl::internal::memory_order_acquire_s) EA_NOEXCEPT
	{ __builtin_trap() /* STUB: not implemented */; }

	bool test_and_set(eastl::internal::memory_order_release_s) EA_NOEXCEPT
	{ __builtin_trap() /* STUB: not implemented */; }

	bool test_and_set(eastl::internal::memory_order_acq_rel_s) EA_NOEXCEPT
	{ __builtin_trap() /* STUB: not implemented */; }

	bool test_and_set(eastl::internal::memory_order_seq_cst_s) EA_NOEXCEPT
	{ __builtin_trap() /* STUB: not implemented */; }

	bool test_and_set() EA_NOEXCEPT
	{ __builtin_trap() /* STUB: not implemented */; }

public: /* test */

	template <typename Order>
	bool test(Order /*order*/) const volatile EA_NOEXCEPT
	{ __builtin_trap() /* STUB: not implemented */; }

	template <typename Order>
	bool test(Order /*order*/) const EA_NOEXCEPT
	{ __builtin_trap() /* STUB: not implemented */; }

	bool test(eastl::internal::memory_order_relaxed_s) const EA_NOEXCEPT
	{ __builtin_trap() /* STUB: not implemented */; }

	bool test(eastl::internal::memory_order_acquire_s) const EA_NOEXCEPT
	{ __builtin_trap() /* STUB: not implemented */; }

	bool test(eastl::internal::memory_order_seq_cst_s) const EA_NOEXCEPT
	{ __builtin_trap() /* STUB: not implemented */; }

	bool test() const EA_NOEXCEPT
	{ __builtin_trap() /* STUB: not implemented */; }

private:

	eastl::atomic<bool> mFlag;
};

EA_RESTORE_VC_WARNING();

EA_RESTORE_CLANG_WARNING();


} // namespace eastl

#endif /* EASTL_ATOMIC_INTERNA_ATOMIC_FLAG_H */
