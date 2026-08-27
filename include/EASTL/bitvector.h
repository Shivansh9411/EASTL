#include <cstdlib>
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) Electronic Arts Inc. All rights reserved.
/////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Implements a bit vector, which is essentially a vector of bool but which
// uses bits instead of bytes. It is thus similar to the original std::vector<bool>.
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Note: This code is not yet complete: it isn't tested and doesn't yet 
//       support containers other than vector.
///////////////////////////////////////////////////////////////////////////////


#ifndef EASTL_BITVECTOR_H
#define EASTL_BITVECTOR_H


#include <EASTL/internal/config.h>
#include <EASTL/vector.h>
#include <EASTL/fixed_vector.h>
#include <EASTL/algorithm.h>
#include <EASTL/bitset.h>
#if EASTL_EXCEPTIONS_ENABLED
#include <stdexcept>
#endif

EA_DISABLE_VC_WARNING(4480); // nonstandard extension used: specifying underlying type for enum

#if defined(EA_PRAGMA_ONCE_SUPPORTED)
	#pragma once // Some compilers (e.g. VC++) benefit significantly from using this. We've measured 3-4% build speed improvements in apps as a result.
#endif



namespace eastl
{

	/// EASTL_BITVECTOR_DEFAULT_NAME
	///
	/// Defines a default container name in the absence of a user-provided name.
	///
	#ifndef EASTL_BITVECTOR_DEFAULT_NAME
		#define EASTL_BITVECTOR_DEFAULT_NAME EASTL_DEFAULT_NAME_PREFIX " bitvector" // Unless the user overrides something, this is "EASTL bitvector".
	#endif

	/// EASTL_BITVECTOR_DEFAULT_ALLOCATOR
	///
	#ifndef EASTL_BITVECTOR_DEFAULT_ALLOCATOR
		#define EASTL_BITVECTOR_DEFAULT_ALLOCATOR allocator_type(EASTL_BITVECTOR_DEFAULT_NAME)
	#endif



	/// BitvectorWordType
	/// Defines the integral data type used by bitvector.
	typedef EASTL_BITSET_WORD_TYPE_DEFAULT BitvectorWordType;


	template <typename Element>
	class bitvector_const_iterator;


	template <typename Element>
	class bitvector_reference
	{
	public:
		typedef eastl_size_t size_type;
		bitvector_reference(Element* ptr, eastl_size_t i);
		bitvector_reference(const bitvector_reference& other);

		bitvector_reference& operator=(bool value);
		bitvector_reference& operator=(const bitvector_reference& rhs);

		operator bool() const // Defined here because some compilers fail otherwise.
			{
    __builtin_trap() /* STUB: not implemented */;
}

	protected:
		friend class bitvector_const_iterator<Element>;

		Element*  mpBitWord;
		size_type mnBitIndex;

		bitvector_reference() {
    __builtin_trap() /* STUB: not implemented */;
}
		void CopyFrom(const bitvector_reference& rhs);
	};



	template <typename Element>
	class bitvector_const_iterator
	{
	public:
		typedef eastl::random_access_iterator_tag iterator_category;
		typedef bitvector_const_iterator<Element>        this_type;
		typedef bool                                     value_type;
		typedef bitvector_reference<Element>             reference_type;
		typedef ptrdiff_t                                difference_type;
		typedef Element                                  element_type;
		typedef element_type*                            pointer;           // This is wrong. It needs to be someting that acts as a pointer to a bit.
		typedef element_type&                            reference;         // This is not right. It needs to be someting that acts as a pointer to a bit.
		typedef eastl_size_t                             size_type;

	protected:
		reference_type mReference;

		enum
		{
			kBitCount = (8 * sizeof(Element))
		};

	public:
		bool operator*() const;
		bool operator[](difference_type n) const;

		bitvector_const_iterator();
		bitvector_const_iterator(const element_type* p, eastl_size_t i);
		bitvector_const_iterator(const reference_type& referenceType);
		bitvector_const_iterator(const bitvector_const_iterator& other);

