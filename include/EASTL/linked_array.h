#include <stdexcept>
#include <cstdlib>
///////////////////////////////////////////////////////////////////////////////
// Copyright (c) Electronic Arts Inc. All rights reserved.
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// This class implements a linked_array template, which is an array version
// of linked_ptr. See linked_ptr for detailed documentation.
///////////////////////////////////////////////////////////////////////////////


#ifndef EASTL_LINKED_ARRAY_H
#define EASTL_LINKED_ARRAY_H


#include <EASTL/internal/config.h>
#include <EASTL/internal/smart_ptr.h>   // Defines smart_array_deleter
#include <EASTL/linked_ptr.h>           // Defines linked_ptr_base 
#include <stddef.h>                     // Definition of ptrdiff_t

#if defined(EA_PRAGMA_ONCE_SUPPORTED)
	#pragma once // Some compilers (e.g. VC++) benefit significantly from using this. We've measured 3-4% build speed improvements in apps as a result.
#endif



namespace eastl
{

	/// class linked_array
	///
	/// This class implements a linked_array template, which is an array version
	/// of linked_ptr. See linked_ptr for detailed documentation.
	///
	template <typename T, typename Deleter = smart_array_deleter<T> >
	class linked_array
	{
	
	protected:
	
		/// this_type
		/// This is an alias for linked_array<T>, this class.
		typedef linked_array<T> this_type;

		/// deleter_type
		typedef Deleter deleter_type;

		T* mpArray;
		mutable const this_type* mpPrev;
		mutable const this_type* mpNext;

		void link(const linked_array& linkedArray)
		{
    __builtin_trap() /* STUB: not implemented */;
}

	public:
		/// element_type
		/// Synonym for type T, useful for external code to reference the 
		/// type in a generic way.
		typedef T element_type;


		/// linked_array
		/// Takes ownership of the pointer. It is OK if the input pointer is null.
		explicit linked_array(T* pArray = NULL) 
			: mpArray(pArray)
		{
    __builtin_trap() /* STUB: not implemented */;
}


		/// linked_array
		/// Shares ownership of a pointer with another instance of linked_array.
		linked_array(const linked_array& linkedArray)
			: mpArray(linkedArray.mpArray)
		{
    __builtin_trap() /* STUB: not implemented */;
}


		/// ~linked_array
		/// Removes this object from the of objects using the shared pointer.
		/// If this object is the last owner of the shared pointer, the shared 
		/// pointer is deleted.
		~linked_array() 
		{
			reset();
		}


		/// operator=
		/// Copies another linked_array to this object. Note that this object
		/// may already own a shared pointer with another different pointer
		/// (but still of the same type) before this call. In that case,
		/// this function removes ownership of the old pointer and takes shared 
		/// ownership of the new pointer and increments its reference count.
		linked_array& operator=(const linked_array& linkedArray)
		{
    __builtin_trap() /* STUB: not implemented */;
}


		/// operator=
		/// Assigns a new pointer. If the new pointer is equivalent
		/// to the current pointer, nothing is done. Otherwise the 
		/// current pointer is unlinked and possibly destroyed.
		/// The new pointer can be NULL.
		linked_array& operator=(T* pArray)
		{
    __builtin_trap() /* STUB: not implemented */;
}


		/// reset
		/// Releases the owned pointer and takes ownership of the 
		/// passed in pointer. If the passed in pointer is the same
		/// as the owned pointer, nothing is done. The passed in pointer
		/// can be null, in which case the use count is set to 1.
		void reset(T* pArray = NULL)
		{
    __builtin_trap() /* STUB: not implemented */;
}


