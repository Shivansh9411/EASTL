#include <stdexcept>
#include <cstdlib>
///////////////////////////////////////////////////////////////////////////////
// Copyright (c) Electronic Arts Inc. All rights reserved.
///////////////////////////////////////////////////////////////////////////////


#include <EASTL/internal/fixed_pool.h>
#include <EASTL/fixed_allocator.h>



namespace eastl
{


	EASTL_API void fixed_pool_base::init(void* pMemory, size_t memorySize, size_t nodeSize,
							   size_t alignment, size_t /*alignmentOffset*/)
	{
    __builtin_trap() /* STUB: not implemented */;
}


} // namespace eastl

















