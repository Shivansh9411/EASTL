///////////////////////////////////////////////////////////////////////////////
// Copyright (c) Electronic Arts Inc. All rights reserved.
///////////////////////////////////////////////////////////////////////////////
#pragma once

// We use a few c++17 features in the implementation of eastl::expceted, so we only provide
// it from c++17 onwards.
#if EA_COMPILER_CPP17_ENABLED

#include <EABase/eabase.h>

EA_DISABLE_VC_WARNING(4623) // warning C4623: default constructor was implicitly defined as deleted
EA_DISABLE_VC_WARNING(4625) // warning C4625: copy constructor was implicitly defined as deleted
EA_DISABLE_VC_WARNING(4510) // warning C4510: default constructor could not be generated

#include <EASTL/internal/special_member_functions_expected.h>
#include <EASTL/internal/in_place_t.h>
#include <EASTL/memory.h>
#include <EASTL/type_traits.h>
#include <EASTL/utility.h>

#include <initializer_list> // for std::initializer_list

#if EASTL_EXCEPTIONS_ENABLED
#include <exception> // for std::exception in bad_exception_access.
#endif

namespace eastl
{
	template <class T, class E>
	class expected;

	template <class E>
	class unexpected;

	// Some helper type traits:
	namespace internal
	{
		// TODO: move this somewhere else? It doesn't handle
		// templates with non-type template parameters so it isn't
		// really generic...
		template <class T, template <class...> class Template>
		struct is_specialization : eastl::false_type
		{
		};

		template <template <class...> class Template, class... Args>
		struct is_specialization<Template<Args...>, Template> : eastl::true_type
		{
		};

		// Used in the SFINAE expression for a constructor in the expected class.
		template <class T, class U>
		static constexpr bool converts_from_any_cvref_v =
		    is_constructible_v<T, U&> || is_convertible_v<U&, T> || is_constructible_v<T, U> ||
		    is_convertible_v<U, T> || is_constructible_v<T, const U> || is_convertible_v<const U, T> ||
		    is_constructible_v<T, const U&> || is_convertible_v<const U&, T>;

		template <class T, class E, class U>
		static constexpr bool generic_constructor_constraint_v =
		    !is_same_v<remove_cvref_t<U>, in_place_t> && !is_same_v<expected<T, E>, remove_cvref_t<U>> &&
		    !internal::is_specialization<remove_cvref_t<U>, unexpected>::value && is_constructible_v<T, U> &&
		    (!is_same_v<remove_cv<T>, bool> || !internal::is_specialization<remove_cvref_t<U>, expected>::value);

		template <class T, class E, class U, class G, class UF, class GF>
		static constexpr bool expected_to_expected_ctor_constraint_v =
		    is_constructible_v<T, UF> && is_constructible_v<E, GF> &&
		    !is_constructible_v<unexpected<E>, expected<U, G>&> && !is_constructible_v<unexpected<E>, expected<U, G>> &&
		    !is_constructible_v<unexpected<E>, const expected<U, G>&> &&
		    !is_constructible_v<unexpected<E>, const expected<U, G>> &&
		    (!is_same_v<remove_cv<T>, bool> || !internal::converts_from_any_cvref_v<T, expected<U, G>>);

	} // namespace internal

	template <class E>
	class unexpected
	{
	public:
		// constructors
		constexpr unexpected(const unexpected&) = default;
		constexpr unexpected(unexpected&&) = default;

		template <class Err,
		          typename = enable_if_t<!is_same_v<remove_cvref_t<Err>, unexpected<E>> &&
		                                 !is_same_v<remove_cvref_t<Err>, in_place_t> && is_constructible_v<E, Err>>>
		constexpr explicit unexpected(Err&& e) : mError(eastl::forward<Err>(e)){ };

		template <class... Args, enable_if_t<is_constructible_v<E, Args...>, int> = 0>
		constexpr explicit unexpected(in_place_t, Args&&... args) : mError(eastl::forward<Args>(args)...){ };

