#include <stdexcept>
///////////////////////////////////////////////////////////////////////////////
// Copyright (c) Electronic Arts Inc. All rights reserved.
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// This file implements the following functions from the C++ standard that
// are found in the <memory> header:
//
// Utility:
//    late_constructed                  - Extention to standard functionality.
//
// Uninitialized operations:
//    These are the same as the copy, fill, and fill_n algorithms, except that
//    they *construct* the destination with the source values rather than assign
//    the destination with the source values.
//
//    uninitialized_copy
//    uninitialized_copy_n
//    uninitialized_default_construct
//    uninitialized_default_construct_n
//    uninitialized_move
//    uninitialized_move_if_noexcept    - Extention to standard functionality.
//    uninitialized_move_n
//    uninitialized_fill
//    uninitialized_fill_n
//    uninitialized_value_construct
//    uninitialized_value_construct_n
//    uninitialized_copy_fill           - Extention to standard functionality.
//    uninitialized_fill_copy           - Extention to standard functionality.
//    uninitialized_copy_copy           - Extention to standard functionality.
//
// In-place destructor helpers:
//    destruct(T*)                      - Non-standard extension. Equivalent to destroy_at(T*)
//    destruct(first, last)             - Non-standard extension. Equivalent to destroy(first, last)
//    destroy_at(T*)
//    destroy(first, last)
//    destroy_n(first, n)
//
// Alignment
//    align
//    align_advance                     - Extention to standard functionality.
//
// Allocator-related
//    uses_allocator
//    allocator_arg_t
//    allocator_arg
//
// Pointers
//    pointer_traits
///////////////////////////////////////////////////////////////////////////////


#ifndef EASTL_MEMORY_H
#define EASTL_MEMORY_H


#include <EASTL/internal/config.h>
#include <EASTL/internal/memory_base.h>
#include <EASTL/internal/memory_uses_allocator.h>
#include <EASTL/internal/pair_fwd_decls.h>
#include <EASTL/internal/functional_base.h>
#include <EASTL/algorithm.h>
#include <EASTL/type_traits.h>
#include <EASTL/iterator.h>
#include <EASTL/utility.h>
#include <EASTL/numeric_limits.h>

EA_DISABLE_ALL_VC_WARNINGS()
#include <stdlib.h>
#include <new>
EA_RESTORE_ALL_VC_WARNINGS()


// 4530 - C++ exception handler used, but unwind semantics are not enabled. Specify /EHsc
// 4146 - unary minus operator applied to unsigned type, result still unsigned
// 4571 - catch(...) semantics changed since Visual C++ 7.1; structured exceptions (SEH) are no longer caught.
EA_DISABLE_VC_WARNING(4530 4146 4571);


#if defined(EA_PRAGMA_ONCE_SUPPORTED)
	#pragma once // Some compilers (e.g. VC++) benefit significantly from using this. We've measured 3-4% build speed improvements in apps as a result.
#endif


namespace eastl
{

	/// EASTL_TEMP_DEFAULT_NAME
	///
	/// Defines a default container name in the absence of a user-provided name.
	///
	#ifndef EASTL_TEMP_DEFAULT_NAME
		#define EASTL_TEMP_DEFAULT_NAME EASTL_DEFAULT_NAME_PREFIX " temp" // Unless the user overrides something, this is "EASTL temp".
	#endif


