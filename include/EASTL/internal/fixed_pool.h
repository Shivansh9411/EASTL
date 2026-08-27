#include <stdexcept>
#include <cstdlib>
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) Electronic Arts Inc. All rights reserved.
/////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// This file implements the following
//     aligned_buffer
//     fixed_pool_base
//     fixed_pool
//     fixed_pool_with_overflow
//     fixed_hashtable_allocator
//     fixed_vector_allocator
//     fixed_swap
//
///////////////////////////////////////////////////////////////////////////////


#ifndef EASTL_INTERNAL_FIXED_POOL_H
#define EASTL_INTERNAL_FIXED_POOL_H


#include <EABase/eabase.h>
#if defined(EA_PRAGMA_ONCE_SUPPORTED)
	#pragma once
#endif

#include <EASTL/internal/config.h>
#include <EASTL/functional.h>
#include <EASTL/memory.h>
#include <EASTL/allocator.h>
#include <EASTL/type_traits.h>


EA_DISABLE_ALL_VC_WARNINGS();
#include <new>
EA_RESTORE_ALL_VC_WARNINGS();

// 4275 - non dll-interface class used as base for DLL-interface classkey 'identifier'
EA_DISABLE_VC_WARNING(4275);


namespace eastl
{

	/// EASTL_FIXED_POOL_DEFAULT_NAME
	///
	/// Defines a default allocator name in the absence of a user-provided name.
	///
	#ifndef EASTL_FIXED_POOL_DEFAULT_NAME
		#define EASTL_FIXED_POOL_DEFAULT_NAME EASTL_DEFAULT_NAME_PREFIX " fixed_pool" // Unless the user overrides something, this is "EASTL fixed_pool".
	#endif



	///////////////////////////////////////////////////////////////////////////
	// aligned_buffer
	///////////////////////////////////////////////////////////////////////////

	/// aligned_buffer
	///
	/// This is useful for creating a buffer of the same size and alignment 
	/// of a given struct or class. This is useful for creating memory pools
	/// that support both size and alignment requirements of stored objects
	/// but without wasting space in over-allocating. 
	///
	/// Note that we implement this via struct specializations, as some 
	/// compilers such as VC++ do not support specification of alignments
	/// in any way other than via an integral constant.
	///
	/// Example usage:
	///    struct Widget{ }; // This class has a given size and alignment.
	///
	///    Declare a char buffer of equal size and alignment to Widget.
	///    aligned_buffer<sizeof(Widget), EASTL_ALIGN_OF(Widget)> mWidgetBuffer; 
	///
	///    Declare an array this time.
	///    aligned_buffer<sizeof(Widget), EASTL_ALIGN_OF(Widget)> mWidgetArray[15]; 
	///
	typedef char EASTL_MAY_ALIAS aligned_buffer_char; 

	template <size_t size, size_t alignment>
	struct aligned_buffer { aligned_buffer_char buffer[size]; };

	template<size_t size>
	struct aligned_buffer<size, 2>    { EA_PREFIX_ALIGN(2) aligned_buffer_char buffer[size] EA_POSTFIX_ALIGN(2); };

	template<size_t size>
	struct aligned_buffer<size, 4>    { EA_PREFIX_ALIGN(4) aligned_buffer_char buffer[size] EA_POSTFIX_ALIGN(4); };

	template<size_t size>
	struct aligned_buffer<size, 8>    { EA_PREFIX_ALIGN(8) aligned_buffer_char buffer[size] EA_POSTFIX_ALIGN(8); };

	template<size_t size>
	struct aligned_buffer<size, 16>   { EA_PREFIX_ALIGN(16) aligned_buffer_char buffer[size] EA_POSTFIX_ALIGN(16); };

	template<size_t size>
	struct aligned_buffer<size, 32>   { EA_PREFIX_ALIGN(32) aligned_buffer_char buffer[size] EA_POSTFIX_ALIGN(32); };

	template<size_t size>
	struct aligned_buffer<size, 64>   { EA_PREFIX_ALIGN(64) aligned_buffer_char buffer[size] EA_POSTFIX_ALIGN(64); };

	template<size_t size>
	struct aligned_buffer<size, 128>  { EA_PREFIX_ALIGN(128) aligned_buffer_char buffer[size] EA_POSTFIX_ALIGN(128); };

	template<size_t size>
	struct aligned_buffer<size, 256>  { EA_PREFIX_ALIGN(256) aligned_buffer_char buffer[size] EA_POSTFIX_ALIGN(256); };

	template<size_t size>
	struct aligned_buffer<size, 512>  { EA_PREFIX_ALIGN(512) aligned_buffer_char buffer[size] EA_POSTFIX_ALIGN(512); };

	template<size_t size>
	struct aligned_buffer<size, 1024> { EA_PREFIX_ALIGN(1024) aligned_buffer_char buffer[size] EA_POSTFIX_ALIGN(1024); };

	template<size_t size>
	struct aligned_buffer<size, 2048> { EA_PREFIX_ALIGN(2048) aligned_buffer_char buffer[size] EA_POSTFIX_ALIGN(2048); };

	template<size_t size>
	struct aligned_buffer<size, 4096> { EA_PREFIX_ALIGN(4096) aligned_buffer_char buffer[size] EA_POSTFIX_ALIGN(4096); };




	///////////////////////////////////////////////////////////////////////////
	// fixed_pool_base
	///////////////////////////////////////////////////////////////////////////

	/// fixed_pool_base
	///
	/// This is a base class for the implementation of fixed-size pools.
	/// In particular, the fixed_pool and fixed_pool_with_overflow classes
	/// are based on fixed_pool_base.
	///
	struct fixed_pool_base
	{
	public:
		/// fixed_pool_base
		///
		fixed_pool_base(void* pMemory = NULL)
			: mpHead((Link*)pMemory)
			, mpNext((Link*)pMemory)
			, mpCapacity((Link*)pMemory)
			, mnNodeSize(0) // This is normally set in the init function.
		{
    __builtin_trap() /* STUB: not implemented */;
}


		/// fixed_pool_base
		///
		// Disabled because the default is sufficient. While it normally makes no sense to deep copy
		// this data, our usage of this class is such that this is OK and wanted.
		//
		// fixed_pool_base(const fixed_pool_base& x)
		// {
		// }


