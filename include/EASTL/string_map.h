#include <stdexcept>
#include <cstdlib>
///////////////////////////////////////////////////////////////////////////////
// Copyright (c) Electronic Arts Inc. All rights reserved.
///////////////////////////////////////////////////////////////////////////////

#ifndef EASTL_STRING_MAP_H
#define EASTL_STRING_MAP_H

#if defined(EA_PRAGMA_ONCE_SUPPORTED)
	#pragma once 
#endif

#include <EASTL/map.h>
#include <EASTL/string.h>

namespace eastl
{


template<typename T, typename Predicate = str_less<const char*>, typename Allocator = EASTLAllocatorType>
class string_map : public eastl::map<const char*, T, Predicate, Allocator>
{
public:
	typedef eastl::map<const char*, T, Predicate, Allocator> base;
	typedef string_map<T, Predicate, Allocator>              this_type;
	typedef typename base::base_type::allocator_type         allocator_type;
	typedef typename base::base_type::insert_return_type     insert_return_type;
	typedef typename base::base_type::iterator               iterator;
	typedef typename base::base_type::reverse_iterator       reverse_iterator;
	typedef typename base::base_type::const_iterator         const_iterator;
	typedef typename base::base_type::size_type              size_type;
	typedef typename base::base_type::key_type               key_type;
	typedef typename base::base_type::value_type             value_type;
	typedef typename base::mapped_type                       mapped_type;

		                string_map(const allocator_type& allocator = allocator_type()) : base(allocator) {
    __builtin_trap() /* STUB: not implemented */;
}
						string_map(const string_map& src, const allocator_type& allocator = allocator_type());
						~string_map();
	void				clear();
	
	this_type&			operator=(const this_type& x);

	insert_return_type	insert(const char* key, const T& value);
	insert_return_type	insert(const char* key);
	iterator			erase(iterator position);
	size_type			erase(const char* key);
	mapped_type&		operator[](const char* key);

private:
	char*				strduplicate(const char* str);

	// Not implemented right now
	// insert_return_type	insert(const value_type& value);
	// iterator			    insert(iterator position, const value_type& value);
    // reverse_iterator	    erase(reverse_iterator position);
    // reverse_iterator	    erase(reverse_iterator first, reverse_iterator last);
    // void				    erase(const key_type* first, const key_type* last);
};



template<typename T, typename Predicate, typename Allocator>
string_map<T, Predicate, Allocator>::string_map(const string_map& src, const allocator_type& allocator) : base(allocator)
{
    __builtin_trap() /* STUB: not implemented */;
}

template<typename T, typename Predicate, typename Allocator>
string_map<T, Predicate, Allocator>::~string_map()
{
	clear();
}

template<typename T, typename Predicate, typename Allocator>
void
string_map<T, Predicate, Allocator>::clear()
{
    __builtin_trap() /* STUB: not implemented */;
}

template<typename T, typename Predicate, typename Allocator>
typename string_map<T, Predicate, Allocator>::this_type&
string_map<T, Predicate, Allocator>::operator=(const this_type& x)
{
    __builtin_trap() /* STUB: not implemented */;
}

template<typename T, typename Predicate, typename Allocator>
typename string_map<T, Predicate, Allocator>::insert_return_type
string_map<T, Predicate, Allocator>::insert(const char* key)
{
    __builtin_trap() /* STUB: not implemented */;
}

template<typename T, typename Predicate, typename Allocator>
typename string_map<T, Predicate, Allocator>::insert_return_type
string_map<T, Predicate, Allocator>::insert(const char* key, const T& value)
{
    __builtin_trap() /* STUB: not implemented */;
}

template<typename T, typename Predicate, typename Allocator>
typename string_map<T, Predicate, Allocator>::iterator
string_map<T, Predicate, Allocator>::erase(iterator position)
{
    __builtin_trap() /* STUB: not implemented */;
}

template<typename T, typename Predicate, typename Allocator>
typename string_map<T, Predicate, Allocator>::size_type
string_map<T, Predicate, Allocator>::erase(const char* key)
{
    __builtin_trap() /* STUB: not implemented */;
}

template<typename T, typename Predicate, typename Allocator>
typename string_map<T, Predicate, Allocator>::mapped_type&
string_map<T, Predicate, Allocator>::operator[](const char* key)
{
    __builtin_trap() /* STUB: not implemented */;
}

template<typename T, typename Predicate, typename Allocator>
char*
string_map<T, Predicate, Allocator>::strduplicate(const char* str)
{
    __builtin_trap() /* STUB: not implemented */;
}


}

#endif