	/// late_constructed
	///
	/// Implements a smart pointer type which separates the memory allocation of an object from
	/// the object's construction. The primary use case is to declare a global variable of the
	/// late_construction type, which allows the memory to be global but the constructor executes
	/// at some point after main() begins as opposed to before main, which is often dangerous
	/// for non-trivial types.
	///
	/// The autoConstruct template parameter controls whether the object is automatically default
	/// constructed upon first reference or must be manually constructed upon the first use of
	/// operator * or ->. autoConstruct is convenient but it causes * and -> to be slightly slower
	/// and may result in construction at an inconvenient time.
	///
	/// The autoDestruct template parameter controls whether the object, if constructed, is automatically
	/// destructed when ~late_constructed() is called or must be manually destructed via a call to
	/// destruct().
	///
	/// While construction can be automatic or manual, automatic destruction support is always present.
	/// Thus you aren't required in any case to manually call destruct. However, you may safely manually
	/// destruct the object at any time before the late_constructed destructor is executed.
	///
	/// You may still use late_constructed after calling destruct(), including calling construct()
	/// again to reconstruct the instance. destruct returns the late_constructed instance to a
	/// state equivalent to before construct was called.
	///
	/// Caveat: While late_constructed instances can be declared in global scope and initialize
	/// prior to main() executing, you cannot otherwise use such globally declared instances prior
	/// to main with guaranteed behavior unless you can ensure that the late_constructed instance
	/// is itself constructed prior to your use of it.
	///
	/// Example usage (demonstrating manual-construction):
	///     late_constructed<Widget, false> gWidget;
	///
	///     void main(){
	///         gWidget.construct(kScrollbarType, kVertical, "MyScrollbar");
	///         gWidget->SetValue(15);
	///         gWidget.destruct();
	///     }
	///
	/// Example usage (demonstrating auto-construction):
	///     late_constructed<Widget, true> gWidget;
	///
	///     void main(){
	///         gWidget->SetValue(15);
	///         // You may want to call destruct here, but aren't required to do so unless the Widget type requires it.
	///     }
	///
	template <typename T, bool autoConstruct = true, bool autoDestruct = true>
	class late_constructed
	{
	public:
		using this_type    = late_constructed<T, autoConstruct, autoDestruct>;
		using value_type   = T;
		using storage_type = eastl::aligned_storage_t<sizeof(value_type), eastl::alignment_of_v<value_type>>;

		constexpr late_constructed() noexcept  // In the case of the late_constructed instance being at global scope, we rely on the
		  : mStorage(), mpValue(nullptr) {
    
}    // compiler executing this constructor or placing the instance in auto-zeroed-at-startup memory.

		~late_constructed()
		{
			if (autoDestruct && mpValue)
				(*mpValue).~value_type();
		}

		template <typename... Args>
		void construct(Args&&... args)
		{
    __builtin_trap() /* STUB: not implemented */;
}

		bool is_constructed() const noexcept
			{
    __builtin_trap() /* STUB: not implemented */;
}

		void destruct()
		{
    __builtin_trap() /* STUB: not implemented */;
}

		value_type& operator*() noexcept
		{
    __builtin_trap() /* STUB: not implemented */;
}

		const value_type& operator*() const noexcept
		{
    __builtin_trap() /* STUB: not implemented */;
}

		value_type* operator->() noexcept
		{
    __builtin_trap() /* STUB: not implemented */;
}

		const value_type* operator->() const noexcept
		{
    __builtin_trap() /* STUB: not implemented */;
}

		value_type* get() noexcept
		{
    __builtin_trap() /* STUB: not implemented */;
}

		const value_type* get() const noexcept
		{
    __builtin_trap() /* STUB: not implemented */;
}

	protected:
		storage_type mStorage; // Declared first because it may have aligment requirements, and it would be more space-efficient if it was first.
		value_type*  mpValue;
	};


	// Specialization that doesn't auto-construct on demand.
	template <typename T, bool autoDestruct>
	class late_constructed<T, false, autoDestruct> : public late_constructed<T, true, autoDestruct>
	{
	public:
		typedef late_constructed<T, true, autoDestruct> base_type;

		typename base_type::value_type& operator*() noexcept
			{
    __builtin_trap() /* STUB: not implemented */;
}

		const typename base_type::value_type& operator*() const noexcept
			{
    __builtin_trap() /* STUB: not implemented */;
}

		typename base_type::value_type* operator->() noexcept
			{
    __builtin_trap() /* STUB: not implemented */;
}

		const typename base_type::value_type* operator->() const noexcept
			{
    __builtin_trap() /* STUB: not implemented */;
}

		typename base_type::value_type* get() noexcept
			{
    __builtin_trap() /* STUB: not implemented */;
}