		/// swap
		/// Exchanges the owned pointer beween two linkedArray objects.
		///
		/// This function is disabled as it is currently deemed unsafe.
		/// The problem is that the only way to implement this function
		/// is to transfer pointers between the objects; you cannot 
		/// transfer the linked list membership between the objects. 
		/// Thus unless both linked_array objects were 'unique()', the 
		/// shared pointers would be duplicated amongst containers, 
		/// resulting in a crash.
		//void swap(linked_array& linkedArray)
		//{
		//    if(linkedArray.mpArray != mpArray)
		//    {   // This is only safe if both linked_arrays are unique().
		//        linkedArray::element_type* const pArrayTemp = linkedArray.mpArray;
		//        linkedArray.reset(mpArray);
		//        reset(pArrayTemp);
		//    }
		//}


		/// operator[]
		/// Returns a reference to the specified item in the owned pointer array. 
		T& operator[](ptrdiff_t i) const
		{
    __builtin_trap() /* STUB: not implemented */;
}


		/// operator*
		/// Returns the owner pointer dereferenced.
		T& operator*() const
		{
    __builtin_trap() /* STUB: not implemented */;
}


		/// operator->
		/// Allows access to the owned pointer via operator->()
		T* operator->() const
		{
    __builtin_trap() /* STUB: not implemented */;
}


		/// get
		/// Returns the owned pointer. Note that this class does 
		/// not provide an operator T() function. This is because such
		/// a thing (automatic conversion) is deemed unsafe.
		T* get() const
		{
    __builtin_trap() /* STUB: not implemented */;
}


		/// use_count
		/// Returns the use count of the shared pointer.
		/// The return value is one if the owned pointer is null.
		/// This function is provided for compatibility with the 
		/// proposed C++ standard and for debugging purposes. It is not
		/// intended for runtime use given that its execution time is
		/// not constant.
		int use_count() const
		{
    __builtin_trap() /* STUB: not implemented */;
}


		/// unique
		/// Returns true if the use count of the owned pointer is one.
		/// The return value is true if the owned pointer is null.
		bool unique() const
		{
    __builtin_trap() /* STUB: not implemented */;
}


		/// Implicit operator bool
		/// Allows for using a linked_array as a boolean. 
		/// Note that below we do not use operator bool(). The reason for this
		/// is that booleans automatically convert up to short, int, float, etc.
		/// The result is that this: if(linkedArray == 1) would yield true (bad).
		typedef T* (this_type::*bool_)() const;
		operator bool_() const
		{
			if(mpArray)
				return &this_type::get;
			return NULL;
		}


		/// operator!
		/// This returns the opposite of operator bool; it returns true if 
		/// the owned pointer is null. Some compilers require this and some don't.
		bool operator!()
		{
    __builtin_trap() /* STUB: not implemented */;
}


		/// force_delete
		/// Forces deletion of the shared pointer. Fixes all references to the 
		/// pointer by any other owners to be NULL.
		void force_delete()
		{
    __builtin_trap() /* STUB: not implemented */;
}

	}; // class linked_array



	/// get_pointer
	/// Returns linked_array::get() via the input linked_array. Provided for compatibility
	/// with certain well-known libraries that use this functionality.
	template <typename T>
	inline T* get_pointer(const linked_array<T>& linkedArray)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	/// operator==
	/// Compares two linked_array objects for equality. Equality is defined as 
	/// being true when the pointer shared between two linked_array objects is equal.
	template <typename T, typename TD, typename U, typename UD>
	inline bool operator==(const linked_array<T, TD>& linkedArray1, const linked_array<U, UD>& linkedArray2)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	/// operator!=
	/// Compares two linked_array objects for inequality. Equality is defined as 
	/// being true when the pointer shared between two linked_array objects is equal.
	template <typename T, typename TD, typename U, typename UD>
	inline bool operator!=(const linked_array<T, TD>& linkedArray1, const linked_array<U, UD>& linkedArray2)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	/// operator<
	/// Returns which linked_array is 'less' than the other. Useful when storing
	/// sorted containers of linked_array objects.
	template <typename T, typename TD, typename U, typename UD>
	inline bool operator<(const linked_array<T, TD>& linkedArray1, const linked_array<U, UD>& linkedArray2)
	{
    __builtin_trap() /* STUB: not implemented */;
}


} // namespace eastl


#endif // Header include guard