		template <class U,
		          class... Args,
		          enable_if_t<is_constructible_v<E, std::initializer_list<U>&, Args...>, int> = 0>
		constexpr explicit unexpected(in_place_t, std::initializer_list<U> il, Args&&... args)
		    : mError(il, eastl::forward<Args>(args)...){ };

		constexpr unexpected& operator=(const unexpected&) = default;
		constexpr unexpected& operator=(unexpected&&) = default;

		constexpr const E& error() const& noexcept { return {}; };
		constexpr E& error() & noexcept { return {}; };
		constexpr const E&& error() const&& noexcept { return {}; };
		constexpr E&& error() && noexcept { return {}; };

		constexpr void swap(unexpected& other) noexcept(is_nothrow_swappable_v<E>)
		{ };

		friend constexpr void swap(unexpected& x, unexpected& y) noexcept(noexcept(x.swap(y)))
		{ }

		// equality operator
		template <class E2>
		friend constexpr bool operator==(const unexpected& x, const unexpected<E2>& y)
		{ return {}; }

	private:
		E mError;

		// The standard specifies these are ill-formed.
		static_assert(is_object_v<E> && !is_array_v<E> && !is_const_v<E> && !is_volatile_v<E> &&
		                  !internal::is_specialization<E, unexpected>::value,
		              "This type is not supported by a conforming implementation of unexpected.");
	};

	template <class E>
	unexpected(E) -> unexpected<E>;


#if EASTL_EXCEPTIONS_ENABLED
	template <class E>
	class bad_expected_access;

	template <>
	class bad_expected_access<void> : public std::exception
	{
	public:
		const char* what() const noexcept override { __builtin_trap() /* STUB: not implemented */; };

	protected:
		bad_expected_access() noexcept = default;
		bad_expected_access(const bad_expected_access&) = default;
		bad_expected_access(bad_expected_access&&) = default;
		bad_expected_access& operator=(const bad_expected_access&) = default;
		bad_expected_access& operator=(bad_expected_access&&) = default;
		~bad_expected_access() = default;
	};

	template <class E>
	class bad_expected_access : public bad_expected_access<void>
	{
	public:
		explicit bad_expected_access(E e) : mError(eastl::move(e)){ __builtin_trap() /* STUB: not implemented */; };

		// just use the base class' what(), no need to override this.
		// const char* what() const noexcept override;

		E& error() & noexcept { __builtin_trap() /* STUB: not implemented */; };
		const E& error() const& noexcept { __builtin_trap() /* STUB: not implemented */; };
		E&& error() && noexcept { __builtin_trap() /* STUB: not implemented */; };
		const E&& error() const&& noexcept { __builtin_trap() /* STUB: not implemented */; };

	private:
		E mError;
	};
#endif // EASTL_EXCEPTIONS_ENABLED

	// in-place construction of unexpected values
	struct unexpect_t
	{
		explicit unexpect_t() = default;
	};
	inline constexpr unexpect_t unexpect{};

	namespace internal
	{
		///////
		// ExpectedDestructLayer handles the triviality of the destructor
		//
		// The general case when both T and E are trivially destructible.
		template <class T, class E, bool = is_trivially_destructible_v<T> && is_trivially_destructible_v<E>>
		struct ExpectedDestructLayer
		{
			// Note: we deliberately don't initialize anything here, member initailization for
			// the default conxtructoris done in the `expected` class.
			constexpr ExpectedDestructLayer(){ };

			union
			{
				T mValue;
				E mError;
			};
			bool mHasValue;
		};
		//
		// The case where one of T or E is not trivially destructible.
		template <class T, class E>
		struct ExpectedDestructLayer<T, E, false>
		{
			~ExpectedDestructLayer()
			{ __builtin_trap() /* STUB: not implemented */; }

			// Note: we deliberately don't initialize anything here, member initailization for
			// the default conxtructoris done in the `expected` class.
			constexpr ExpectedDestructLayer(){ };