		bitvector_const_iterator& operator++();
		bitvector_const_iterator  operator++(int);
		bitvector_const_iterator& operator--();
		bitvector_const_iterator  operator--(int);

		bitvector_const_iterator& operator+=(difference_type dist);
		bitvector_const_iterator& operator-=(difference_type dist);
		bitvector_const_iterator  operator+ (difference_type dist) const;
		bitvector_const_iterator  operator- (difference_type dist) const;

		difference_type operator-(const this_type& rhs) const;

		bitvector_const_iterator& operator= (const this_type& rhs);

		bool operator==(const this_type& rhs) const;
		bool operator!=(const this_type& rhs) const;

		bool operator< (const this_type& rhs) const;
		bool operator<=(const this_type& rhs) const;
		bool operator> (const this_type& rhs) const;
		bool operator>=(const this_type& rhs) const;

		int validate(const element_type* pStart, const element_type* pEnd, eastl_size_t nExtraBits) const;

	protected:
		template <typename, typename, typename>
		friend class bitvector;

		reference_type& get_reference_type() {
    __builtin_trap() /* STUB: not implemented */;
}
	};



	template <typename Element>
	class bitvector_iterator : public bitvector_const_iterator<Element>
	{
	public:
		typedef eastl::random_access_iterator_tag iterator_category;
		typedef bitvector_iterator                       this_type;
		typedef bitvector_const_iterator<Element>        base_type;
		typedef bool                                     value_type;
		typedef bitvector_reference<Element>             reference_type;
		typedef ptrdiff_t                                difference_type;
		typedef Element                                  element_type;
		typedef element_type*                            pointer;           // This is wrong. It needs to be someting that acts as a pointer to a bit.
		typedef element_type&                            reference;         // This is not right. It needs to be someting that acts as a pointer to a bit.

	public:
		reference_type operator*() const;
		reference_type operator[](difference_type n) const;

		bitvector_iterator();
		bitvector_iterator(element_type* p, eastl_size_t i);
		bitvector_iterator(reference_type& referenceType);

		bitvector_iterator& operator++()    {
    __builtin_trap() /* STUB: not implemented */;
}
		bitvector_iterator& operator--()    {
    __builtin_trap() /* STUB: not implemented */;
}
		bitvector_iterator  operator++(int);
		bitvector_iterator  operator--(int);

		bitvector_iterator& operator+=(difference_type dist) {
    __builtin_trap() /* STUB: not implemented */;
}
		bitvector_iterator& operator-=(difference_type dist) {
    __builtin_trap() /* STUB: not implemented */;
}
		bitvector_iterator  operator+ (difference_type dist) const;
		bitvector_iterator  operator- (difference_type dist) const;

		// We need this here because we are overloading operator-, so for some reason the
		// other overload of the function can't be found unless it's explicitly specified.
		difference_type operator-(const base_type& rhs) const {
    __builtin_trap() /* STUB: not implemented */;
}
	};



	/// bitvector
	///
	/// Implements an array of bits treated as boolean values.
	/// bitvector is similar to vector<bool> but uses bits instead of bytes and 
	/// allows the user to use other containers such as deque instead of vector.
	/// bitvector is different from bitset in that bitset is less flexible but
	/// uses less memory and has higher performance.
	///
	/// To consider: Rename the Element template parameter to WordType, for 
	/// consistency with bitset.
	///
	template <typename Allocator = EASTLAllocatorType, 
			  typename Element   = BitvectorWordType, 
			  typename Container = eastl::vector<Element, Allocator> >
	class bitvector
	{
	public:
		typedef bitvector<Allocator, Element, Container>    this_type;
		typedef bool                                        value_type;
		typedef bitvector_reference<Element>                reference;
		typedef bool                                        const_reference;
		typedef bitvector_iterator<Element>                 iterator;
		typedef bitvector_const_iterator<Element>           const_iterator;
		typedef eastl::reverse_iterator<iterator>           reverse_iterator;
		typedef eastl::reverse_iterator<const_iterator>     const_reverse_iterator;
		typedef Allocator                                   allocator_type;
		typedef Element                                     element_type;
		typedef Container                                   container_type;
		typedef eastl_size_t                                size_type;
		typedef ptrdiff_t                                   difference_type;