		const typename base_type::value_type* get() const noexcept
			{
    __builtin_trap() /* STUB: not implemented */;
}
	};


	// uninitialized_copy
	//
	namespace Internal
	{
		template<bool isTriviallyCopyable, bool isInputIteratorReferenceAddressable, bool areIteratorsContiguous>
		struct uninitialized_copy_impl
		{
			template <typename InputIterator, typename ForwardIterator>
			static ForwardIterator impl(InputIterator first, InputIterator last, ForwardIterator dest)
			{
    __builtin_trap() /* STUB: not implemented */;
}
		};

		template<>
		struct uninitialized_copy_impl<true, true, false>
		{
			template <typename InputIterator, typename ForwardIterator>
			static ForwardIterator impl(InputIterator first, InputIterator last, ForwardIterator dest)
			{
    __builtin_trap() /* STUB: not implemented */;
}
		};

		template<>
		struct uninitialized_copy_impl<true, true, true>
		{
			template <typename InputIterator, typename ForwardIterator>
			static ForwardIterator impl(InputIterator first, InputIterator last, ForwardIterator dest)
			{
    __builtin_trap() /* STUB: not implemented */;
}
		};
	}

	/// uninitialized_copy
	///
	/// Copies a source range to a destination, copy-constructing the destination with
	/// the source values (and not *assigning* the destination with the source values).
	/// Returns the end of the destination range (i.e. dest + (last - first)).
	///
	/// Declaration:
	///    template <typename InputIterator, typename ForwardIterator>
	///    ForwardIterator uninitialized_copy(InputIterator sourceFirst, InputIterator sourceLast, ForwardIterator destination);
	///
	/// Example usage:
	///    SomeClass* pArray = malloc(10 * sizeof(SomeClass));
	///    uninitialized_copy(pSourceDataBegin, pSourceDataBegin + 10, pArray);
	///
	template <typename InputIterator, typename ForwardIterator>
	inline ForwardIterator uninitialized_copy(InputIterator first, InputIterator last, ForwardIterator result)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	/// uninitialized_copy_n
	///
	/// Copies count elements from a range beginning at first to an uninitialized memory area
	/// beginning at dest. The elements in the uninitialized area are constructed using copy constructor.
	/// If an exception is thrown during the initialization, the function has no final effects.
	///
	/// first:        Beginning of the range of the elements to copy.
	/// dest:         Beginning of the destination range.
	/// return value: Iterator of dest type to the element past the last element copied.
	///
	namespace Internal
	{
		template <typename InputIterator, typename Count, typename ForwardIterator, typename IteratorTag>
		struct uninitialized_copy_n_impl
		{
			static ForwardIterator impl(InputIterator first, Count n, ForwardIterator dest)
			{
    __builtin_trap() /* STUB: not implemented */;
}
		};

		template <typename InputIterator, typename Count, typename ForwardIterator>
		struct uninitialized_copy_n_impl<InputIterator, Count, ForwardIterator, eastl::random_access_iterator_tag>
		{
			static inline ForwardIterator impl(InputIterator first, Count n, ForwardIterator dest)
			{
    __builtin_trap() /* STUB: not implemented */;
}
		};
	}

	template<typename InputIterator, typename Count, typename ForwardIterator>
	inline ForwardIterator uninitialized_copy_n(InputIterator first, Count n, ForwardIterator dest)
	{
    __builtin_trap() /* STUB: not implemented */;
}



	/// uninitialized_copy_ptr
	///
	/// This is a specialization of uninitialized_copy for iterators that are pointers. We use it because
	/// internally it uses generic_iterator to make pointers act like regular eastl::iterator.
	///
	template <typename First, typename Last, typename Result>
	EASTL_REMOVE_AT_2024_SEPT inline Result uninitialized_copy_ptr(First first, Last last, Result result)
	{
    __builtin_trap() /* STUB: not implemented */;
}



