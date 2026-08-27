#include <stdexcept>
#include <cstdlib>
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) Electronic Arts Inc. All rights reserved.
/////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// This file implements a string which uses a fixed size memory pool.
// The bEnableOverflow template parameter allows the container to resort to
// heap allocations if the memory pool is exhausted.
///////////////////////////////////////////////////////////////////////////////


#ifndef EASTL_FIXED_STRING_H
#define EASTL_FIXED_STRING_H

#include <EASTL/internal/config.h>
#include <EASTL/string.h>
#include <EASTL/internal/fixed_pool.h>

#if defined(EA_PRAGMA_ONCE_SUPPORTED)
	#pragma once // Some compilers (e.g. VC++) benefit significantly from using this. We've measured 3-4% build speed improvements in apps as a result.
#endif


namespace eastl
{
	/// EASTL_FIXED_STRING_DEFAULT_NAME
	///
	/// Defines a default container name in the absence of a user-provided name.
	/// In the case of fixed-size containers, the allocator name always refers
	/// to overflow allocations.
	///
	#ifndef EASTL_FIXED_STRING_DEFAULT_NAME
		#define EASTL_FIXED_STRING_DEFAULT_NAME EASTL_DEFAULT_NAME_PREFIX " fixed_string" // Unless the user overrides something, this is "EASTL fixed_string".
	#endif

	/// EASTL_FIXED_STRING_DEFAULT_ALLOCATOR
	///
	#ifndef EASTL_FIXED_STRING_DEFAULT_ALLOCATOR
		#define EASTL_FIXED_STRING_DEFAULT_ALLOCATOR overflow_allocator_type(EASTL_FIXED_STRING_DEFAULT_NAME)
	#endif


	/// fixed_string
	///
	/// A fixed_string with bEnableOverflow == true is identical to a regular
	/// string in terms of its behavior. All the expectations of regular string
	/// apply to it and no additional expectations come from it. When bEnableOverflow
	/// is false, fixed_string behaves like regular string with the exception that
	/// its capacity can never increase. All operations you do on such a fixed_string
	/// which require a capacity increase will result in undefined behavior or an
	/// C++ allocation exception, depending on the configuration of EASTL.
	///
	/// Note: The nodeCount value is the amount of characters to allocate, which needs to
	/// take into account a terminating zero. Thus if you want to store strings with a strlen
	/// of 30, the nodeCount value must be at least 31.
	///
	/// Template parameters:
	///     T                      The type of object the string holds (char, wchar_t, char8_t, char16_t, char32_t).
	///     nodeCount              The max number of objects to contain.
	///     bEnableOverflow        Whether or not we should use the overflow heap if our object pool is exhausted.
	///     OverflowAllocator              Overflow allocator, which is only used if bEnableOverflow == true. Defaults to the global heap.
	///
	/// Notes:
	///     The nodeCount value must be at least 2, one for a character and one for a terminating 0.
	///
	///     As of this writing, the string class necessarily reallocates when an insert of
	///     self is done into self. As a result, the fixed_string class doesn't support
	///     inserting self into self unless the bEnableOverflow template parameter is true.
	///
	/// Example usage:
	///    fixed_string<char, 128 + 1, true> fixedString("hello world"); // Can hold up to a strlen of 128.
	///
	///    fixedString = "hola mundo";
	///    fixedString.clear();
	///    fixedString.resize(200);
	///    fixedString.sprintf("%f", 1.5f);
	///
	template <typename T, int nodeCount, bool bEnableOverflow = true, typename OverflowAllocator = EASTLAllocatorType>
	class fixed_string : public basic_string<T, fixed_vector_allocator<sizeof(T), nodeCount, EASTL_ALIGN_OF(T), 0, bEnableOverflow, OverflowAllocator> >
	{
	public:
		typedef fixed_vector_allocator<sizeof(T), nodeCount, EASTL_ALIGN_OF(T),
							0, bEnableOverflow, OverflowAllocator>              fixed_allocator_type;
		typedef typename fixed_allocator_type::overflow_allocator_type          overflow_allocator_type;
		typedef basic_string<T, fixed_allocator_type>                           base_type;
		typedef fixed_string<T, nodeCount, bEnableOverflow, OverflowAllocator>  this_type;
		typedef typename base_type::size_type                                   size_type;
		typedef typename base_type::value_type                                  value_type;
		typedef typename base_type::CtorDoNotInitialize                         CtorDoNotInitialize;
		typedef typename base_type::CtorSprintf                                 CtorSprintf;
		typedef aligned_buffer<nodeCount * sizeof(T), EASTL_ALIGN_OF(T)>        aligned_buffer_type;