		#if defined(_MSC_VER) && (_MSC_VER >= 1400) && (_MSC_VER <= 1600) && !EASTL_STD_CPP_ONLY  // _MSC_VER of 1400 means VS2005, 1600 means VS2010. VS2012 generates errors with usage of enum:size_type.
			enum : size_type {                      // Use Microsoft enum language extension, allowing for smaller debug symbols than using a static const. Users have been affected by this.
				npos     = container_type::npos,
				kMaxSize = container_type::kMaxSize
			};
		#else
			static const size_type npos     = container_type::npos;      /// 'npos' means non-valid position or simply non-position.
			static const size_type kMaxSize = container_type::kMaxSize;  /// -1 is reserved for 'npos'. It also happens to be slightly beneficial that kMaxSize is a value less than -1, as it helps us deal with potential integer wraparound issues.
		#endif

		enum
		{
			kBitCount = 8 * sizeof(Element)
		};

	protected:
		container_type mContainer;
		size_type      mFreeBitCount;      // Unused bits in the last word of mContainer.

	public:
		bitvector();
		explicit bitvector(const allocator_type& allocator);
		explicit bitvector(size_type n, const allocator_type& allocator = EASTL_BITVECTOR_DEFAULT_ALLOCATOR);
		bitvector(size_type n, value_type value, const allocator_type& allocator = EASTL_BITVECTOR_DEFAULT_ALLOCATOR);

		template <typename InputIterator>
		bitvector(InputIterator first, InputIterator last);

		void swap(this_type& x);

		template <typename InputIterator>
		void assign(InputIterator first, InputIterator last);

		iterator       begin() EA_NOEXCEPT;
		const_iterator begin() const EA_NOEXCEPT;
		const_iterator cbegin() const EA_NOEXCEPT;

		iterator       end() EA_NOEXCEPT;
		const_iterator end() const EA_NOEXCEPT;
		const_iterator cend() const EA_NOEXCEPT;

		reverse_iterator       rbegin() EA_NOEXCEPT;
		const_reverse_iterator rbegin() const EA_NOEXCEPT;
		const_reverse_iterator crbegin() const EA_NOEXCEPT;

		reverse_iterator       rend() EA_NOEXCEPT;
		const_reverse_iterator rend() const EA_NOEXCEPT;
		const_reverse_iterator crend() const EA_NOEXCEPT;

		bool      empty() const EA_NOEXCEPT;
		size_type size() const EA_NOEXCEPT;
		size_type capacity() const EA_NOEXCEPT;

		void resize(size_type n, value_type value);
		void resize(size_type n);
		void reserve(size_type n);
		void set_capacity(size_type n = npos);                  // Revises the capacity to the user-specified value. Resizes the container to match the capacity if the requested capacity n is less than the current size. If n == npos then the capacity is reallocated (if necessary) such that capacity == size.

		void push_back();
		void push_back(value_type value);
		void pop_back();

		reference       front();
		const_reference front() const;
		reference       back();
		const_reference back() const;

		bool            test(size_type n, bool defaultValue) const; // Returns true if the bit index is < size() and set. Returns defaultValue if the bit is >= size().
		void            set(size_type n, bool value);               // Resizes the container to accomodate n if necessary. 

		reference       at(size_type n);                    // throws an out_of_range exception if n is invalid.
		const_reference at(size_type n) const;

		reference       operator[](size_type n);            // behavior is undefined if n is invalid.
		const_reference operator[](size_type n) const;