	/// uninitialized_move_ptr
	///
	/// This is a specialization of uninitialized_move for iterators that are pointers. We use it because
	/// internally it uses generic_iterator to make pointers act like regular eastl::iterator.
	///
	template <typename First, typename Last, typename Result>
	EASTL_REMOVE_AT_2024_SEPT inline Result uninitialized_move_ptr(First first, Last last, Result dest)
	{
    __builtin_trap() /* STUB: not implemented */;
}




	/// uninitialized_move
	///
	/// Moves a source range to a destination, move-constructing the destination with
	/// the source values (and not *assigning* the destination with the source values).
	/// Returns the end of the destination range (i.e. dest + (last - first)).
	///
	/// Example usage:
	///    SomeClass* pArray = malloc(10 * sizeof(SomeClass));
	///    uninitialized_move(pSourceDataBegin, pSourceDataBegin + 10, pArray);
	///
	template <typename InputIterator, typename ForwardIterator>
	inline ForwardIterator uninitialized_move(InputIterator first, InputIterator last, ForwardIterator dest)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	/// uninitialized_move_if_noexcept
	///
	/// If the iterated type can be moved without exceptions, move construct the dest with the input. Else copy-construct
	/// the dest witih the input. If move isn't supported by the compiler, do regular copy.
	///
	template <typename InputIterator, typename ForwardIterator>
	inline ForwardIterator uninitialized_move_if_noexcept(InputIterator first, InputIterator last, ForwardIterator dest)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	/// uninitialized_move_ptr_if_noexcept
	///
	template <typename First, typename Last, typename Result>
	EASTL_REMOVE_AT_2024_SEPT inline Result uninitialized_move_ptr_if_noexcept(First first, Last last, Result dest)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	/// uninitialized_move_n
	///
	/// Moves count elements from a range beginning at first to an uninitialized memory area
	/// beginning at dest. The elements in the uninitialized area are constructed using copy constructor.
	/// If an exception is thrown during the initialization, the function has no final effects.
	///
	/// first:        Beginning of the range of the elements to move.
	/// dest:         Beginning of the destination range.
	/// return value: Iterator of dest type to the element past the last element moved.
	///
	template<typename InputIterator, typename Count, typename ForwardIterator>
	inline ForwardIterator uninitialized_move_n(InputIterator first, Count n, ForwardIterator dest)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	/// uninitialized_default_construct
	///
	/// Constructs objects in the uninitialized storage designated by the range [first, last) by default-initialization.
	///
	/// Default-initialization:
	///  If T is a class, the default constructor is called; otherwise, no initialization is done, resulting in
	///  indeterminate values.
	///
	/// http://en.cppreference.com/w/cpp/memory/uninitialized_default_construct
	///
	template <typename ForwardIterator>
	inline void uninitialized_default_construct(ForwardIterator first, ForwardIterator last)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	/// uninitialized_default_construct_n
	///
	/// Constructs n objects in the uninitialized storage starting at first by default-initialization.
	///
	/// http://en.cppreference.com/w/cpp/memory/uninitialized_default_construct_n
	///
	template <typename ForwardIterator, typename Count>
	inline ForwardIterator uninitialized_default_construct_n(ForwardIterator first, Count n)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	/// uninitialized_fill
	///
	/// Copy-constructs the elements in the destination range with the given input value.
	/// Returns void. It wouldn't be useful to return the end of the destination range,
	/// as that is the same as the 'last' input parameter.
	///
	/// Declaration:
	///    template <typename ForwardIterator, typename T>
	///    void uninitialized_fill(ForwardIterator destinationFirst, ForwardIterator destinationLast, const T& value);
	///
	namespace Internal
	{
		template <typename ForwardIterator, typename T>
		inline void uninitialized_fill_impl(ForwardIterator first, ForwardIterator last, const T& value, true_type)
		{
    __builtin_trap() /* STUB: not implemented */;
}

		template <typename ForwardIterator, typename T>
		void uninitialized_fill_impl(ForwardIterator first, ForwardIterator last, const T& value, false_type)
		{
    __builtin_trap() /* STUB: not implemented */;
}
	}