		enum { kMaxSize = nodeCount - 1 }; // -1 because we need to save one element for the silent terminating null.

		using base_type::npos;
		using base_type::append;
		using base_type::resize;
		using base_type::clear;
		using base_type::capacity;
		using base_type::size;
		using base_type::sprintf_va_list;
		using base_type::get_allocator;

	protected:
		using base_type::mPair;
		using base_type::DoAllocate;
		using base_type::DoFree;
		using base_type::internalLayout;

		union // We define a union in order to avoid strict pointer aliasing issues with compilers like GCC.
		{
			value_type          mArray[1];
			aligned_buffer_type mBuffer;     // Question: Why are we doing this aligned_buffer thing? Why not just do an array of value_type, given that we are using just strings of char types.
		};

	public:
		fixed_string();
		explicit fixed_string(const overflow_allocator_type& overflowAllocator);             // Only applicable if bEnableOverflow is true.
		fixed_string(const base_type& x, size_type position, size_type n = base_type::npos); // Currently we don't support overflowAllocator specification for other constructors, for simplicity.
		fixed_string(const value_type* p, size_type n);
		fixed_string(const value_type* p);
		fixed_string(size_type n, const value_type& value);
		fixed_string(const this_type& x);
		fixed_string(const this_type& x, const overflow_allocator_type& overflowAllocator);
		fixed_string(const base_type& x);
		fixed_string(const value_type* pBegin, const value_type* pEnd);
		fixed_string(CtorDoNotInitialize, size_type n);
		fixed_string(CtorSprintf, const value_type* pFormat, ...);
		fixed_string(std::initializer_list<T> ilist, const overflow_allocator_type& overflowAllocator = EASTL_FIXED_STRING_DEFAULT_ALLOCATOR);
		fixed_string(this_type&& x);
		fixed_string(this_type&& x, const overflow_allocator_type& overflowAllocator);

		this_type& operator=(const this_type& x);
		this_type& operator=(const base_type& x);
		this_type& operator=(const value_type* p);
		this_type& operator=(const value_type c);
		this_type& operator=(std::initializer_list<T> ilist);
		this_type& operator=(this_type&& x);

		void swap(this_type& x);

		void      set_capacity(size_type n);
		void      reset_lose_memory();          // This is a unilateral reset to an initially empty state. No destructors are called, no deallocation occurs.
		size_type max_size() const;
		bool      full() const;                 // Returns true if the fixed space has been fully allocated. Note that if overflow is enabled, the container size can be greater than nodeCount but full() could return true because the fixed space may have a recently freed slot.
		bool      has_overflowed() const;       // Returns true if the allocations spilled over into the overflow allocator. Meaningful only if overflow is enabled.
		static constexpr bool can_overflow() { return bEnableOverflow; } // Returns the value of the bEnableOverflow template parameter.

		// The inherited versions of substr/left/right call the basic_string constructor,
		// which will call the overflow allocator and fail if bEnableOverflow == false
		this_type substr(size_type position, size_type n) const;
		this_type left(size_type n) const;
		this_type right(size_type n) const;

		// OverflowAllocator
		const overflow_allocator_type& get_overflow_allocator() const EA_NOEXCEPT;
		overflow_allocator_type&       get_overflow_allocator() EA_NOEXCEPT;
		void                           set_overflow_allocator(const overflow_allocator_type& allocator);
	}; // fixed_string





	///////////////////////////////////////////////////////////////////////
	// fixed_string
	///////////////////////////////////////////////////////////////////////