		/*
		Work in progress:
		template <bool value = true> iterator find_first();                                 // Finds the lowest "on" bit.
		template <bool value = true> iterator find_next(const_iterator it);                 // Finds the next lowest "on" bit after it.
		template <bool value = true> iterator find_last();                                  // Finds the index of the last "on" bit, returns size if none are set.
		template <bool value = true> iterator find_prev(const_iterator it);                 // Finds the index of the last "on" bit before last_find, returns size if none are set.

		template <bool value = true> const_iterator find_first() const;                     // Finds the lowest "on" bit.
		template <bool value = true> const_iterator find_next(const_iterator it) const;     // Finds the next lowest "on" bit after it.
		template <bool value = true> const_iterator find_last() const;                      // Finds the index of the last "on" bit, returns size if none are set.
		template <bool value = true> const_iterator find_prev(const_iterator it) const;     // Finds the index of the last "on" bit before last_find, returns size if none are set.
		*/

		element_type*       data() EA_NOEXCEPT;
		const element_type* data() const EA_NOEXCEPT;
		
		iterator insert(const_iterator position, value_type value);
		void     insert(const_iterator position, size_type n, value_type value);

		// template <typename InputIterator> Not yet implemented. See below for disabled definition.
		// void insert(const_iterator position, InputIterator first, InputIterator last);

		iterator erase(const_iterator position);
		iterator erase(const_iterator first, const_iterator last);

		reverse_iterator erase(const_reverse_iterator position);
		reverse_iterator erase(const_reverse_iterator first, const_reverse_iterator last);

		void clear();
		void reset_lose_memory(); // This is a unilateral reset to an initially empty state. No destructors are called, no deallocation occurs.

		container_type&       get_container();
		const container_type& get_container() const;

		bool validate() const;
		int  validate_iterator(const_iterator i) const;

		bool any() const;
		bool all() const;
	};




	///////////////////////////////////////////////////////////////////////
	// bitvector_reference
	///////////////////////////////////////////////////////////////////////

	template <typename Element>
	bitvector_reference<Element>::bitvector_reference(Element* p, eastl_size_t i)
	  : mpBitWord(p), 
		mnBitIndex(i)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Element>
	bitvector_reference<Element>::bitvector_reference(const bitvector_reference& other)
	  : mpBitWord(other.mpBitWord), 
		mnBitIndex(other.mnBitIndex)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Element>
	bitvector_reference<Element>&
	bitvector_reference<Element>::operator=(bool value)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Element>
	bitvector_reference<Element>&
	bitvector_reference<Element>::operator=(const bitvector_reference& rhs)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Element>
	void bitvector_reference<Element>::CopyFrom(const bitvector_reference& rhs)
	{
    __builtin_trap() /* STUB: not implemented */;
}




	///////////////////////////////////////////////////////////////////////
	// bitvector_const_iterator
	///////////////////////////////////////////////////////////////////////