	template <typename ForwardIterator, typename T>
	inline void uninitialized_fill(ForwardIterator first, ForwardIterator last, const T& value)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	/// uninitialized_value_construct
	///
	/// Constructs objects in the uninitialized storage range [first, last) by value-initialization.
	///
	/// Value-Initialization:
	/// If T is a class, the object is default-initialized (after being zero-initialized if T's default
	/// constructor is not user-provided/deleted); otherwise, the object is zero-initialized.
	///
	/// http://en.cppreference.com/w/cpp/memory/uninitialized_value_construct
	///
	template <class ForwardIterator>
	void uninitialized_value_construct(ForwardIterator first, ForwardIterator last)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	/// uninitialized_value_construct_n
	///
	/// Constructs n objects in the uninitialized storage starting at first by value-initialization.
	///
	/// Value-Initialization:
	/// If T is a class, the object is default-initialized (after being zero-initialized if T's default
	/// constructor is not user-provided/deleted); otherwise, the object is zero-initialized.
	///
	/// http://en.cppreference.com/w/cpp/memory/uninitialized_value_construct_n
	///
	template <class ForwardIterator, class Count>
	ForwardIterator uninitialized_value_construct_n(ForwardIterator first, Count n)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	/// uninitialized_fill_ptr
	///
	/// This is a specialization of uninitialized_fill for iterators that are pointers.
	/// It exists so that we can declare a value_type for the iterator, which you
	/// can't do with a pointer by itself.
	///
	template <typename T>
	EASTL_REMOVE_AT_2024_SEPT inline void uninitialized_fill_ptr(T* first, T* last, const T& value)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	/// uninitialized_fill_n
	///
	/// Copy-constructs the range of [first, first + n) with the given input value.
	/// Returns void as per the C++ standard, though returning the end input iterator
	/// value may be of use.
	///
	/// Declaration:
	///    template <typename ForwardIterator, typename Count, typename T>
	///    void uninitialized_fill_n(ForwardIterator destination, Count n, const T& value);
	///
	namespace Internal
	{
		template <typename ForwardIterator, typename Count, typename T>
		inline void uninitialized_fill_n_impl(ForwardIterator first, Count n, const T& value, true_type /* is_trivially_copy_assignable */)
		{
    __builtin_trap() /* STUB: not implemented */;
}

		template <typename ForwardIterator, typename Count, typename T>
		void uninitialized_fill_n_impl(ForwardIterator first, Count n, const T& value, false_type /* is_trivially_copy_assignable */)
		{
    __builtin_trap() /* STUB: not implemented */;
}
	}

	template <typename ForwardIterator, typename Count, typename T>
	inline void uninitialized_fill_n(ForwardIterator first, Count n, const T& value)
	{
    __builtin_trap() /* STUB: not implemented */;
}



	/// uninitialized_fill_n_ptr
	///
	/// This is a specialization of uninitialized_fill_n for iterators that are pointers.
	/// It exists so that we can declare a value_type for the iterator, which you
	/// can't do with a pointer by itself.
	///
	template <typename T, typename Count>
	EASTL_REMOVE_AT_2024_SEPT inline void uninitialized_fill_n_ptr(T* first, Count n, const T& value)
	{
    __builtin_trap() /* STUB: not implemented */;
}




	/// uninitialized_copy_fill
	///
	/// Copies [first1, last1) into [first2, first2 + (last1 - first1)) then
	/// fills [first2 + (last1 - first1), last2) with value.
	///
	template <typename InputIterator, typename ForwardIterator, typename T>
	inline void uninitialized_copy_fill(InputIterator first1, InputIterator last1,
										ForwardIterator first2, ForwardIterator last2, const T& value)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	/// uninitialized_move_fill
	///
	/// Moves [first1, last1) into [first2, first2 + (last1 - first1)) then
	/// fills [first2 + (last1 - first1), last2) with value.
	///
	template <typename InputIterator, typename ForwardIterator, typename T>
	inline void uninitialized_move_fill(InputIterator first1, InputIterator last1,
										ForwardIterator first2, ForwardIterator last2, const T& value)
	{
    __builtin_trap() /* STUB: not implemented */;
}