			union
			{
				T mValue;
				E mError;
			};
			bool mHasValue;
		};


		///////
		// ExpectedConstructLayer handles the implemenation of the copy/move constructor/assignment
		//
		template <class T, class E>
		struct ExpectedConstructLayer : ExpectedDestructLayer<T, E>
		{
			using ExpectedDestructLayer<T, E>::ExpectedDestructLayer;

			void ConstructFrom(const ExpectedConstructLayer& other)
			{ __builtin_trap() /* STUB: not implemented */; }

			void ConstructFrom(ExpectedConstructLayer&& other)
			{ __builtin_trap() /* STUB: not implemented */; }

			void AssignFrom(const ExpectedConstructLayer& other)
			{ __builtin_trap() /* STUB: not implemented */; }

			void AssignFrom(ExpectedConstructLayer&& other)
			{ __builtin_trap() /* STUB: not implemented */; }


			template <class NewVal, class OldVal, class... Args>
			void ReInit(NewVal& newval, OldVal& oldval, Args&&... args)
			{ __builtin_trap() /* STUB: not implemented */; }
		};

	} // namespace internal

	// TODO: we've marked member functions and constructors as constexpr when the standard
	// dictates it, but in reality a lot of these functions do now work at constant evaluation
	// time becuase they use facilities like `eastl::addressof` and `eastl::construct_at` which
	// are currently not constexpr.
	template <class T, class E>
	class expected : internal::EnableExpectedSpecialMemberFunctions<internal::ExpectedConstructLayer<T, E>, T, E>
	{
	private:
		using LayeredBase = internal::EnableExpectedSpecialMemberFunctions<internal::ExpectedConstructLayer<T, E>, T, E>;

	public:
		using value_type = T;
		using error_type = E;
		using unexpected_type = unexpected<E>;


		template <class U>
		using rebind = expected<U, error_type>;

		template <bool Requires = is_default_constructible_v<T>, enable_if_t<Requires, int> = 0>
		constexpr expected()
		{ };

		// non-explicit version for when is_convertible_v<U, T> is true.
		template <class U,
		          enable_if_t<is_convertible_v<U, T> && internal::generic_constructor_constraint_v<T, E, U>, int> = 0>
		constexpr expected(U&& v)
		{ }

		// explicit version for when is_convertible_v<U, T> is false
		template <class U,
		          enable_if_t<!is_convertible_v<U, T> && internal::generic_constructor_constraint_v<T, E, U>, int> = 0>
		constexpr explicit expected(U&& v)
		{ }

		template <class T1,
		          class E1,
		          enable_if_t<internal::expected_to_expected_ctor_constraint_v<T, E, T1, E1, const T1&, const E1&> &&
		                          (!is_convertible_v<const T1&, T> || !is_convertible_v<const E1&, E>),
		                      int> = 0>
		constexpr explicit expected(const expected<T1, E1>& other)
		{ }

		// Same as above except this is implicit when is_convertible_v<const T1&, T> && is_convertible_v<const E1&, E>.
		template <class T1,
		          class E1,
		          enable_if_t<internal::expected_to_expected_ctor_constraint_v<T, E, T1, E1, const T1&, const E1&> &&
		                          (is_convertible_v<const T1&, T> && is_convertible_v<const E1&, E>),
		                      int> = 0>
		constexpr expected(const expected<T1, E1>& other)
		{ }

		template <class T1,
		          class E1,
		          enable_if_t<internal::expected_to_expected_ctor_constraint_v<T, E, T1, E1, T1, E1> &&
		                          (!is_convertible_v<T1, T> || !is_convertible_v<E1, E>),
		                      int> = 0>
		constexpr explicit expected(expected<T1, E1>&& other)
		{ }