	template <typename Element>
	bitvector_const_iterator<Element>::bitvector_const_iterator()
		: mReference(0, 0)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Element>
	bitvector_const_iterator<Element>::bitvector_const_iterator(const Element* p, eastl_size_t i)
		: mReference(const_cast<Element*>(p), i) // const_cast is safe here because we never let mReference leak and we don't modify it.
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Element>
	bitvector_const_iterator<Element>::bitvector_const_iterator(const reference_type& reference)
		: mReference(reference)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Element>
	bitvector_const_iterator<Element>::bitvector_const_iterator(const bitvector_const_iterator& other)
		: mReference(other.mReference)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Element>
	bitvector_const_iterator<Element>&
	bitvector_const_iterator<Element>::operator++()
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Element>
	bitvector_const_iterator<Element>&
	bitvector_const_iterator<Element>::operator--()
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Element>
	bitvector_const_iterator<Element>
	bitvector_const_iterator<Element>::operator++(int)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Element>
	bitvector_const_iterator<Element>
	bitvector_const_iterator<Element>::operator--(int)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Element>
	bitvector_const_iterator<Element>&
	bitvector_const_iterator<Element>::operator+=(difference_type n)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Element>
	bitvector_const_iterator<Element>&
	bitvector_const_iterator<Element>::operator-=(difference_type n)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Element>
	bitvector_const_iterator<Element>
	bitvector_const_iterator<Element>::operator+(difference_type n) const
	{
    __builtin_trap() /* STUB: not implemented */;
}

	
	template <typename Element>
	bitvector_const_iterator<Element>
	bitvector_const_iterator<Element>::operator-(difference_type n) const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Element>
	typename bitvector_const_iterator<Element>::difference_type
	bitvector_const_iterator<Element>::operator-(const this_type& rhs) const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Element>
	bool bitvector_const_iterator<Element>::operator==(const this_type& rhs) const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Element>
	bool bitvector_const_iterator<Element>::operator!=(const this_type& rhs) const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Element>
	bool bitvector_const_iterator<Element>::operator<(const this_type& rhs) const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Element>
	bool bitvector_const_iterator<Element>::operator<=(const this_type& rhs) const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Element>
	bool bitvector_const_iterator<Element>::operator>(const this_type& rhs) const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Element>
	bool bitvector_const_iterator<Element>::operator>=(const this_type& rhs) const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Element>
	bool bitvector_const_iterator<Element>::operator*() const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Element>
	bool bitvector_const_iterator<Element>::operator[](difference_type n) const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Element>
	bitvector_const_iterator<Element>& bitvector_const_iterator<Element>::operator= (const this_type& rhs)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Element>
	int bitvector_const_iterator<Element>::validate(const Element* pStart, const Element* pEnd, eastl_size_t nExtraBits) const
	{
    __builtin_trap() /* STUB: not implemented */;
}



	///////////////////////////////////////////////////////////////////////
	// bitvector_iterator
	///////////////////////////////////////////////////////////////////////

	template <typename Element>
	bitvector_iterator<Element>::bitvector_iterator()
		: base_type()
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename Element>
	bitvector_iterator<Element>::bitvector_iterator(Element* p, eastl_size_t i)
		: base_type(p, i)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Element>
	bitvector_iterator<Element>::bitvector_iterator(reference_type& reference)
		: base_type(reference)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Element>
	typename bitvector_iterator<Element>::reference_type
	bitvector_iterator<Element>::operator*() const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Element>
	typename bitvector_iterator<Element>::reference_type
	bitvector_iterator<Element>::operator[](difference_type n) const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Element>
	void MoveBits(bitvector_iterator<Element> start, 
				  bitvector_iterator<Element> end, 
				  bitvector_iterator<Element> dest)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Element>
	bitvector_iterator<Element>
	bitvector_iterator<Element>::operator++(int)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Element>
	bitvector_iterator<Element>
	bitvector_iterator<Element>::operator--(int)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Element>
	bitvector_iterator<Element>
	bitvector_iterator<Element>::operator+(difference_type n) const
	{
    __builtin_trap() /* STUB: not implemented */;
}

	
	template <typename Element>
	bitvector_iterator<Element>
	bitvector_iterator<Element>::operator-(difference_type n) const
	{
    __builtin_trap() /* STUB: not implemented */;
}




	///////////////////////////////////////////////////////////////////////
	// bitvector
	///////////////////////////////////////////////////////////////////////