	/// uninitialized_fill_copy
	///
	/// Fills [result, mid) with value then copies [first, last) into [mid, mid + (last - first)).
	///
	template <typename ForwardIterator, typename T, typename InputIterator>
	inline ForwardIterator
	uninitialized_fill_copy(ForwardIterator result, ForwardIterator mid, const T& value, InputIterator first, InputIterator last)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	/// uninitialized_fill_move
	///
	/// Fills [result, mid) with value then copies [first, last) into [mid, mid + (last - first)).
	///
	template <typename ForwardIterator, typename T, typename InputIterator>
	inline ForwardIterator
	uninitialized_fill_move(ForwardIterator result, ForwardIterator mid, const T& value, InputIterator first, InputIterator last)
	{
    __builtin_trap() /* STUB: not implemented */;
}



	/// uninitialized_copy_copy
	///
	/// Copies [first1, last1) into [result, result + (last1 - first1)) then
	/// copies [first2, last2) into [result, result + (last1 - first1) + (last2 - first2)).
	///
	template <typename InputIterator1, typename InputIterator2, typename ForwardIterator>
	inline ForwardIterator
	uninitialized_copy_copy(InputIterator1 first1, InputIterator1 last1,
							InputIterator2 first2, InputIterator2 last2,
							ForwardIterator result)
	{
    __builtin_trap() /* STUB: not implemented */;
}



	/// destruct
	///
	/// Calls the destructor of a given object.
	///
	/// Note that we don't have a specialized version of this for objects
	/// with trivial destructors, such as integers. This is because the
	/// compiler can already see in our version here that the destructor
	/// is a no-op.
	///
	template <typename T>
	inline void destruct(T* p)
	{
    __builtin_trap() /* STUB: not implemented */;
}



	// destruct(first, last)
	//
	template <typename ForwardIterator>
	inline void destruct_impl(ForwardIterator /*first*/, ForwardIterator /*last*/, true_type) // true means the type has a trivial destructor.
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename ForwardIterator>
	inline void destruct_impl(ForwardIterator first, ForwardIterator last, false_type) // false means the type has a significant destructor.
	{
    __builtin_trap() /* STUB: not implemented */;
}

	/// destruct
	///
	/// Calls the destructor on a range of objects.
	///
	/// We have a specialization for objects with trivial destructors, such as
	/// PODs. In this specialization the destruction of the range is a no-op.
	///
	template <typename ForwardIterator>
	inline void destruct(ForwardIterator first, ForwardIterator last)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	/// destroy_at
	///
	/// Calls the destructor of a given object.
	///
	/// Note that we don't have a specialized version of this for objects
	/// with trivial destructors, such as integers. This is because the
	/// compiler can already see in our version here that the destructor
	/// is a no-op.
	///
	/// This is the same as eastl::destruct but we included for C++17 compliance.
	///
	/// http://en.cppreference.com/w/cpp/memory/destroy_at
	///
	template <typename T>
	inline void destroy_at(T* p)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	/// destroy
	///
	/// Calls the destructor on a range of objects.
	///
	/// http://en.cppreference.com/w/cpp/memory/destroy
	///
	template <typename ForwardIterator>
	inline void destroy(ForwardIterator first, ForwardIterator last)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	/// destroy_n
	///
	/// Calls the destructor on the n objects in the range.
	///
	/// http://en.cppreference.com/w/cpp/memory/destroy_n
	///
	template <typename ForwardIterator, typename Size>
	ForwardIterator destroy_n(ForwardIterator first, Size n)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	/// align
	///
	/// Same as C++11 std::align. http://en.cppreference.com/w/cpp/memory/align
	/// If it is possible to fit size bytes of storage aligned by alignment into the buffer pointed to by
	/// ptr with length space, the function updates ptr to point to the first possible address of such storage,
	/// decreases space by the number of bytes used for alignment, and returns the new ptr value. Otherwise,
	/// the function returns NULL and leaves ptr and space unmodified.
	///
	/// Example usage:
	///     char   buffer[512];
	///     size_t space = sizeof(buffer);
	///     void*  p  = buffer;
	///     void*  p1 = eastl::align(16,  3, p, space); p = (char*)p +  3; space -=  3;
	///     void*  p2 = eastl::align(32, 78, p, space); p = (char*)p + 78; space -= 78;
	///     void*  p3 = eastl::align(64,  9, p, space); p = (char*)p +  9; space -=  9;