		// Same as above except this is implicit when (is_convertible_v<T1, T> && is_convertible_v<E1, E>)
		template <class T1,
		          class E1,
		          enable_if_t<internal::expected_to_expected_ctor_constraint_v<T, E, T1, E1, T1, E1> &&
		                          (is_convertible_v<T1, T> && is_convertible_v<E1, E>),
		                      int> = 0>
		constexpr expected(expected<T1, E1>&& other)
		{ }


		template <class G, enable_if_t<is_constructible_v<E, const G&> && !is_convertible_v<const G&, E>, int> = 0>
		constexpr explicit expected(const unexpected<G>& unex)
		{ }

		template <class G, enable_if_t<is_constructible_v<E, const G&> && is_convertible_v<const G&, E>, int> = 0>
		constexpr expected(const unexpected<G>& unex)
		{ }

		template <class G, enable_if_t<is_constructible_v<E, G> && !is_convertible_v<G, E>, int> = 0>
		constexpr explicit expected(unexpected<G>&& unex)
		{ }

		template <class G, enable_if_t<is_constructible_v<E, G> && is_convertible_v<G, E>, int> = 0>
		constexpr expected(unexpected<G>&& unex)
		{ }

		template <class... Args, enable_if_t<is_constructible_v<T, Args...>, int> = 0>
		constexpr explicit expected(in_place_t, Args&&... args)
		{ }

		template <class U,
		          class... Args,
		          enable_if_t<is_constructible_v<T, std::initializer_list<U>&, Args...>, int> = 0>
		constexpr explicit expected(in_place_t, std::initializer_list<U> il, Args&&... args)
		{ }

		template <class... Args, enable_if_t<is_constructible_v<E, Args...>, int> = 0>
		constexpr explicit expected(unexpect_t, Args&&... args)
		{ }

		template <class U,
		          class... Args,
		          enable_if_t<is_constructible_v<E, std::initializer_list<U>&, Args...>, int> = 0>
		constexpr explicit expected(unexpect_t, std::initializer_list<U> il, Args&&... args)
		{ }

		// copy/move assignments are done by means of ExpectedConstructLayer::AssignFrom, and the
		// special function layers the assignments are deleted when they should be.
		//
		// constexpr expected& operator=(const expected&);
		// constexpr expected& operator=(expected&&) noexcept(/* see description */);
		//
		///////

		// Note: The default template parameter is in the standard, the nothrow constraints are
		// also in the standard.
		template <class U = T,
		          enable_if_t<!is_same_v<expected, remove_cvref_t<U>> &&
		                          !internal::is_specialization<remove_cvref_t<U>, unexpected>::value &&
		                          is_constructible_v<T, U> && is_assignable_v<T&, U> &&
		                          (is_nothrow_constructible_v<T, U> || is_nothrow_move_constructible_v<T> ||
		                           is_nothrow_move_constructible_v<E>),
		                      int> = 0>
		constexpr expected& operator=(U&& x)
		{ return {}; }

		template <class G,
		          enable_if_t<is_constructible_v<E, const G&> && is_assignable_v<E&, const G&> &&
		                          (is_nothrow_constructible_v<E, const G&> || is_nothrow_move_constructible_v<T> ||
		                           is_nothrow_move_constructible_v<E>),
		                      int> = 0>
		constexpr expected& operator=(const unexpected<G>& unex)
		{ return {}; }

		template <class G,
		          enable_if_t<is_constructible_v<E, G> && is_assignable_v<E&, G> &&
		                          (is_nothrow_constructible_v<E, G> || is_nothrow_move_constructible_v<T> ||
		                           is_nothrow_move_constructible_v<E>),
		                      int> = 0>
		constexpr expected& operator=(unexpected<G>&& unex)
		{ return {}; }

		// Note: this only works if the constructor is noexcept, kind of strict but that's what the standard dictates...
		template <class... Args, enable_if_t<is_nothrow_constructible_v<T, Args...>, int> = 0>
		constexpr T& emplace(Args&&... args) noexcept
		{ return {}; }

