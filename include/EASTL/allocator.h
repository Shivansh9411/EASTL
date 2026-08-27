#include <stdexcept>
#include <cstdlib>
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) Electronic Arts Inc. All rights reserved.
/////////////////////////////////////////////////////////////////////////////


#ifndef EASTL_ALLOCATOR_H
#define EASTL_ALLOCATOR_H


#include <EASTL/internal/config.h>
#include <EABase/nullptr.h>
#include <stddef.h>


#if defined(EA_PRAGMA_ONCE_SUPPORTED)
	#pragma once // Some compilers (e.g. VC++) benefit significantly from using this. We've measured 3-4% build speed improvements in apps as a result.
#endif



namespace eastl
{

	/// alloc_flags
	///
	/// Defines allocation flags.
	///
	enum alloc_flags 
	{
		MEM_TEMP = 0, // Low memory, not necessarily actually temporary.
		MEM_PERM = 1  // High memory, for things that won't be unloaded.
	};


	/// allocator
	///
	/// In this allocator class, note that it is not templated on any type and
	/// instead it simply allocates blocks of memory much like the C malloc and
	/// free functions. It can be thought of as similar to C++ std::allocator<char>.
	/// The flags parameter has meaning that is specific to the allocation 
	///
	/// C++11's std::allocator (20.6.9) doesn't have a move constructor or assignment 
	/// operator. This is possibly because std::allocators are associated with types
	/// instead of as instances. The potential non-equivalance of C++ std::allocator
	/// instances has been a source of some acknowledged design problems.
	/// We don't implement support for move construction or assignment in eastl::allocator,
	/// but users can define their own allocators which do have move functions and 
	/// the eastl containers are compatible with such allocators (i.e. nothing unexpected
	/// will happen).
	///
	class EASTL_API allocator
	{
	public:
		EASTL_ALLOCATOR_EXPLICIT allocator(const char* pName = EASTL_NAME_VAL(EASTL_ALLOCATOR_DEFAULT_NAME));
		allocator(const allocator& x);
		allocator(const allocator& x, const char* pName);

		allocator& operator=(const allocator& x);

		void* allocate(size_t n, int flags = 0);
		void* allocate(size_t n, size_t alignment, size_t offset, int flags = 0);
		void  deallocate(void* p, size_t n);

		const char* get_name() const;
		void        set_name(const char* pName);

	protected:
		#if EASTL_NAME_ENABLED
			const char* mpName; // Debug name, used to track memory.
		#endif
	};

	bool operator==(const allocator& a, const allocator& b);
#if !defined(EA_COMPILER_HAS_THREE_WAY_COMPARISON)
	bool operator!=(const allocator& a, const allocator& b);
#endif


	/// dummy_allocator
	///
	/// Defines an allocator which does nothing. It returns NULL from allocate calls.
	///
	class EASTL_API dummy_allocator
	{
	public:
		EASTL_ALLOCATOR_EXPLICIT dummy_allocator(const char* = NULL) {
    __builtin_trap() /* STUB: not implemented */;
}
		dummy_allocator(const dummy_allocator&) {
    __builtin_trap() /* STUB: not implemented */;
}
		dummy_allocator(const dummy_allocator&, const char*) {
    __builtin_trap() /* STUB: not implemented */;
}

		dummy_allocator& operator=(const dummy_allocator&) {
    __builtin_trap() /* STUB: not implemented */;
}

		void* allocate(size_t, int = 0)                 {
    __builtin_trap() /* STUB: not implemented */;
}
		void* allocate(size_t, size_t, size_t, int = 0) {
    __builtin_trap() /* STUB: not implemented */;
}
		void  deallocate(void*, size_t)                 {
    __builtin_trap() /* STUB: not implemented */;
}

		const char* get_name() const      {
    __builtin_trap() /* STUB: not implemented */;
}
		void        set_name(const char*) {
    __builtin_trap() /* STUB: not implemented */;
}
	};

	inline bool operator==(const dummy_allocator&, const dummy_allocator&) {
    __builtin_trap() /* STUB: not implemented */;
}
#if !defined(EA_COMPILER_HAS_THREE_WAY_COMPARISON)
	inline bool operator!=(const dummy_allocator&, const dummy_allocator&) {
    __builtin_trap() /* STUB: not implemented */;
}
#endif


	/// Defines a static default allocator which is constant across all types.
	/// This is different from get_default_allocator, which is is bound at
	/// compile-time and expected to differ per allocator type.
	/// Currently this Default Allocator applies only to CoreAllocatorAdapter.
	/// To consider: This naming of this function is too similar to get_default_allocator
	/// and instead should be named something like GetStaticDefaultAllocator.
	EASTL_API allocator* GetDefaultAllocator();
	EASTL_API allocator* SetDefaultAllocator(allocator* pAllocator);