	template <typename Allocator, typename Element, typename Container>
	template <typename InputIterator>
	void bitvector<Allocator, Element, Container>::assign(InputIterator first, InputIterator last)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Allocator, typename Element, typename Container>
	typename bitvector<Allocator, Element, Container>::iterator
	bitvector<Allocator, Element, Container>::begin() EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Allocator, typename Element, typename Container>
	typename bitvector<Allocator, Element, Container>::const_iterator
	bitvector<Allocator, Element, Container>::begin() const EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Allocator, typename Element, typename Container>
	typename bitvector<Allocator, Element, Container>::const_iterator
	bitvector<Allocator, Element, Container>::cbegin() const EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Allocator, typename Element, typename Container>
	typename bitvector<Allocator, Element, Container>::iterator
	bitvector<Allocator, Element, Container>::end() EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Allocator, typename Element, typename Container>
	typename bitvector<Allocator, Element, Container>::const_iterator
	bitvector<Allocator, Element, Container>::end() const EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Allocator, typename Element, typename Container>
	typename bitvector<Allocator, Element, Container>::const_iterator
	bitvector<Allocator, Element, Container>::cend() const EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Allocator, typename Element, typename Container>
	bool bitvector<Allocator, Element, Container>::empty() const EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Allocator, typename Element, typename Container>
	typename bitvector<Allocator, Element, Container>::size_type
	bitvector<Allocator, Element, Container>::size() const EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Allocator, typename Element, typename Container>
	typename bitvector<Allocator, Element, Container>::size_type
	bitvector<Allocator, Element, Container>::capacity() const EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Allocator, typename Element, typename Container>
	void bitvector<Allocator, Element, Container>::set_capacity(size_type n)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Allocator, typename Element, typename Container>
	typename bitvector<Allocator, Element, Container>::reverse_iterator
	bitvector<Allocator, Element, Container>::rbegin() EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Allocator, typename Element, typename Container>
	typename bitvector<Allocator, Element, Container>::const_reverse_iterator
	bitvector<Allocator, Element, Container>::rbegin() const EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Allocator, typename Element, typename Container>
	typename bitvector<Allocator, Element, Container>::const_reverse_iterator
	bitvector<Allocator, Element, Container>::crbegin() const EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Allocator, typename Element, typename Container>
	typename bitvector<Allocator, Element, Container>::reverse_iterator
	bitvector<Allocator, Element, Container>::rend() EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Allocator, typename Element, typename Container>
	typename bitvector<Allocator, Element, Container>::const_reverse_iterator
	bitvector<Allocator, Element, Container>::rend() const EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Allocator, typename Element, typename Container>
	typename bitvector<Allocator, Element, Container>::const_reverse_iterator
	bitvector<Allocator, Element, Container>::crend() const EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Allocator, typename Element, typename Container>
	typename bitvector<Allocator, Element, Container>::reference
	bitvector<Allocator, Element, Container>::front()
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Allocator, typename Element, typename Container>
	typename bitvector<Allocator, Element, Container>::const_reference
	bitvector<Allocator, Element, Container>::front() const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Allocator, typename Element, typename Container>
	typename bitvector<Allocator, Element, Container>::reference
	bitvector<Allocator, Element, Container>::back()
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Allocator, typename Element, typename Container>
	typename bitvector<Allocator, Element, Container>::const_reference
	bitvector<Allocator, Element, Container>::back() const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Allocator, typename Element, typename Container>
	void bitvector<Allocator, Element, Container>::push_back()
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Allocator, typename Element, typename Container>
	void bitvector<Allocator, Element, Container>::push_back(value_type value)
	{
    __builtin_trap() /* STUB: not implemented */;
}
	