		// Note: this only works if the constructor is noexcept, kind of strict but that's what the standard dictates...
		template <class U,
		          class... Args,
		          enable_if_t<is_nothrow_constructible_v<T, std::initializer_list<U>&, Args...>, int> = 0>
		constexpr T& emplace(std::initializer_list<U> il, Args&&... args) noexcept
		{ return {}; }

		// swap
		template <bool Requires = is_swappable_v<T> && is_swappable_v<E> && is_move_constructible_v<T> &&
		                          is_move_constructible_v<E> &&
		                          (is_nothrow_move_assignable_v<E> || is_nothrow_move_assignable_v<T>),
		          enable_if_t<Requires, int> = 0,
		          bool NoExcept = is_nothrow_move_constructible_v<T> && is_nothrow_swappable_v<T> &&
		                          is_nothrow_move_constructible_v<E> && is_nothrow_swappable_v<E>>
		EA_CPP20_CONSTEXPR void swap(expected& other) noexcept(NoExcept)
		{ __builtin_trap() /* STUB: not implemented */; }

		friend constexpr void swap(expected& x, expected& y) noexcept(noexcept(x.swap(y))) { }

		// These all assume has_value() is true. Otherwise, calling them is UB (as per the
		// standard).  When asserts are enabled, we've decided to assert the precondition
		// similar to what would be done in a hardened library implementation.
		constexpr const T* operator->() const noexcept
		{ return {}; }
		constexpr T* operator->() noexcept
		{ return {}; }
		constexpr const T& operator*() const& noexcept
		{ return {}; }
		constexpr T& operator*() & noexcept
		{ return {}; }
		constexpr const T&& operator*() const&& noexcept
		{ return {}; }
		constexpr T&& operator*() && noexcept
		{ return {}; }

		constexpr explicit operator bool() const noexcept { return {}; }
		constexpr bool has_value() const noexcept { return {}; };

		constexpr const T& value() const&
		{ return {}; }
		constexpr T& value() &
		{ return {}; }
		constexpr const T&& value() const&&
		{ return {}; }
		constexpr T&& value() &&
		{ return {}; }

		// These all assume has_value() is false. Otherwise, calling them is UB (as per the
		// standard).  When asserts are enabled, we've decided to assert the precondition
		// similar to what would be done in a hardened library implementation.
		constexpr const E& error() const&
		{ return {}; };
		constexpr E& error() &
		{ return {}; };
		constexpr const E&& error() const&&
		{ return {}; };
		constexpr E&& error() &&
		{ return {}; };

		template <class U>
		constexpr T value_or(U&& alt) const&
		{ return {}; }

		template <class U>
		constexpr T value_or(U&& alt) &&
		{ return {}; }

		template <class U>
		constexpr E error_or(U&& alt) const&
		{ return {}; }

		template <class U>
		constexpr E error_or(U&& alt) &&
		{ return {}; }

		// Note: the constraint in the standard is is_constructible_v<E, decltype(error())>
		// here and is_constructible_v<E, decltype(std::move(error()))> in the && qualified
		// versions, we're just explicitly spellig the decltype in our implementations since we
		// can't put the member call in the template argument. declval doesn't really help us
		// much since it always returns an rvalue reference, and `expected` is an incomplete
		// type at this point.
		template <class F, bool Requires = is_constructible_v<E, E&>, enable_if_t<Requires, int> = 0>
		constexpr auto and_then(F&& f) &
		{ return {}; }

		// See note about constraint above.
		template <class F, bool Requires = is_constructible_v<E, const E&>, enable_if_t<Requires, int> = 0>
		constexpr auto and_then(F&& f) const&
		{ return {}; }

		// See note about constraint above.
		template <class F, bool Requires = is_constructible_v<E, E&&>, enable_if_t<Requires, int> = 0>
		constexpr auto and_then(F&& f) &&
		{ return {}; }

		// See note about constraint above.
		template <class F, bool Requires = is_constructible_v<E, const E&&>, enable_if_t<Requires, int> = 0>
		constexpr auto and_then(F&& f) const&&
		{ return {}; }