	template <typename T, int nodeCount, bool bEnableOverflow, typename OverflowAllocator>
	inline fixed_string<T, nodeCount, bEnableOverflow, OverflowAllocator>::fixed_string()
		: base_type(fixed_allocator_type(mBuffer.buffer))
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, int nodeCount, bool bEnableOverflow, typename OverflowAllocator>
	inline fixed_string<T, nodeCount, bEnableOverflow, OverflowAllocator>::fixed_string(const overflow_allocator_type& overflowAllocator)
		: base_type(fixed_allocator_type(mBuffer.buffer, overflowAllocator))
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, int nodeCount, bool bEnableOverflow, typename OverflowAllocator>
	inline fixed_string<T, nodeCount, bEnableOverflow, OverflowAllocator>::fixed_string(const this_type& x)
		: base_type(fixed_allocator_type(mBuffer.buffer))
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, int nodeCount, bool bEnableOverflow, typename OverflowAllocator>
	inline fixed_string<T, nodeCount, bEnableOverflow, OverflowAllocator>::fixed_string(const this_type& x, const overflow_allocator_type& overflowAllocator)
		: base_type(fixed_allocator_type(mBuffer.buffer, overflowAllocator))
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, int nodeCount, bool bEnableOverflow, typename OverflowAllocator>
	inline fixed_string<T, nodeCount, bEnableOverflow, OverflowAllocator>::fixed_string(const base_type& x)
		: base_type(fixed_allocator_type(mBuffer.buffer))
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, int nodeCount, bool bEnableOverflow, typename OverflowAllocator>
	inline fixed_string<T, nodeCount, bEnableOverflow, OverflowAllocator>::fixed_string(const base_type& x, size_type position, size_type n)
		: base_type(fixed_allocator_type(mBuffer.buffer))
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, int nodeCount, bool bEnableOverflow, typename OverflowAllocator>
	inline fixed_string<T, nodeCount, bEnableOverflow, OverflowAllocator>::fixed_string(const value_type* p, size_type n)
		: base_type(fixed_allocator_type(mBuffer.buffer))
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, int nodeCount, bool bEnableOverflow, typename OverflowAllocator>
	inline fixed_string<T, nodeCount, bEnableOverflow, OverflowAllocator>::fixed_string(const value_type* p)
		: base_type(fixed_allocator_type(mBuffer.buffer))
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, int nodeCount, bool bEnableOverflow, typename OverflowAllocator>
	inline fixed_string<T, nodeCount, bEnableOverflow, OverflowAllocator>::fixed_string(size_type n, const value_type& value)
		: base_type(fixed_allocator_type(mBuffer.buffer))
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, int nodeCount, bool bEnableOverflow, typename OverflowAllocator>
	inline fixed_string<T, nodeCount, bEnableOverflow, OverflowAllocator>::fixed_string(const value_type* pBegin, const value_type* pEnd)
		: base_type(fixed_allocator_type(mBuffer.buffer))
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, int nodeCount, bool bEnableOverflow, typename OverflowAllocator>
	inline fixed_string<T, nodeCount, bEnableOverflow, OverflowAllocator>::fixed_string(CtorDoNotInitialize, size_type n)
		: base_type(fixed_allocator_type(mBuffer.buffer))
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, int nodeCount, bool bEnableOverflow, typename OverflowAllocator>
	inline fixed_string<T, nodeCount, bEnableOverflow, OverflowAllocator>::fixed_string(CtorSprintf, const value_type* pFormat, ...)
		: base_type(fixed_allocator_type(mBuffer.buffer))
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, int nodeCount, bool bEnableOverflow, typename OverflowAllocator>
	inline fixed_string<T, nodeCount, bEnableOverflow, OverflowAllocator>::fixed_string(std::initializer_list<T> ilist, const overflow_allocator_type& overflowAllocator)
		: base_type(fixed_allocator_type(mBuffer.buffer, overflowAllocator))
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, int nodeCount, bool bEnableOverflow, typename OverflowAllocator>
	inline fixed_string<T, nodeCount, bEnableOverflow, OverflowAllocator>::fixed_string(this_type&& x)
		: base_type(fixed_allocator_type(mBuffer.buffer))
	{
		// We copy from x instead of trade with it. We need to do so because fixed_ containers use local memory buffers.
		#if EASTL_NAME_ENABLED
			get_allocator().set_name(x.get_allocator().get_name());
		#endif

		internalLayout().SetHeapBeginPtr(mArray);
		internalLayout().SetHeapCapacity(nodeCount - 1);
		internalLayout().SetHeapSize(0);

		*internalLayout().HeapBeginPtr() = 0;

		append(x); // Let x destruct its own items.
	}