	inline void* align(size_t alignment, size_t size, void*& ptr, size_t& space)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	/// align_advance
	///
	/// Same as align except ptr and space can be adjusted to reflect remaining space.
	/// Not present in the C++ Standard.
	/// Note that the example code here is similar to align but simpler.
	///
	/// Example usage:
	///     char   buffer[512];
	///     size_t space = sizeof(buffer);
	///     void*  p  = buffer;
	///     void*  p1 = eastl::align_advance(16,  3, p, space, &p, &space); // p is advanced and space reduced accordingly.
	///     void*  p2 = eastl::align_advance(32, 78, p, space, &p, &space);
	///     void*  p3 = eastl::align_advance(64,  9, p, space, &p, &space);
	///     void*  p4 = eastl::align_advance(16, 33, p, space);

	inline void* align_advance(size_t alignment, size_t size, void* ptr, size_t space, void** ptrAdvanced = NULL, size_t* spaceReduced = NULL)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	///////////////////////////////////////////////////////////////////////
	// pointer_traits
	//
	// C++11 Standard section 20.6.3
	// Provides information about a pointer type, mostly for the purpose
	// of handling the case where the pointer type isn't a built-in T* but
	// rather is a class that acts like a pointer.
	//
	// A user-defined Pointer has the following properties, by example:
	//     template <class T, class... MoreArgs>
	//     struct Pointer
	//     {
	//         typedef Pointer pointer;                         // required for use by pointer_traits.
	//         typedef T1      element_type;                    // optional for use by pointer_traits.
	//         typedef T2      difference_type;                 // optional for use by pointer_traits.
	//
	//         template <class Other>
	//         using rebind = typename Ptr<Other, MoreArgs...>; // optional for use by pointer_traits.
	//
	//         static pointer pointer_to(element_type& obj);    // required for use by pointer_traits.
	//     };
	//
	//
	// Example usage:
	//     template <typename Pointer>
	//     typename pointer_traits::element_type& GetElementPointedTo(Pointer p)
	//      { return *p; }
	//
	///////////////////////////////////////////////////////////////////////

	namespace Internal
	{
		// pointer_element_type
		template <typename Pointer>
		struct has_element_type // has_element_type<T>::value is true if T has an element_type member typedef.
		{
		private:
			template <typename U> static eastl::no_type  test(...);
			template <typename U> static eastl::yes_type test(typename U::element_type* = 0);
		public:
			static const bool value = sizeof(test<Pointer>(0)) == sizeof(eastl::yes_type);
		};

		template <typename Pointer, bool = has_element_type<Pointer>::value>
		struct pointer_element_type
		{
			using type = Pointer;
		};

		template <typename Pointer>
		struct pointer_element_type<Pointer, true>
			{ typedef typename Pointer::element_type type; };

		template <template <typename, typename...> class Pointer, typename T, typename... Args>
		struct pointer_element_type<Pointer<T, Args...>, false>
			{ typedef T type; };


		// pointer_difference_type
		template <typename Pointer>
		struct has_difference_type // has_difference_type<T>::value is true if T has an difference_type member typedef.
		{
		private:
			template <typename U> static eastl::no_type  test(...);
			template <typename U> static eastl::yes_type test(typename U::difference_type* = 0);
		public:
			static const bool value = sizeof((test<Pointer>(0))) == sizeof(eastl::yes_type);
		};

		template <typename Pointer, bool = has_difference_type<Pointer>::value>
		struct pointer_difference_type
			{ typedef typename Pointer::difference_type type; };