	template <typename Allocator, typename Element, typename Container>
	void bitvector<Allocator, Element, Container>::pop_back()
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Allocator, typename Element, typename Container>
	void bitvector<Allocator, Element, Container>::reserve(size_type n)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Allocator, typename Element, typename Container>
	void bitvector<Allocator, Element, Container>::resize(size_type n)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Allocator, typename Element, typename Container>
	void bitvector<Allocator, Element, Container>::resize(size_type n, value_type value)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Allocator, typename Element, typename Container>
	bool bitvector<Allocator, Element, Container>::test(size_type n, bool defaultValue) const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Allocator, typename Element, typename Container>
	void bitvector<Allocator, Element, Container>::set(size_type n, bool value)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Allocator, typename Element, typename Container>
	typename bitvector<Allocator, Element, Container>::reference
	bitvector<Allocator, Element, Container>::at(size_type n)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Allocator, typename Element, typename Container>
	typename bitvector<Allocator, Element, Container>::const_reference
	bitvector<Allocator, Element, Container>::at(size_type n) const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Allocator, typename Element, typename Container>
	typename bitvector<Allocator, Element, Container>::reference
	bitvector<Allocator, Element, Container>::operator[](size_type n)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Allocator, typename Element, typename Container>
	typename bitvector<Allocator, Element, Container>::const_reference
	bitvector<Allocator, Element, Container>::operator[](size_type n) const
	{
    __builtin_trap() /* STUB: not implemented */;
}


/*
	template <typename Allocator, typename Element, typename Container>
	template <bool value>
	typename bitvector<Allocator, Element, Container>::iterator 
	bitvector<Allocator, Element, Container>::find_first()
	{
		return begin();
	}

	template <bool value> iterator find_next(const_iterator it);
	template <bool value> iterator find_last();
	template <bool value> iterator find_prev(const_iterator it);
						
	template <bool value> const_iterator find_first() const;
	template <bool value> const_iterator find_next(const_iterator it) const;
	template <bool value> const_iterator find_last() const;
	template <bool value> const_iterator find_prev(const_iterator it) const;
*/