	template <typename T, int nodeCount, bool bEnableOverflow, typename OverflowAllocator>
	inline fixed_string<T, nodeCount, bEnableOverflow, OverflowAllocator>::fixed_string(this_type&& x, const overflow_allocator_type& overflowAllocator)
		: base_type(fixed_allocator_type(mBuffer.buffer, overflowAllocator))
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, int nodeCount, bool bEnableOverflow, typename OverflowAllocator>
	inline typename fixed_string<T, nodeCount, bEnableOverflow, OverflowAllocator>::this_type&
	fixed_string<T, nodeCount, bEnableOverflow, OverflowAllocator>::operator=(const this_type& x)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, int nodeCount, bool bEnableOverflow, typename OverflowAllocator>
	inline typename fixed_string<T, nodeCount, bEnableOverflow, OverflowAllocator>::
	this_type& fixed_string<T, nodeCount, bEnableOverflow, OverflowAllocator>::operator=(const base_type& x)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, int nodeCount, bool bEnableOverflow, typename OverflowAllocator>
	inline typename fixed_string<T, nodeCount, bEnableOverflow, OverflowAllocator>::
	this_type& fixed_string<T, nodeCount, bEnableOverflow, OverflowAllocator>::operator=(const value_type* p)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, int nodeCount, bool bEnableOverflow, typename OverflowAllocator>
	inline typename fixed_string<T, nodeCount, bEnableOverflow, OverflowAllocator>::
	this_type& fixed_string<T, nodeCount, bEnableOverflow, OverflowAllocator>::operator=(const value_type c)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, int nodeCount, bool bEnableOverflow, typename OverflowAllocator>
	inline typename fixed_string<T, nodeCount, bEnableOverflow, OverflowAllocator>::
	this_type& fixed_string<T, nodeCount, bEnableOverflow, OverflowAllocator>::operator=(std::initializer_list<T> ilist)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, int nodeCount, bool bEnableOverflow, typename OverflowAllocator>
	inline typename fixed_string<T, nodeCount, bEnableOverflow, OverflowAllocator>::
	this_type& fixed_string<T, nodeCount, bEnableOverflow, OverflowAllocator>::operator=(this_type&& x)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, int nodeCount, bool bEnableOverflow, typename OverflowAllocator>
	inline void fixed_string<T, nodeCount, bEnableOverflow, OverflowAllocator>::swap(this_type& x)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, int nodeCount, bool bEnableOverflow, typename OverflowAllocator>
	inline void fixed_string<T, nodeCount, bEnableOverflow, OverflowAllocator>::set_capacity(size_type n)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, int nodeCount, bool bEnableOverflow, typename OverflowAllocator>
	inline void fixed_string<T, nodeCount, bEnableOverflow, OverflowAllocator>::reset_lose_memory()
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, int nodeCount, bool bEnableOverflow, typename OverflowAllocator>
	inline typename fixed_string<T, nodeCount, bEnableOverflow, OverflowAllocator>::
	size_type fixed_string<T, nodeCount, bEnableOverflow, OverflowAllocator>::max_size() const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, int nodeCount, bool bEnableOverflow, typename OverflowAllocator>
	inline bool fixed_string<T, nodeCount, bEnableOverflow, OverflowAllocator>::full() const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, int nodeCount, bool bEnableOverflow, typename OverflowAllocator>
	inline bool fixed_string<T, nodeCount, bEnableOverflow, OverflowAllocator>::has_overflowed() const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, int nodeCount, bool bEnableOverflow, typename OverflowAllocator>
	inline typename fixed_string<T, nodeCount, bEnableOverflow, OverflowAllocator>::
	this_type fixed_string<T, nodeCount, bEnableOverflow, OverflowAllocator>::substr(size_type position, size_type n) const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, int nodeCount, bool bEnableOverflow, typename OverflowAllocator>
	inline typename fixed_string<T, nodeCount, bEnableOverflow, OverflowAllocator>::
	this_type fixed_string<T, nodeCount, bEnableOverflow, OverflowAllocator>::left(size_type n) const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, int nodeCount, bool bEnableOverflow, typename OverflowAllocator>
	inline typename fixed_string<T, nodeCount, bEnableOverflow, OverflowAllocator>::
	this_type fixed_string<T, nodeCount, bEnableOverflow, OverflowAllocator>::right(size_type n) const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, int nodeCount, bool bEnableOverflow, typename OverflowAllocator>
	inline const typename fixed_string<T, nodeCount, bEnableOverflow, OverflowAllocator>::
	overflow_allocator_type& fixed_string<T, nodeCount, bEnableOverflow, OverflowAllocator>::get_overflow_allocator() const EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, int nodeCount, bool bEnableOverflow, typename OverflowAllocator>
	inline typename fixed_string<T, nodeCount, bEnableOverflow, OverflowAllocator>::
	overflow_allocator_type& fixed_string<T, nodeCount, bEnableOverflow, OverflowAllocator>::get_overflow_allocator() EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, int nodeCount, bool bEnableOverflow, typename OverflowAllocator>
	inline void
	fixed_string<T, nodeCount, bEnableOverflow, OverflowAllocator>::set_overflow_allocator(const overflow_allocator_type& allocator)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <class T>
	inline size_t hash_string(const T* p)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T, int nodeCount, bool bEnableOverflow, typename OverflowAllocator>
	struct hash<fixed_string<T, nodeCount, bEnableOverflow, OverflowAllocator>>
	{
		size_t operator()(const fixed_string<T, nodeCount, bEnableOverflow, OverflowAllocator>& x) const
		{
    __builtin_trap() /* STUB: not implemented */;
}
	};