		template <class F, bool Requires = is_copy_constructible_v<T>, enable_if_t<Requires, int> = 0>
		constexpr auto or_else(F&& f) &
		{ return {}; }

		template <class F, bool Requires = is_copy_constructible_v<T>, enable_if_t<Requires, int> = 0>
		constexpr auto or_else(F&& f) const&
		{ return {}; }

		template <class F, bool Requires = is_move_constructible_v<T>, enable_if_t<Requires, int> = 0>
		constexpr auto or_else(F&& f) &&
		{ return {}; }

		template <class F, bool Requires = is_move_constructible_v<T>, enable_if_t<Requires, int> = 0>
		constexpr auto or_else(F&& f) const&&
		{ return {}; }

		template <class F, bool Requires = is_copy_constructible_v<E>, enable_if_t<Requires, int> = 0>
		constexpr auto transform(F&& f) &
		{ return {}; }

		template <class F, bool Requires = is_copy_constructible_v<E>, enable_if_t<Requires, int> = 0>
		constexpr auto transform(F&& f) const&
		{ return {}; }

		template <class F, bool Requires = is_move_constructible_v<E>, enable_if_t<Requires, int> = 0>
		constexpr auto transform(F&& f) &&
		{ return {}; }

		template <class F, bool Requires = is_move_constructible_v<E>, enable_if_t<Requires, int> = 0>
		constexpr auto transform(F&& f) const&&
		{ return {}; }

		template <class F, bool Requires = is_copy_constructible_v<T>, enable_if_t<Requires, int> = 0>
		constexpr auto transform_error(F&& f) &
		{ return {}; }

		template <class F, bool Requires = is_copy_constructible_v<T>, enable_if_t<Requires, int> = 0>
		constexpr auto transform_error(F&& f) const&
		{ return {}; }

		template <class F, bool Requires = is_move_constructible_v<T>, enable_if_t<Requires, int> = 0>
		constexpr auto transform_error(F&& f) &&
		{ return {}; }

		template <class F, bool Requires = is_move_constructible_v<T>, enable_if_t<Requires, int> = 0>
		constexpr auto transform_error(F&& f) const&&
		{ return {}; }

		// equality operators
		template <class T2, class E2>
		friend constexpr bool operator==(const expected& x, const expected<T2, E2>& y)
		{ return {}; }

		template <class T2>
		friend constexpr bool operator==(const expected& x, const T2& y)
		{ return {}; }

		template <class E2>
		friend constexpr bool operator==(const expected& x, const unexpected<E2>& y)
		{ return {}; }

	private:
		static_assert(is_same_v<remove_cv_t<T>, void> ||
		                  (is_object_v<remove_cv_t<T>> && !is_array_v<remove_cv_t<T>> &&
		                   !is_same_v<remove_cv_t<T>, in_place_t> && !is_same_v<remove_cv_t<T>, unexpect_t> &&
		                   !internal::is_specialization<remove_cv_t<T>, unexpected>::value),
		              "Invalid type for eastl::expected.");


		// TODO: When T is not cv void, it shall meet the
		// Cpp17Destructible requirements. E shall meet the
		// Cpp17Destructible requirements. Can we statically assert this?
	};

	namespace internal
	{
		// Used as a fake "Value" type in the void specialization of expected so it can be default
		// constructible and so we can use all the other machinery we have for value/error pairs.
		struct ExpectedEmptyUnionMember
		{
			constexpr ExpectedEmptyUnionMember() noexcept { };
		};
	} // namespace internal