	template <typename Allocator, typename Element, typename Container>
	inline typename bitvector<Allocator, Element, Container>::container_type&
	bitvector<Allocator, Element, Container>::get_container()
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Allocator, typename Element, typename Container>
	inline const typename bitvector<Allocator, Element, Container>::container_type&
	bitvector<Allocator, Element, Container>::get_container() const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Allocator, typename Element, typename Container>
	bool bitvector<Allocator, Element, Container>::validate() const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Allocator, typename Element, typename Container>
	int bitvector<Allocator, Element, Container>::validate_iterator(const_iterator i) const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Allocator, typename Element, typename Container>
	typename bitvector<Allocator, Element, Container>::element_type*
	bitvector<Allocator, Element, Container>::data() EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}
	

	template <typename Allocator, typename Element, typename Container>
	const typename bitvector<Allocator, Element, Container>::element_type*
	bitvector<Allocator, Element, Container>::data() const EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Allocator, typename Element, typename Container>
	typename bitvector<Allocator, Element, Container>::iterator
	bitvector<Allocator, Element, Container>::insert(const_iterator position, value_type value)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Allocator, typename Element, typename Container>
	void bitvector<Allocator, Element, Container>::insert(const_iterator position, size_type n, value_type value)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	/*
	The following is a placeholder for a future implementation. It turns out that a correct implementation of 
	insert(pos, first, last) is a non-trivial exercise that would take a few hours to implement and test. 
	The reasons why involve primarily the problem of handling the case where insertion source comes from 
	within the container itself, and the case that first and last (note they are templated) might not refer 
	to iterators might refer to a value/count pair. The C++ Standard requires you to handle this case and 
	I (Paul Pedriana) believe that it applies even for a bitvector, given that bool is an integral type. 
	So you have to set up a compile-time type traits function chooser. See vector, for example.

	template <typename Allocator, typename Element, typename Container>
	template <typename InputIterator>
	void bitvector<Allocator, Element, Container>::insert(const_iterator position, InputIterator first, InputIterator last)
	{
		iterator iPosition(position.get_reference_type()); // This is just a non-const version of position.

		// This implementation is probably broken due to not handling insertion into self.
		// To do: Make a more efficient version of this.
		difference_type distance = (iPosition - begin());

		while(first != last)
		{
			insert(iPosition, *first);
			iPosition = begin() + ++distance;
			++first;
		}
	}
	*/


	template <typename Allocator, typename Element, typename Container>
	typename bitvector<Allocator, Element, Container>::iterator
	bitvector<Allocator, Element, Container>::erase(const_iterator position)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Allocator, typename Element, typename Container>
	typename bitvector<Allocator, Element, Container>::iterator
	bitvector<Allocator, Element, Container>::erase(const_iterator first, const_iterator last)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Allocator, typename Element, typename Container>
	typename bitvector<Allocator, Element, Container>::reverse_iterator
	bitvector<Allocator, Element, Container>::erase(const_reverse_iterator position)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Allocator, typename Element, typename Container>
	typename bitvector<Allocator, Element, Container>::reverse_iterator
	bitvector<Allocator, Element, Container>::erase(const_reverse_iterator first, const_reverse_iterator last)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Allocator, typename Element, typename Container>
	void bitvector<Allocator, Element, Container>::swap(this_type& rhs)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Allocator, typename Element, typename Container>
	void bitvector<Allocator, Element, Container>::reset_lose_memory()
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Allocator, typename Element, typename Container>
	void bitvector<Allocator, Element, Container>::clear()
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Allocator, typename Element, typename Container>
	bitvector<Allocator, Element, Container>::bitvector()
	  : mContainer(), 
		mFreeBitCount(0)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Allocator, typename Element, typename Container>
	bitvector<Allocator, Element, Container>::bitvector(const allocator_type& allocator)
	  : mContainer(allocator), 
		mFreeBitCount(0)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Allocator, typename Element, typename Container>
	bitvector<Allocator, Element, Container>::bitvector(size_type n, const allocator_type& allocator)
	   : mContainer((n + kBitCount - 1) / kBitCount, allocator)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Allocator, typename Element, typename Container>
	bitvector<Allocator, Element, Container>::bitvector(size_type n, value_type value, const allocator_type& allocator)
	  : mContainer((n + kBitCount - 1) / kBitCount, value ? ~element_type(0) : element_type(0), allocator)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Allocator, typename Element, typename Container>
	template <typename InputIterator>
	bitvector<Allocator, Element, Container>::bitvector(InputIterator first, InputIterator last)
	  : mContainer(), 
		mFreeBitCount(0)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename Allocator, typename Element, typename Container>
	bool bitvector<Allocator, Element, Container>::any() const
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename Allocator, typename Element, typename Container>
	bool bitvector<Allocator, Element, Container>::all() const
	{
    __builtin_trap() /* STUB: not implemented */;
}


	///////////////////////////////////////////////////////////////////////
	// global operators
	///////////////////////////////////////////////////////////////////////

	template <typename Allocator, typename Element, typename Container>
	inline bool operator==(const bitvector<Allocator, Element, Container>& a, 
						   const bitvector<Allocator, Element, Container>& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Allocator, typename Element, typename Container>
	inline bool operator!=(const bitvector<Allocator, Element, Container>& a, 
						   const bitvector<Allocator, Element, Container>& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Allocator, typename Element, typename Container>
	inline bool operator<(const bitvector<Allocator, Element, Container>& a, 
						  const bitvector<Allocator, Element, Container>& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Allocator, typename Element, typename Container>
	inline bool operator>(const bitvector<Allocator, Element, Container>& a, 
						  const bitvector<Allocator, Element, Container>& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Allocator, typename Element, typename Container>
	inline bool operator<=(const bitvector<Allocator, Element, Container>& a, 
						   const bitvector<Allocator, Element, Container>& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename Allocator, typename Element, typename Container>
	inline bool operator>=(const bitvector<Allocator, Element, Container>& a, 
						   const bitvector<Allocator, Element, Container>& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename Allocator, typename Element, typename Container>
	inline void swap(bitvector<Allocator, Element, Container>& a,
					 bitvector<Allocator, Element, Container>& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <size_t nodeCount,
              typename Allocator = EASTLAllocatorType, 
              typename Element   = BitvectorWordType, 
              typename Container = eastl::fixed_vector<Element, ((nodeCount + (sizeof(Element) << 3ULL)  - 1ULL) / (sizeof(Element) << 3ULL)), true, Allocator>>
	using fixed_bitvector = eastl::bitvector<Allocator, Element, Container>;

} // namespace eastl


EA_RESTORE_VC_WARNING();

#endif // Header include guard