		/// operator=
		///
		fixed_pool_base& operator=(const fixed_pool_base&)
		{
    __builtin_trap() /* STUB: not implemented */;
}


		/// init
		///
		/// Initializes a fixed_pool with a given set of parameters.
		/// You cannot call this function twice else the resulting 
		/// behaviour will be undefined. You can only call this function
		/// after constructing the fixed_pool with the default constructor.
		///
		EASTL_API void init(void* pMemory, size_t memorySize, size_t nodeSize,
							size_t alignment, size_t alignmentOffset = 0);


		/// peak_size
		///
		/// Returns the maximum number of outstanding allocations there have been
		/// at any one time. This represents a high water mark for the allocation count.
		///
		size_t peak_size() const
		{
    __builtin_trap() /* STUB: not implemented */;
}


		/// can_allocate
		///
		/// Returns true if there are any free links.
		///
		bool can_allocate() const
		{
    __builtin_trap() /* STUB: not implemented */;
}

	public:
		/// Link
		/// Implements a singly-linked list.
		struct Link
		{
			Link* mpNext;
		};

		Link*   mpHead;
		Link*   mpNext;
		Link*   mpCapacity;
		size_t  mnNodeSize;

		#if EASTL_FIXED_SIZE_TRACKING_ENABLED
			uint32_t mnCurrentSize; /// Current number of allocated nodes.
			uint32_t mnPeakSize;    /// Max number of allocated nodes at any one time.
		#endif

	}; // fixed_pool_base





	///////////////////////////////////////////////////////////////////////////
	// fixed_pool
	///////////////////////////////////////////////////////////////////////////

	/// fixed_pool
	///
	/// Implements a simple fixed pool allocator for use by fixed-size containers. 
	/// This is not a generic eastl allocator which can be plugged into an arbitrary
	/// eastl container, as it simplifies some functions are arguments for the 
	/// purpose of efficiency.
	/// 
	class EASTL_API fixed_pool : public fixed_pool_base
	{
	public:
		/// fixed_pool
		///
		/// Default constructor. User usually will want to call init() after  
		/// constructing via this constructor. The pMemory argument is for the 
		/// purposes of temporarily storing a pointer to the buffer to be used.
		/// Even though init may have a pMemory argument, this arg is useful 
		/// for temporary storage, as per copy construction.
		///
		fixed_pool(void* pMemory = NULL)
			: fixed_pool_base(pMemory)
		{
    __builtin_trap() /* STUB: not implemented */;
}


		/// fixed_pool
		///
		/// Constructs a fixed_pool with a given set of parameters.
		///
		fixed_pool(void* pMemory, size_t memorySize, size_t nodeSize, 
					size_t alignment, size_t alignmentOffset = 0)
		{
    __builtin_trap() /* STUB: not implemented */;
}


		/// fixed_pool
		///
		// Disabled because the default is sufficient. While it normally makes no sense to deep copy
		// this data, our usage of this class is such that this is OK and wanted.
		//
		// fixed_pool(const fixed_pool& x)
		// {
		// }


		/// operator=
		///
		fixed_pool& operator=(const fixed_pool&)
		{
    __builtin_trap() /* STUB: not implemented */;
}


		/// allocate
		///
		/// Allocates a new object of the size specified upon class initialization.
		/// Returns NULL if there is no more memory. 
		///
		void* allocate()
		{
    __builtin_trap() /* STUB: not implemented */;
}

		void* allocate(size_t /*alignment*/, size_t /*offset*/)
		{
    __builtin_trap() /* STUB: not implemented */;
}
		
		/// deallocate
		///
		/// Frees the given object which was allocated by allocate(). 
		/// If the given node was not allocated by allocate() then the behaviour 
		/// is undefined.
		///
		void deallocate(void* p)
		{
    __builtin_trap() /* STUB: not implemented */;
}


		using fixed_pool_base::can_allocate;


		const char* get_name() const
		{
    __builtin_trap() /* STUB: not implemented */;
}


		void set_name(const char*)
		{
    __builtin_trap() /* STUB: not implemented */;
}

	}; // fixed_pool





	///////////////////////////////////////////////////////////////////////////
	// fixed_pool_with_overflow
	///////////////////////////////////////////////////////////////////////////

	/// fixed_pool_with_overflow
	///
	template <typename OverflowAllocator = EASTLAllocatorType>
	class fixed_pool_with_overflow : public fixed_pool_base
	{
	public:
		typedef OverflowAllocator overflow_allocator_type;


		fixed_pool_with_overflow(void* pMemory = NULL)
			: fixed_pool_base(pMemory),
			  mOverflowAllocator(EASTL_FIXED_POOL_DEFAULT_NAME)
		{
    __builtin_trap() /* STUB: not implemented */;
}


		fixed_pool_with_overflow(void* pMemory, const overflow_allocator_type& allocator)
			: fixed_pool_base(pMemory),
			  mOverflowAllocator(allocator)
		{
    __builtin_trap() /* STUB: not implemented */;
}


		fixed_pool_with_overflow(void* pMemory, size_t memorySize, size_t nodeSize, 
								 size_t alignment, size_t alignmentOffset = 0)
			: mOverflowAllocator(EASTL_FIXED_POOL_DEFAULT_NAME)
		{
    __builtin_trap() /* STUB: not implemented */;
}


		fixed_pool_with_overflow(void* pMemory, size_t memorySize, size_t nodeSize, 
								 size_t alignment, size_t alignmentOffset,
								 const overflow_allocator_type& allocator)
			: mOverflowAllocator(allocator)
		{
    __builtin_trap() /* STUB: not implemented */;
}


		// Disabled because the default is sufficient. While it normally makes no sense to deep copy
		// this data, our usage of this class is such that this is OK and wanted.
		//
		//fixed_pool_with_overflow(const fixed_pool_with_overflow& x)
		//{
		//    ...
		//}


		fixed_pool_with_overflow& operator=(const fixed_pool_with_overflow& x)
		{
    __builtin_trap() /* STUB: not implemented */;
}