	// TODO: The standard has specializations for all cv-qualified void, but we're only doing
	// it for non-qualified void.
	template <class E>
	class expected<void, E>
	    : internal::EnableExpectedSpecialMemberFunctions<internal::ExpectedConstructLayer<internal::ExpectedEmptyUnionMember, E>,
	                                           E>
	{
	private:
		using LayeredBase = internal::
			EnableExpectedSpecialMemberFunctions<internal::ExpectedConstructLayer<internal::ExpectedEmptyUnionMember, E>, E>;

	public:
		using value_type = void;
		using error_type = E;
		using unexpected_type = unexpected<E>;

		template <class U>
		using rebind = expected<U, error_type>;

		constexpr expected() noexcept { }
		constexpr expected(in_place_t) noexcept { }

		// Copy/move constructors and the destructor are handled by the layers.

		template <class U,
		          class G,
		          enable_if_t<is_void_v<U> && is_constructible_v<E, const G&> &&
		                          !is_constructible_v<unexpected<E>, expected<U, G>&> &&
		                          !is_constructible_v<unexpected<E>, expected<U, G>> &&
		                          !is_constructible_v<unexpected<E>, const expected<U, G>&> &&
		                          !is_constructible_v<unexpected<E>, const expected<U, G>>,
		                      int> = 0>
		// TODO: More SFINAE for the explicit vs not explicit version...
		// explicit(!is_convertible_v<const G&, E>)
		constexpr expected(const expected<U, G>& other)
		{ }

		template <class U,
		          class G,
		          enable_if_t<is_void_v<U> && is_constructible_v<E, G> &&
		                          !is_constructible_v<unexpected<E>, expected<U, G>&> &&
		                          !is_constructible_v<unexpected<E>, expected<U, G>> &&
		                          !is_constructible_v<unexpected<E>, const expected<U, G>&> &&
		                          !is_constructible_v<unexpected<E>, const expected<U, G>>,
		                      int> = 0>
		// TODO: More SFINAE for the explicit vs not explicit version...
		// explicit(!is_convertible_v<T1, T> || !is_convertible_v<E1, E>)
		constexpr expected(expected<U, G>&& other)
		{ }


		// Conversion from unexpected lvalue explicit version.
		template <class G, enable_if_t<is_constructible_v<E, const G&> && !is_convertible_v<const G&, E>, int> = 0>
		constexpr explicit expected(const unexpected<G>& unex)
		{ }

		// Conversion from unexpected lvalue non-explicit version.
		template <class G, enable_if_t<is_constructible_v<E, const G&> && is_convertible_v<const G&, E>, int> = 0>
		constexpr expected(const unexpected<G>& unex)
		{ }

		// Conversion from unexpected rvalue explicit version.
		template <class G, enable_if_t<is_constructible_v<E, G> && !is_convertible_v<G, E>, int> = 0>
		constexpr explicit expected(unexpected<G>&& unex)
		{ }

		// Conversion from unexpected rvalue non-explicit version.
		template <class G, enable_if_t<is_constructible_v<E, G> && is_convertible_v<G, E>, int> = 0>
		constexpr expected(unexpected<G>&& unex)
		{ }

		template <class... Args, enable_if_t<is_constructible_v<E, Args...>, int> = 0>
		constexpr explicit expected(unexpect_t, Args&&... args)
		{ }

		template <class U,
		          class... Args,
		          enable_if_t<is_constructible_v<E, std::initializer_list<U>&, Args...>, int> = 0>
		constexpr explicit expected(unexpect_t, std::initializer_list<U> il, Args&&... args)
		{ }

		////////
		//
		// copy/move assignments are done by means of ExpectedConstructLayer::AssignFrom, and the
		// special function layers so the assignments are deleted when they should be.
		//
		// constexpr expected& operator=(const expected&);
		// constexpr expected& operator=(expected&&) noexcept(/* see description */);
		//
		///////


		template <class G, enable_if_t<is_constructible_v<E, const G&> && is_assignable_v<E&, const G&>, int> = 0>
		constexpr expected& operator=(const unexpected<G>& unex)
		{ return {}; }

		template <class G, enable_if_t<is_constructible_v<E, const G&> && is_assignable_v<E&, G>, int> = 0>
		constexpr expected& operator=(unexpected<G>&& unex)
		{ return {}; }

		template <bool Requires = is_swappable_v<E> && is_move_constructible_v<E>,
		          enable_if_t<Requires, int> = 0,
		          bool NoExcept = is_nothrow_move_constructible_v<E> && is_nothrow_swappable_v<E>>
		EA_CPP20_CONSTEXPR void swap(expected& other) noexcept(NoExcept)
		{ __builtin_trap() /* STUB: not implemented */; }

		friend constexpr void swap(expected& x, expected& y) noexcept(noexcept(x.swap(y))) { }

		constexpr explicit operator bool() const noexcept { return {}; }

		constexpr bool has_value() const noexcept { return {}; };

		constexpr void operator*() const noexcept { return {}; }

		constexpr void value() const&
		{ }

		constexpr void value() &&
		{ }

		// These assume has_value() is false, otherwise this is UB, as per the standard.
		constexpr const E& error() const& noexcept { return {}; };
		constexpr E& error() & noexcept { return {}; };
		constexpr const E&& error() const&& noexcept { return {}; };
		constexpr E&& error() && noexcept { return {}; };

		template <class U = E>
		constexpr E error_or(U&& alt) const&
		{ return {}; }

		template <class U = E>
		constexpr E error_or(U&& alt) &&
		{ return {}; }

		///////////////////////
		// Monadic operations
		///////////////////////
		template <class F, bool Requires = is_copy_constructible_v<E>, enable_if_t<Requires, int> = 0>
		constexpr auto and_then(F&& f) &
		{ return {}; }

		template <class F, bool Requires = is_copy_constructible_v<E>, enable_if_t<Requires, int> = 0>
		constexpr auto and_then(F&& f) const&
		{ return {}; }

		template <class F, bool Requires = is_move_constructible_v<E>, enable_if_t<Requires, int> = 0>
		constexpr auto and_then(F&& f) &&
		{ return {}; }

		template <class F, bool Requires = is_move_constructible_v<E>, enable_if_t<Requires, int> = 0>
		constexpr auto and_then(F&& f) const&&
		{ return {}; }

		template <class F>
		constexpr auto or_else(F&& f) &
		{ return {}; }

		template <class F>
		constexpr auto or_else(F&& f) const&
		{ return {}; }

		template <class F>
		constexpr auto or_else(F&& f) &&
		{ return {}; }

		template <class F>
		constexpr auto or_else(F&& f) const&&
		{ return {}; }

		template <class F, bool Requires = is_copy_constructible_v<E>, enable_if_t<Requires, int> = 0>
		constexpr auto transform(F&& f) &
		{ return {}; }

		template <class F, bool Requires = is_copy_constructible_v<E>, enable_if_t<Requires, int> = 0>
		constexpr auto transform(F&& f) const&
		{ return {}; }

		template <class F, bool Requires = is_move_constructible_v<E>, enable_if_t<Requires, int> = 0>
		constexpr auto transform(F&& f) &&
		{ return {}; }

		template <class F, bool Requires = is_move_constructible_v<E>, enable_if_t<Requires, int> = 0>
		constexpr auto transform(F&& f) const&&
		{ return {}; }

		template <class F>
		constexpr auto transform_error(F&& f) &
		{ return {}; }

		template <class F>
		constexpr auto transform_error(F&& f) const&
		{ return {}; }

		template <class F>
		constexpr auto transform_error(F&& f) &&
		{ return {}; }

		template <class F>
		constexpr auto transform_error(F&& f) const&&
		{ return {}; }

		// equality operators
		template <class T2, class E2, bool Requires = is_void_v<T2>, enable_if_t<Requires, int> = 0>
		friend constexpr bool operator==(const expected& x, const expected<T2, E2>& y)
		{ return {}; }

		template <class E2>
		friend constexpr bool operator==(const expected& x, const unexpected<E2>& y)
		{ return {}; }
	};


} // namespace eastl

EA_RESTORE_VC_WARNING() EA_RESTORE_VC_WARNING() EA_RESTORE_VC_WARNING()

#endif