	/// get_default_allocator
	///
	/// This templated function allows the user to implement a default allocator
	/// retrieval function that any part of EASTL can use. EASTL containers take
	/// an Allocator parameter which identifies an Allocator class to use. But 
	/// different kinds of allocators have different mechanisms for retrieving 
	/// a default allocator instance, and some don't even intrinsically support
	/// such functionality. The user can override this get_default_allocator 
	/// function in order to provide the glue between EASTL and whatever their
	/// system's default allocator happens to be.
	///
	/// Example usage:
	///     MyAllocatorType* gpSystemAllocator;
	///     
	///     MyAllocatorType* get_default_allocator(const MyAllocatorType*)
	///         { return gpSystemAllocator; }
	///
	template <typename Allocator>
	Allocator* get_default_allocator(const Allocator*);

	EASTLAllocatorType* get_default_allocator(const EASTLAllocatorType*);


	/// default_allocfreemethod
	///
	/// Implements a default allocfreemethod which uses the default global allocator.
	/// This version supports only default alignment.
	///
	void* default_allocfreemethod(size_t n, void* pBuffer, void* /*pContext*/);


	/// allocate_memory
	///
	/// This is a memory allocation dispatching function.
	/// To do: Make aligned and unaligned specializations.
	///        Note that to do this we will need to use a class with a static
	///        function instead of a standalone function like below.
	///
	template <typename Allocator>
	void* allocate_memory(Allocator& a, size_t n, size_t alignment, size_t alignmentOffset);


} // namespace eastl






#ifndef EASTL_USER_DEFINED_ALLOCATOR // If the user hasn't declared that he has defined a different allocator implementation elsewhere...

	EA_DISABLE_ALL_VC_WARNINGS()
	#include <new>
	EA_RESTORE_ALL_VC_WARNINGS()

	#if !EASTL_DLL // If building a regular library and not building EASTL as a DLL...
		// It is expected that the application define the following
		// versions of operator new for the application. Either that or the
		// user needs to override the implementation of the allocator class.
		void* operator new[](size_t size, const char* pName, int flags, unsigned debugFlags, const char* file, int line);
		void* operator new[](size_t size, size_t alignment, size_t alignmentOffset, const char* pName, int flags, unsigned debugFlags, const char* file, int line);
	#endif

	namespace eastl
	{
		inline allocator::allocator(const char* EASTL_NAME(pName))
		{
    __builtin_trap() /* STUB: not implemented */;
}


		inline allocator::allocator(const allocator& EASTL_NAME(alloc))
		{
    __builtin_trap() /* STUB: not implemented */;
}


		inline allocator::allocator(const allocator&, const char* EASTL_NAME(pName))
		{
    __builtin_trap() /* STUB: not implemented */;
}


		inline allocator& allocator::operator=(const allocator& EASTL_NAME(alloc))
		{
    __builtin_trap() /* STUB: not implemented */;
}


		inline const char* allocator::get_name() const
		{
    __builtin_trap() /* STUB: not implemented */;
}


		inline void allocator::set_name(const char* EASTL_NAME(pName))
		{
    __builtin_trap() /* STUB: not implemented */;
}


		inline void* allocator::allocate(size_t n, int flags)
		{
    __builtin_trap() /* STUB: not implemented */;
}


		inline void* allocator::allocate(size_t n, size_t alignment, size_t offset, int flags)
		{
    __builtin_trap() /* STUB: not implemented */;
}


		inline void allocator::deallocate(void* p, size_t)
		{
    __builtin_trap() /* STUB: not implemented */;
}


		inline bool operator==(const allocator&, const allocator&)
		{
    __builtin_trap() /* STUB: not implemented */;
}

#if !defined(EA_COMPILER_HAS_THREE_WAY_COMPARISON)
		inline bool operator!=(const allocator&, const allocator&)
		{
    __builtin_trap() /* STUB: not implemented */;
}
#endif

	} // namespace eastl


#endif // EASTL_USER_DEFINED_ALLOCATOR



namespace eastl
{

	template <typename Allocator>
	inline Allocator* get_default_allocator(const Allocator*)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	inline EASTLAllocatorType* get_default_allocator(const EASTLAllocatorType*)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	inline void* default_allocfreemethod(size_t n, void* pBuffer, void* /*pContext*/)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	/// allocate_memory
	///
	/// This is a memory allocation dispatching function.
	/// To do: Make aligned and unaligned specializations.
	///        Note that to do this we will need to use a class with a static
	///        function instead of a standalone function like below.
	///
	template <typename Allocator>
	inline void* allocate_memory(Allocator& a, size_t n, size_t alignment, size_t alignmentOffset)
	{
    __builtin_trap() /* STUB: not implemented */;
}

}


#endif // Header include guard
