	///////////////////////////////////////////////////////////////////////
	// global operators
	///////////////////////////////////////////////////////////////////////


	// Operator +
	template <typename T, int nodeCount, bool bEnableOverflow, typename OverflowAllocator>
	fixed_string<T, nodeCount, bEnableOverflow, OverflowAllocator> operator+(const fixed_string<T, nodeCount, bEnableOverflow, OverflowAllocator>& a,
																			 const fixed_string<T, nodeCount, bEnableOverflow, OverflowAllocator>& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, int nodeCount, bool bEnableOverflow, typename OverflowAllocator>
	fixed_string<T, nodeCount, bEnableOverflow, OverflowAllocator> operator+(const typename fixed_string<T, nodeCount, bEnableOverflow, OverflowAllocator>::value_type* p,
																			 const fixed_string<T, nodeCount, bEnableOverflow, OverflowAllocator>& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, int nodeCount, bool bEnableOverflow, typename OverflowAllocator>
	fixed_string<T, nodeCount, bEnableOverflow, OverflowAllocator> operator+(typename fixed_string<T, nodeCount, bEnableOverflow, OverflowAllocator>::value_type c,
																				const fixed_string<T, nodeCount, bEnableOverflow, OverflowAllocator>& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, int nodeCount, bool bEnableOverflow, typename OverflowAllocator>
	fixed_string<T, nodeCount, bEnableOverflow, OverflowAllocator> operator+(const fixed_string<T, nodeCount, bEnableOverflow, OverflowAllocator>& a,
																	const typename fixed_string<T, nodeCount, bEnableOverflow, OverflowAllocator>::value_type* p)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, int nodeCount, bool bEnableOverflow, typename OverflowAllocator>
	fixed_string<T, nodeCount, bEnableOverflow, OverflowAllocator> operator+(const fixed_string<T, nodeCount, bEnableOverflow, OverflowAllocator>& a,
																		  typename fixed_string<T, nodeCount, bEnableOverflow, OverflowAllocator>::value_type c)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T, int nodeCount, bool bEnableOverflow, typename OverflowAllocator>
	fixed_string<T, nodeCount, bEnableOverflow, OverflowAllocator> operator+(fixed_string<T, nodeCount, bEnableOverflow, OverflowAllocator>&& a,
																			 fixed_string<T, nodeCount, bEnableOverflow, OverflowAllocator>&& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T, int nodeCount, bool bEnableOverflow, typename OverflowAllocator>
	fixed_string<T, nodeCount, bEnableOverflow, OverflowAllocator> operator+(fixed_string<T, nodeCount, bEnableOverflow, OverflowAllocator>&& a,
																	   const fixed_string<T, nodeCount, bEnableOverflow, OverflowAllocator>& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T, int nodeCount, bool bEnableOverflow, typename OverflowAllocator>
	fixed_string<T, nodeCount, bEnableOverflow, OverflowAllocator> operator+(const typename fixed_string<T, nodeCount, bEnableOverflow, OverflowAllocator>::value_type* p,
																							fixed_string<T, nodeCount, bEnableOverflow, OverflowAllocator>&& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T, int nodeCount, bool bEnableOverflow, typename OverflowAllocator>
	fixed_string<T, nodeCount, bEnableOverflow, OverflowAllocator> operator+(fixed_string<T, nodeCount, bEnableOverflow, OverflowAllocator>&& a,
															  const typename fixed_string<T, nodeCount, bEnableOverflow, OverflowAllocator>::value_type* p)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T, int nodeCount, bool bEnableOverflow, typename OverflowAllocator>
	fixed_string<T, nodeCount, bEnableOverflow, OverflowAllocator> operator+(fixed_string<T, nodeCount, bEnableOverflow, OverflowAllocator>&& a,
																	typename fixed_string<T, nodeCount, bEnableOverflow, OverflowAllocator>::value_type c)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	// operator ==, !=, <, >, <=, >= come from the string implementations.

	template <typename T, int nodeCount, bool bEnableOverflow, typename OverflowAllocator>
	inline void swap(fixed_string<T, nodeCount, bEnableOverflow, OverflowAllocator>& a,
					 fixed_string<T, nodeCount, bEnableOverflow, OverflowAllocator>& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}


} // namespace eastl

#endif // Header include guard