		void init(void* pMemory, size_t memorySize, size_t nodeSize,
					size_t alignment, size_t alignmentOffset = 0)
		{
    __builtin_trap() /* STUB: not implemented */;
}


		void* allocate()
		{
    __builtin_trap() /* STUB: not implemented */;
}


		void* allocate(size_t alignment, size_t alignmentOffset)
		{
    __builtin_trap() /* STUB: not implemented */;
}

		void deallocate(void* p)
		{
    __builtin_trap() /* STUB: not implemented */;
}


		using fixed_pool_base::can_allocate;


		const char* get_name() const
		{
    __builtin_trap() /* STUB: not implemented */;
}


		void set_name(const char* pName)
		{
    __builtin_trap() /* STUB: not implemented */;
}


		const overflow_allocator_type& get_overflow_allocator() const
		{
    __builtin_trap() /* STUB: not implemented */;
}


		overflow_allocator_type& get_overflow_allocator()
		{
    __builtin_trap() /* STUB: not implemented */;
}
		

		void set_overflow_allocator(const overflow_allocator_type& overflowAllocator)
		{
    __builtin_trap() /* STUB: not implemented */;
}
	public:
		OverflowAllocator mOverflowAllocator; 
		void*             mpPoolBegin;         // Ideally we wouldn't need this member variable. he problem is that the information about the pool buffer and object size is stored in the owning container and we can't have access to it without increasing the amount of code we need and by templating more code. It may turn out that simply storing data here is smaller in the end.

	}; // fixed_pool_with_overflow              





	///////////////////////////////////////////////////////////////////////////
	// fixed_node_allocator
	///////////////////////////////////////////////////////////////////////////

	/// fixed_node_allocator
	///
	/// Note: This class was previously named fixed_node_pool, but was changed because this name
	///       was inconsistent with the other allocators here which ended with _allocator.
	///
	/// Implements a fixed_pool with a given node count, alignment, and alignment offset.
	/// fixed_node_allocator is like fixed_pool except it is templated on the node type instead
	/// of being a generic allocator. All it does is pass allocations through to
	/// the fixed_pool base. This functionality is separate from fixed_pool because there
	/// are other uses for fixed_pool.
	///
	/// We template on kNodeSize instead of node_type because the former allows for the
	/// two different node_types of the same size to use the same template implementation.
	///
	/// Template parameters:
	///     nodeSize               The size of the object to allocate.
	///     nodeCount              The number of objects the pool contains.
	///     nodeAlignment          The alignment of the objects to allocate.
	///     nodeAlignmentOffset    The alignment offset of the objects to allocate.
	///     bEnableOverflow        Whether or not we should use the overflow heap if our object pool is exhausted.
	///     OverflowAllocator      Overflow allocator, which is only used if bEnableOverflow == true. Defaults to the global heap.
	///
	template <size_t nodeSize, size_t nodeCount, size_t nodeAlignment, size_t nodeAlignmentOffset, bool bEnableOverflow, typename OverflowAllocator = EASTLAllocatorType>
	class fixed_node_allocator
	{
	public:
		typedef typename conditional<bEnableOverflow, fixed_pool_with_overflow<OverflowAllocator>, fixed_pool>::type  pool_type;
		typedef fixed_node_allocator<nodeSize, nodeCount, nodeAlignment, nodeAlignmentOffset, bEnableOverflow, OverflowAllocator>   this_type;
		typedef OverflowAllocator overflow_allocator_type;

		enum
		{
			kNodeSize            = nodeSize,
			kNodeCount           = nodeCount,
			kNodesSize           = nodeCount * nodeSize, // Note that the kBufferSize calculation assumes that the compiler sets sizeof(T) to be a multiple alignof(T), and so sizeof(T) is always >= alignof(T).
			kBufferSize          = kNodesSize + ((nodeAlignment > 1) ? nodeSize-1 : 0) + nodeAlignmentOffset,
			kNodeAlignment       = nodeAlignment,
			kNodeAlignmentOffset = nodeAlignmentOffset
		};

	public:
		pool_type mPool;

	public:
		//fixed_node_allocator(const char* pName)
		//{
		//    mPool.set_name(pName);
		//}


		fixed_node_allocator(void* pNodeBuffer)
			: mPool(pNodeBuffer, kNodesSize, kNodeSize, kNodeAlignment, kNodeAlignmentOffset)
		{
    __builtin_trap() /* STUB: not implemented */;
}


		fixed_node_allocator(void* pNodeBuffer, const overflow_allocator_type& allocator)
			: mPool(pNodeBuffer, kNodesSize, kNodeSize, kNodeAlignment, kNodeAlignmentOffset, allocator)
		{
    __builtin_trap() /* STUB: not implemented */;
}


		/// fixed_node_allocator
		///
		/// Note that we are copying x.mpHead to our own fixed_pool. This at first may seem 
		/// broken, as fixed pools cannot take over ownership of other fixed pools' memory.
		/// However, we declare that this copy ctor can only ever be safely called when 
		/// the user has intentionally pre-seeded the source with the destination pointer.
		/// This is somewhat playing with fire, but it allows us to get around chicken-and-egg
		/// problems with containers being their own allocators, without incurring any memory
		/// costs or extra code costs. There's another reason for this: we very strongly want
		/// to avoid full copying of instances of fixed_pool around, especially via the stack.
		/// Larger pools won't even be able to fit on many machine's stacks. So this solution
		/// is also a mechanism to prevent that situation from existing and being used. 
		/// Perhaps some day we'll find a more elegant yet costless way around this. 
		///
		fixed_node_allocator(const this_type& x)
			: mPool(x.mPool.mpNext, kNodesSize, kNodeSize, kNodeAlignment, kNodeAlignmentOffset, x.mPool.mOverflowAllocator)
		{
    __builtin_trap() /* STUB: not implemented */;
}


		this_type& operator=(const this_type& x)
		{
    __builtin_trap() /* STUB: not implemented */;
}


		void* allocate(size_t n, int /*flags*/ = 0)
		{
    __builtin_trap() /* STUB: not implemented */;
}


		void* allocate(size_t n, size_t alignment, size_t offset, int /*flags*/ = 0)
		{
    __builtin_trap() /* STUB: not implemented */;
}