		template <typename Pointer>
		struct pointer_difference_type<Pointer, false>
			{ typedef ptrdiff_t type; };


		// pointer_rebind
		// The following isn't correct, as it is unilaterally requiring that Pointer typedef its
		// own rebind. We can fix this if needed to make it optional (in which case it would return
		// its own type), but we don't currently use rebind in EASTL (as we have a different allocator
		// system than the C++ Standard Library has) and this is currently moot.
		template <typename Pointer, typename U>
		struct pointer_rebind
		{
			typedef typename Pointer::template rebind<U> type;
		};


	} // namespace Internal


	template <typename Pointer>
	struct pointer_traits
	{
		typedef Pointer                                                   pointer;
		typedef typename Internal::pointer_element_type<pointer>::type    element_type;
		typedef typename Internal::pointer_difference_type<pointer>::type difference_type;

		#if defined(EA_COMPILER_NO_TEMPLATE_ALIASES)
			template <typename U>
			struct rebind { typedef typename Internal::pointer_rebind<pointer, U>::type other; };
		#else
			template <typename U>
			using rebind = typename Internal::pointer_rebind<pointer, U>::type;
		#endif

	public:
		static pointer pointer_to(typename eastl::conditional<eastl::is_void<element_type>::value, void, element_type>::type& r) // 20.6.3.2: if element_type is (possibly cv-qualified) void, the type of r is unspecified; otherwise, it is T&.
			{
    __builtin_trap() /* STUB: not implemented */;
} // The C++11 Standard requires that Pointer provides a static pointer_to function.
	};


	template <typename T>
	struct pointer_traits<T*>
	{
		typedef T*        pointer;
		typedef T         element_type;
		typedef ptrdiff_t difference_type;

		#if defined(EA_COMPILER_NO_TEMPLATE_ALIASES)
			template <typename U>
			struct rebind { typedef U* other; };
		#else
			template <typename U>
			using rebind = U*;
		#endif

	public:
		static pointer pointer_to(typename eastl::conditional<eastl::is_void<element_type>::value, void, element_type>::type& r) EA_NOEXCEPT
			{
    __builtin_trap() /* STUB: not implemented */;
} // 20.6.3.2: if element_type is (possibly cv-qualified) void, the type of r is unspecified; otherwise, it is T&.
	};

	///////////////////////////////////////////////////////////////////////
	// to_address
	//
	// Helper that call the customization point in pointer_traits<T>::to_address for retrieving the address of a pointer.
	// This is useful if you are using fancy-pointers.
	///////////////////////////////////////////////////////////////////////

	namespace Internal
	{
		template <class T>
		using detect_pointer_traits_to_address = decltype(eastl::pointer_traits<T>::to_address(eastl::declval<const T&>()));

		template <class T>
		using result_detect_pointer_traits_to_address = eastl::is_detected<detect_pointer_traits_to_address, T>;
	}

	template<class T>
	EA_CPP14_CONSTEXPR T* to_address(T* p) noexcept
	{
		static_assert(!eastl::is_function<T>::value, "Cannot call to_address with a function pointer. C++20 20.2.4.1 - Pointer conversion.");
		return p;
	}

	template <class Ptr, typename eastl::enable_if<Internal::result_detect_pointer_traits_to_address<Ptr>::value, int>::type = 0>
	EA_CPP14_CONSTEXPR auto to_address(const Ptr& ptr) noexcept -> decltype(eastl::pointer_traits<Ptr>::to_address(ptr))
	{
		return eastl::pointer_traits<Ptr>::to_address(ptr);
	}

	template <class Ptr, typename eastl::enable_if<!Internal::result_detect_pointer_traits_to_address<Ptr>::value, int>::type = 0>
	EA_CPP14_CONSTEXPR auto to_address(const Ptr& ptr) noexcept -> decltype(to_address(ptr.operator->()))
	{
		return to_address(ptr.operator->());
	}
} // namespace eastl


EA_RESTORE_VC_WARNING();


#endif // Header include guard