		void deallocate(void* p, size_t)
		{
    __builtin_trap() /* STUB: not implemented */;
}


		/// can_allocate
		///
		/// Returns true if there are any free links.
		///
		bool can_allocate() const
		{
    __builtin_trap() /* STUB: not implemented */;
}


		/// reset
		///
		/// This function unilaterally resets the fixed pool back to a newly initialized
		/// state. This is useful for using in tandem with container reset functionality.
		///
		void reset(void* pNodeBuffer)
		{
    __builtin_trap() /* STUB: not implemented */;
}


		const char* get_name() const
		{
    __builtin_trap() /* STUB: not implemented */;
}


		void set_name(const char* pName)
		{
    __builtin_trap() /* STUB: not implemented */;
}


		const overflow_allocator_type& get_overflow_allocator() const EA_NOEXCEPT
		{
    __builtin_trap() /* STUB: not implemented */;
}


		overflow_allocator_type& get_overflow_allocator() EA_NOEXCEPT
		{
    __builtin_trap() /* STUB: not implemented */;
}


		void set_overflow_allocator(const overflow_allocator_type& allocator)
		{
    __builtin_trap() /* STUB: not implemented */;
}


		void copy_overflow_allocator(const this_type& x)  // This function exists so we can write generic code that works for allocators that do and don't have overflow allocators.
		{
    __builtin_trap() /* STUB: not implemented */;
}

	}; // fixed_node_allocator


	// This is a near copy of the code above, with the only difference being 
	// the 'false' bEnableOverflow template parameter, the pool_type and this_type typedefs, 
	// and the get_overflow_allocator / set_overflow_allocator functions.
	template <size_t nodeSize, size_t nodeCount, size_t nodeAlignment, size_t nodeAlignmentOffset, typename OverflowAllocator>
	class fixed_node_allocator<nodeSize, nodeCount, nodeAlignment, nodeAlignmentOffset, false, OverflowAllocator>
	{
	public:
		typedef fixed_pool pool_type;
		typedef fixed_node_allocator<nodeSize, nodeCount, nodeAlignment, nodeAlignmentOffset, false, OverflowAllocator>   this_type;
		typedef OverflowAllocator overflow_allocator_type;

		enum
		{
			kNodeSize            = nodeSize,
			kNodeCount           = nodeCount,
			kNodesSize           = nodeCount * nodeSize, // Note that the kBufferSize calculation assumes that the compiler sets sizeof(T) to be a multiple alignof(T), and so sizeof(T) is always >= alignof(T).
			kBufferSize          = kNodesSize + ((nodeAlignment > 1) ? nodeSize-1 : 0) + nodeAlignmentOffset,
			kNodeAlignment       = nodeAlignment,
			kNodeAlignmentOffset = nodeAlignmentOffset
		};

	public:
		pool_type mPool;

	public:
		fixed_node_allocator(void* pNodeBuffer)
			: mPool(pNodeBuffer, kNodesSize, kNodeSize, kNodeAlignment, kNodeAlignmentOffset)
		{
    __builtin_trap() /* STUB: not implemented */;
}


		fixed_node_allocator(void* pNodeBuffer, const overflow_allocator_type& /*allocator*/) // allocator is unused because bEnableOverflow is false in this specialization.
			: mPool(pNodeBuffer, kNodesSize, kNodeSize, kNodeAlignment, kNodeAlignmentOffset)
		{
    __builtin_trap() /* STUB: not implemented */;
}


		/// fixed_node_allocator
		///
		/// Note that we are copying x.mpHead to our own fixed_pool. This at first may seem 
		/// broken, as fixed pools cannot take over ownership of other fixed pools' memory.
		/// However, we declare that this copy ctor can only ever be safely called when 
		/// the user has intentionally pre-seeded the source with the destination pointer.
		/// This is somewhat playing with fire, but it allows us to get around chicken-and-egg
		/// problems with containers being their own allocators, without incurring any memory
		/// costs or extra code costs. There's another reason for this: we very strongly want
		/// to avoid full copying of instances of fixed_pool around, especially via the stack.
		/// Larger pools won't even be able to fit on many machine's stacks. So this solution
		/// is also a mechanism to prevent that situation from existing and being used. 
		/// Perhaps some day we'll find a more elegant yet costless way around this. 
		///
		fixed_node_allocator(const this_type& x)            // No need to copy the overflow allocator, because bEnableOverflow is false in this specialization.
			: mPool(x.mPool.mpNext, kNodesSize, kNodeSize, kNodeAlignment, kNodeAlignmentOffset)
		{
    __builtin_trap() /* STUB: not implemented */;
}


		this_type& operator=(const this_type& x)
		{
    __builtin_trap() /* STUB: not implemented */;
}


		void* allocate(size_t n, int /*flags*/ = 0)
		{
    __builtin_trap() /* STUB: not implemented */;
}


		void* allocate(size_t n, size_t alignment, size_t offset, int /*flags*/ = 0)
		{
    __builtin_trap() /* STUB: not implemented */;
}


		void deallocate(void* p, size_t)
		{
    __builtin_trap() /* STUB: not implemented */;
}


		bool can_allocate() const
		{
    __builtin_trap() /* STUB: not implemented */;
}


		void reset(void* pNodeBuffer)
		{
    __builtin_trap() /* STUB: not implemented */;
}


		const char* get_name() const
		{
    __builtin_trap() /* STUB: not implemented */;
}


		void set_name(const char* pName)
		{
    __builtin_trap() /* STUB: not implemented */;
}


		const overflow_allocator_type& get_overflow_allocator() const EA_NOEXCEPT
		{
    __builtin_trap() /* STUB: not implemented */;
}


		overflow_allocator_type& get_overflow_allocator() EA_NOEXCEPT
		{
    __builtin_trap() /* STUB: not implemented */;
}


		void set_overflow_allocator(const overflow_allocator_type& /*allocator*/)
		{
    __builtin_trap() /* STUB: not implemented */;
}


		void copy_overflow_allocator(const this_type&)  // This function exists so we can write generic code that works for allocators that do and don't have overflow allocators.
		{
    __builtin_trap() /* STUB: not implemented */;
}

	}; // fixed_node_allocator




	///////////////////////////////////////////////////////////////////////
	// global operators
	///////////////////////////////////////////////////////////////////////

	template <size_t nodeSize, size_t nodeCount, size_t nodeAlignment, size_t nodeAlignmentOffset, bool bEnableOverflow, typename OverflowAllocator>
	inline bool operator==(const fixed_node_allocator<nodeSize, nodeCount, nodeAlignment, nodeAlignmentOffset, bEnableOverflow, OverflowAllocator>& a, 
						   const fixed_node_allocator<nodeSize, nodeCount, nodeAlignment, nodeAlignmentOffset, bEnableOverflow, OverflowAllocator>& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <size_t nodeSize, size_t nodeCount, size_t nodeAlignment, size_t nodeAlignmentOffset, bool bEnableOverflow, typename OverflowAllocator>
	inline bool operator!=(const fixed_node_allocator<nodeSize, nodeCount, nodeAlignment, nodeAlignmentOffset, bEnableOverflow, OverflowAllocator>& a, 
						   const fixed_node_allocator<nodeSize, nodeCount, nodeAlignment, nodeAlignmentOffset, bEnableOverflow, OverflowAllocator>& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}






	///////////////////////////////////////////////////////////////////////////
	// fixed_hashtable_allocator
	///////////////////////////////////////////////////////////////////////////

	/// fixed_hashtable_allocator
	///
	/// Provides a base class for fixed hashtable allocations.
	/// To consider: Have this inherit from fixed_node_allocator.
	///
	/// Template parameters:
	///     bucketCount            The fixed number of hashtable buckets to provide.
	///     nodeCount              The number of objects the pool contains.
	///     nodeAlignment          The alignment of the objects to allocate.
	///     nodeAlignmentOffset    The alignment offset of the objects to allocate.
	///     bEnableOverflow        Whether or not we should use the overflow heap if our object pool is exhausted.
	///     OverflowAllocator      Overflow allocator, which is only used if bEnableOverflow == true. Defaults to the global heap.
	///
	template <size_t bucketCount, size_t nodeSize, size_t nodeCount, size_t nodeAlignment, size_t nodeAlignmentOffset, bool bEnableOverflow, typename OverflowAllocator = EASTLAllocatorType>
	class fixed_hashtable_allocator
	{
	public:
		typedef typename conditional<bEnableOverflow, fixed_pool_with_overflow<OverflowAllocator>, fixed_pool>::type                                 pool_type;
		typedef fixed_hashtable_allocator<bucketCount, nodeSize, nodeCount, nodeAlignment, nodeAlignmentOffset, bEnableOverflow, OverflowAllocator>  this_type;
		typedef OverflowAllocator overflow_allocator_type;

		enum
		{
			kBucketCount         = bucketCount + 1, // '+1' because the hash table needs a null terminating bucket.
			kBucketsSize         = bucketCount * sizeof(void*),
			kNodeSize            = nodeSize,
			kNodeCount           = nodeCount,
			kNodesSize           = nodeCount * nodeSize, // Note that the kBufferSize calculation assumes that the compiler sets sizeof(T) to be a multiple alignof(T), and so sizeof(T) is always >= alignof(T).
			kBufferSize          = kNodesSize + ((nodeAlignment > 1) ? nodeSize-1 : 0) + nodeAlignmentOffset, // Don't need to include kBucketsSize in this calculation, as fixed_hash_xxx containers have a separate buffer for buckets.
			kNodeAlignment       = nodeAlignment,
			kNodeAlignmentOffset = nodeAlignmentOffset,
			kAllocFlagBuckets    = 0x00400000               // Flag to allocator which indicates that we are allocating buckets and not nodes.
		};

	protected:
		pool_type mPool;
		void*     mpBucketBuffer;

	public:
		// Disabled because it causes compile conflicts.
		//fixed_hashtable_allocator(const char* pName)
		//{
		//    mPool.set_name(pName);
		//}

		fixed_hashtable_allocator(void* pNodeBuffer)
			: mPool(pNodeBuffer, kBufferSize, kNodeSize, kNodeAlignment, kNodeAlignmentOffset),
			  mpBucketBuffer(NULL)
		{
    __builtin_trap() /* STUB: not implemented */;
}


		fixed_hashtable_allocator(void* pNodeBuffer, const overflow_allocator_type& allocator)
			: mPool(pNodeBuffer, kBufferSize, kNodeSize, kNodeAlignment, kNodeAlignmentOffset, allocator),
			  mpBucketBuffer(NULL)
		{
    __builtin_trap() /* STUB: not implemented */;
}


		fixed_hashtable_allocator(void* pNodeBuffer, void* pBucketBuffer)
			: mPool(pNodeBuffer, kBufferSize, kNodeSize, kNodeAlignment, kNodeAlignmentOffset),
			  mpBucketBuffer(pBucketBuffer)
		{
    __builtin_trap() /* STUB: not implemented */;
}


		fixed_hashtable_allocator(void* pNodeBuffer, void* pBucketBuffer, const overflow_allocator_type& allocator)
			: mPool(pNodeBuffer, kBufferSize, kNodeSize, kNodeAlignment, kNodeAlignmentOffset, allocator),
			  mpBucketBuffer(pBucketBuffer)
		{
    __builtin_trap() /* STUB: not implemented */;
}


		/// fixed_hashtable_allocator
		///
		/// Note that we are copying x.mpHead and mpBucketBuffer to our own fixed_pool. 
		/// See the discussion above in fixed_node_allocator for important information about this.
		///
		fixed_hashtable_allocator(const this_type& x)
			: mPool(x.mPool.mpHead, kBufferSize, kNodeSize, kNodeAlignment, kNodeAlignmentOffset, x.mPool.mOverflowAllocator),
			  mpBucketBuffer(x.mpBucketBuffer)
		{
    __builtin_trap() /* STUB: not implemented */;
}


		fixed_hashtable_allocator& operator=(const fixed_hashtable_allocator& x)
		{
    __builtin_trap() /* STUB: not implemented */;
}


		void* allocate(size_t n, int flags = 0)
		{
    __builtin_trap() /* STUB: not implemented */;
}


		void* allocate(size_t n, size_t alignment, size_t offset, int flags = 0)
		{
    __builtin_trap() /* STUB: not implemented */;
}


		void deallocate(void* p, size_t)
		{
    __builtin_trap() /* STUB: not implemented */;
}


		bool can_allocate() const
		{
    __builtin_trap() /* STUB: not implemented */;
}


		void reset(void* pNodeBuffer)
		{
    __builtin_trap() /* STUB: not implemented */;
}


		const char* get_name() const
		{
    __builtin_trap() /* STUB: not implemented */;
}


		void set_name(const char* pName)
		{
    __builtin_trap() /* STUB: not implemented */;
}


		const overflow_allocator_type& get_overflow_allocator() const
		{
    __builtin_trap() /* STUB: not implemented */;
}


		overflow_allocator_type& get_overflow_allocator()
		{
    __builtin_trap() /* STUB: not implemented */;
}


		void set_overflow_allocator(const overflow_allocator_type& allocator)
		{
    __builtin_trap() /* STUB: not implemented */;
}


		void copy_overflow_allocator(const this_type& x)  // This function exists so we can write generic code that works for allocators that do and don't have overflow allocators.
		{
    __builtin_trap() /* STUB: not implemented */;
}

	}; // fixed_hashtable_allocator


	// This is a near copy of the code above, with the only difference being 
	// the 'false' bEnableOverflow template parameter, the pool_type and this_type typedefs, 
	// and the get_overflow_allocator / set_overflow_allocator functions.
	template <size_t bucketCount, size_t nodeSize, size_t nodeCount, size_t nodeAlignment, size_t nodeAlignmentOffset, typename OverflowAllocator>
	class fixed_hashtable_allocator<bucketCount, nodeSize, nodeCount, nodeAlignment, nodeAlignmentOffset, false, OverflowAllocator>
	{
	public:
		typedef fixed_pool pool_type;
		typedef fixed_hashtable_allocator<bucketCount, nodeSize, nodeCount, nodeAlignment, nodeAlignmentOffset, false, OverflowAllocator>  this_type;
		typedef OverflowAllocator overflow_allocator_type;

		enum
		{
			kBucketCount         = bucketCount + 1, // '+1' because the hash table needs a null terminating bucket.
			kBucketsSize         = bucketCount * sizeof(void*),
			kNodeSize            = nodeSize,
			kNodeCount           = nodeCount,
			kNodesSize           = nodeCount * nodeSize, // Note that the kBufferSize calculation assumes that the compiler sets sizeof(T) to be a multiple alignof(T), and so sizeof(T) is always >= alignof(T).
			kBufferSize          = kNodesSize + ((nodeAlignment > 1) ? nodeSize-1 : 0) + nodeAlignmentOffset, // Don't need to include kBucketsSize in this calculation, as fixed_hash_xxx containers have a separate buffer for buckets.
			kNodeAlignment       = nodeAlignment,
			kNodeAlignmentOffset = nodeAlignmentOffset,
			kAllocFlagBuckets    = 0x00400000               // Flag to allocator which indicates that we are allocating buckets and not nodes.
		};

	protected:
		pool_type mPool;
		void*     mpBucketBuffer;

	public:
		// Disabled because it causes compile conflicts.
		//fixed_hashtable_allocator(const char* pName)
		//{
		//    mPool.set_name(pName);
		//}

		fixed_hashtable_allocator(void* pNodeBuffer)
			: mPool(pNodeBuffer, kBufferSize, kNodeSize, kNodeAlignment, kNodeAlignmentOffset),
			  mpBucketBuffer(NULL)
		{
    __builtin_trap() /* STUB: not implemented */;
}

		fixed_hashtable_allocator(void* pNodeBuffer, const overflow_allocator_type& /*allocator*/) // allocator is unused because bEnableOverflow is false in this specialization.
			: mPool(pNodeBuffer, kBufferSize, kNodeSize, kNodeAlignment, kNodeAlignmentOffset),
			  mpBucketBuffer(NULL)
		{
    __builtin_trap() /* STUB: not implemented */;
}


		fixed_hashtable_allocator(void* pNodeBuffer, void* pBucketBuffer)
			: mPool(pNodeBuffer, kBufferSize, kNodeSize, kNodeAlignment, kNodeAlignmentOffset),
			  mpBucketBuffer(pBucketBuffer)
		{
    __builtin_trap() /* STUB: not implemented */;
}


		fixed_hashtable_allocator(void* pNodeBuffer, void* pBucketBuffer, const overflow_allocator_type& /*allocator*/) // allocator is unused because bEnableOverflow is false in this specialization.
			: mPool(pNodeBuffer, kBufferSize, kNodeSize, kNodeAlignment, kNodeAlignmentOffset),
			  mpBucketBuffer(pBucketBuffer)
		{
    __builtin_trap() /* STUB: not implemented */;
}


		/// fixed_hashtable_allocator
		///
		/// Note that we are copying x.mpHead and mpBucketBuffer to our own fixed_pool. 
		/// See the discussion above in fixed_node_allocator for important information about this.
		///
		fixed_hashtable_allocator(const this_type& x)   // No need to copy the overflow allocator, because bEnableOverflow is false in this specialization.
			: mPool(x.mPool.mpHead, kBufferSize, kNodeSize, kNodeAlignment, kNodeAlignmentOffset),
			  mpBucketBuffer(x.mpBucketBuffer)
		{
    __builtin_trap() /* STUB: not implemented */;
}


		fixed_hashtable_allocator& operator=(const fixed_hashtable_allocator& x)
		{
    __builtin_trap() /* STUB: not implemented */;
}


		void* allocate(size_t n, int flags = 0)
		{
    __builtin_trap() /* STUB: not implemented */;
}


		void* allocate(size_t n, size_t alignment, size_t offset, int flags = 0)
		{
    __builtin_trap() /* STUB: not implemented */;
}


		void deallocate(void* p, size_t)
		{
    __builtin_trap() /* STUB: not implemented */;
}


		bool can_allocate() const
		{
    __builtin_trap() /* STUB: not implemented */;
}


		void reset(void* pNodeBuffer)
		{
    __builtin_trap() /* STUB: not implemented */;
}


		const char* get_name() const
		{
    __builtin_trap() /* STUB: not implemented */;
}


		void set_name(const char* pName)
		{
    __builtin_trap() /* STUB: not implemented */;
}


		const overflow_allocator_type& get_overflow_allocator() const EA_NOEXCEPT
		{
    __builtin_trap() /* STUB: not implemented */;
}


		overflow_allocator_type& get_overflow_allocator() EA_NOEXCEPT
		{
    __builtin_trap() /* STUB: not implemented */;
}

		void set_overflow_allocator(const overflow_allocator_type& /*allocator*/)
		{
    __builtin_trap() /* STUB: not implemented */;
}

		void copy_overflow_allocator(const this_type&)  // This function exists so we can write generic code that works for allocators that do and don't have overflow allocators.
		{
    __builtin_trap() /* STUB: not implemented */;
}

	}; // fixed_hashtable_allocator


	///////////////////////////////////////////////////////////////////////
	// global operators
	///////////////////////////////////////////////////////////////////////

	template <size_t bucketCount, size_t nodeSize, size_t nodeCount, size_t nodeAlignment, size_t nodeAlignmentOffset, bool bEnableOverflow, typename OverflowAllocator>
	inline bool operator==(const fixed_hashtable_allocator<bucketCount, nodeSize, nodeCount, nodeAlignment, nodeAlignmentOffset, bEnableOverflow, OverflowAllocator>& a, 
						   const fixed_hashtable_allocator<bucketCount, nodeSize, nodeCount, nodeAlignment, nodeAlignmentOffset, bEnableOverflow, OverflowAllocator>& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <size_t bucketCount, size_t nodeSize, size_t nodeCount, size_t nodeAlignment, size_t nodeAlignmentOffset, bool bEnableOverflow, typename OverflowAllocator>
	inline bool operator!=(const fixed_hashtable_allocator<bucketCount, nodeSize, nodeCount, nodeAlignment, nodeAlignmentOffset, bEnableOverflow, OverflowAllocator>& a, 
						   const fixed_hashtable_allocator<bucketCount, nodeSize, nodeCount, nodeAlignment, nodeAlignmentOffset, bEnableOverflow, OverflowAllocator>& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}






	///////////////////////////////////////////////////////////////////////////
	// fixed_vector_allocator
	///////////////////////////////////////////////////////////////////////////

	/// fixed_vector_allocator
	///
	/// Template parameters:
	///     nodeSize               The size of individual objects.
	///     nodeCount              The number of objects the pool contains.
	///     nodeAlignment          The alignment of the objects to allocate.
	///     nodeAlignmentOffset    The alignment offset of the objects to allocate.
	///     bEnableOverflow        Whether or not we should use the overflow heap if our object pool is exhausted.
	///     OverflowAllocator      Overflow allocator, which is only used if bEnableOverflow == true. Defaults to the global heap.
	///
	template <size_t nodeSize, size_t nodeCount, size_t nodeAlignment, size_t nodeAlignmentOffset, bool bEnableOverflow, typename OverflowAllocator = EASTLAllocatorType>
	class fixed_vector_allocator
	{
	public:
		typedef fixed_vector_allocator<nodeSize, nodeCount, nodeAlignment, nodeAlignmentOffset, bEnableOverflow, OverflowAllocator>  this_type;
		typedef OverflowAllocator overflow_allocator_type;

		enum
		{
			kNodeSize            = nodeSize,
			kNodeCount           = nodeCount,
			kNodesSize           = nodeCount * nodeSize, // Note that the kBufferSize calculation assumes that the compiler sets sizeof(T) to be a multiple alignof(T), and so sizeof(T) is always >= alignof(T).
			kBufferSize          = kNodesSize + ((nodeAlignment > 1) ? nodeSize-1 : 0) + nodeAlignmentOffset,
			kNodeAlignment       = nodeAlignment,
			kNodeAlignmentOffset = nodeAlignmentOffset
		};

	public:
		overflow_allocator_type mOverflowAllocator;
		void*                   mpPoolBegin;         // To consider: Find some way to make this data unnecessary, without increasing template proliferation.

	public:
		// Disabled because it causes compile conflicts.
		//fixed_vector_allocator(const char* pName = NULL)
		//{
		//    mOverflowAllocator.set_name(pName);
		//}

		fixed_vector_allocator(void* pNodeBuffer = nullptr)
			: mpPoolBegin(pNodeBuffer)
		{
    __builtin_trap() /* STUB: not implemented */;
}

		fixed_vector_allocator(void* pNodeBuffer, const overflow_allocator_type& allocator)
			: mOverflowAllocator(allocator), mpPoolBegin(pNodeBuffer)
		{
    __builtin_trap() /* STUB: not implemented */;
}

		fixed_vector_allocator(const fixed_vector_allocator& x)
			: mOverflowAllocator(x.mOverflowAllocator), mpPoolBegin(x.mpPoolBegin)
		{
    __builtin_trap() /* STUB: not implemented */;
}

		fixed_vector_allocator& operator=(const fixed_vector_allocator& x)
		{
    __builtin_trap() /* STUB: not implemented */;
}

		void* allocate(size_t n, int flags = 0)
		{
    __builtin_trap() /* STUB: not implemented */;
}

		void* allocate(size_t n, size_t alignment, size_t offset, int flags = 0)
		{
    __builtin_trap() /* STUB: not implemented */;
}

		void deallocate(void* p, size_t n)
		{
    __builtin_trap() /* STUB: not implemented */;
}

		const char* get_name() const
		{
    __builtin_trap() /* STUB: not implemented */;
}

		void set_name(const char* pName)
		{
    __builtin_trap() /* STUB: not implemented */;
}

		const overflow_allocator_type& get_overflow_allocator() const EA_NOEXCEPT
		{
    __builtin_trap() /* STUB: not implemented */;
}

		overflow_allocator_type& get_overflow_allocator() EA_NOEXCEPT
		{
    __builtin_trap() /* STUB: not implemented */;
}

		void set_overflow_allocator(const overflow_allocator_type& allocator)
		{
    __builtin_trap() /* STUB: not implemented */;
}

		void copy_overflow_allocator(const this_type& x)  // This function exists so we can write generic code that works for allocators that do and don't have overflow allocators.
		{
    __builtin_trap() /* STUB: not implemented */;
}

	}; // fixed_vector_allocator


	template <size_t nodeSize, size_t nodeCount, size_t nodeAlignment, size_t nodeAlignmentOffset, typename OverflowAllocator>
	class fixed_vector_allocator<nodeSize, nodeCount, nodeAlignment, nodeAlignmentOffset, false, OverflowAllocator>
	{
	public:
		typedef fixed_vector_allocator<nodeSize, nodeCount, nodeAlignment, nodeAlignmentOffset, false, OverflowAllocator>  this_type;
		typedef OverflowAllocator overflow_allocator_type;

		enum
		{
			kNodeSize            = nodeSize,
			kNodeCount           = nodeCount,
			kNodesSize           = nodeCount * nodeSize, // Note that the kBufferSize calculation assumes that the compiler sets sizeof(T) to be a multiple alignof(T), and so sizeof(T) is always >= alignof(T).
			kBufferSize          = kNodesSize + ((nodeAlignment > 1) ? nodeSize-1 : 0) + nodeAlignmentOffset,
			kNodeAlignment       = nodeAlignment,
			kNodeAlignmentOffset = nodeAlignmentOffset
		};

		// Disabled because it causes compile conflicts.
		//fixed_vector_allocator(const char* = NULL) // This char* parameter is present so that this class can be like the other version.
		//{
		//}

		fixed_vector_allocator()
		{
    __builtin_trap() /* STUB: not implemented */;
}

		fixed_vector_allocator(void* /*pNodeBuffer*/)
		{
    __builtin_trap() /* STUB: not implemented */;
}

		fixed_vector_allocator(void* /*pNodeBuffer*/, const overflow_allocator_type& /*allocator*/)  // allocator is unused because bEnableOverflow is false in this specialization.
		{
    __builtin_trap() /* STUB: not implemented */;
}

		/// fixed_vector_allocator
		///
		// Disabled because there is nothing to do. No member data. And the default for this is sufficient.
		// fixed_vector_allocator(const fixed_vector_allocator&)
		// {
		// }

		// Disabled because there is nothing to do. No member data.
		//fixed_vector_allocator& operator=(const fixed_vector_allocator& x)
		//{
		//    return *this;
		//}

		void* allocate(size_t /*n*/, int /*flags*/ = 0)
		{
    __builtin_trap() /* STUB: not implemented */;
}

		void* allocate(size_t /*n*/, size_t /*alignment*/, size_t /*offset*/, int /*flags*/ = 0)
		{
    __builtin_trap() /* STUB: not implemented */;
}

		void deallocate(void* /*p*/, size_t /*n*/)
		{
    __builtin_trap() /* STUB: not implemented */;
}

		const char* get_name() const
		{
    __builtin_trap() /* STUB: not implemented */;
}

		void set_name(const char* /*pName*/)
		{
    __builtin_trap() /* STUB: not implemented */;
}

		const overflow_allocator_type& get_overflow_allocator() const EA_NOEXCEPT
		{
    __builtin_trap() /* STUB: not implemented */;
}

		overflow_allocator_type& get_overflow_allocator() EA_NOEXCEPT
		{
    __builtin_trap() /* STUB: not implemented */;
}

		void set_overflow_allocator(const overflow_allocator_type& /*allocator*/)
		{
    __builtin_trap() /* STUB: not implemented */;
}

		void copy_overflow_allocator(const this_type&)  // This function exists so we can write generic code that works for allocators that do and don't have overflow allocators.
		{
    __builtin_trap() /* STUB: not implemented */;
}

	}; // fixed_vector_allocator


	///////////////////////////////////////////////////////////////////////
	// global operators
	///////////////////////////////////////////////////////////////////////

	template <size_t nodeSize, size_t nodeCount, size_t nodeAlignment, size_t nodeAlignmentOffset, bool bEnableOverflow, typename OverflowAllocator>
	inline bool operator==(const fixed_vector_allocator<nodeSize, nodeCount, nodeAlignment, nodeAlignmentOffset, bEnableOverflow, OverflowAllocator>& a, 
						   const fixed_vector_allocator<nodeSize, nodeCount, nodeAlignment, nodeAlignmentOffset, bEnableOverflow, OverflowAllocator>& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <size_t nodeSize, size_t nodeCount, size_t nodeAlignment, size_t nodeAlignmentOffset, bool bEnableOverflow, typename OverflowAllocator>
	inline bool operator!=(const fixed_vector_allocator<nodeSize, nodeCount, nodeAlignment, nodeAlignmentOffset, bEnableOverflow, OverflowAllocator>& a, 
						   const fixed_vector_allocator<nodeSize, nodeCount, nodeAlignment, nodeAlignmentOffset, bEnableOverflow, OverflowAllocator>& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}





	///////////////////////////////////////////////////////////////////////////
	// fixed_swap
	///////////////////////////////////////////////////////////////////////////

	/// fixed_swap
	///
	/// This function implements a swap suitable for fixed containers.
	/// This is an issue because the size of fixed containers can be very 
	/// large, due to their having the container buffer within themselves.
	/// Note that we are referring to sizeof(container) and not the total
	/// sum of memory allocated by the container from the heap. 
	///
	///
	/// This implementation switches at compile time whether or not the 
	/// temporary is allocated on the stack or the heap as some compilers
	/// will allocate the (large) stack frame regardless of which code
	/// path is picked.
	template <typename Container, bool UseHeapTemporary>
	class fixed_swap_impl
	{
	public: 
		static void swap(Container& a, Container& b);
	};


	template <typename Container>
	class fixed_swap_impl<Container, false>
	{
	public:
		static void swap(Container& a, Container& b)
		{
    __builtin_trap() /* STUB: not implemented */;
}
	};


	template <typename Container>
	class fixed_swap_impl<Container, true>
	{
	public:
		static void swap(Container& a, Container& b)
		{
    __builtin_trap() /* STUB: not implemented */;
}
	};


	template<typename Container>
	void fixed_swap(Container& a, Container& b)
	{
    __builtin_trap() /* STUB: not implemented */;
}



} // namespace eastl


EA_RESTORE_VC_WARNING();


#endif // Header include guard
