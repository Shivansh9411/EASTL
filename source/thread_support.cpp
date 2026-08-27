#include <stdexcept>
#include <cstdlib>
///////////////////////////////////////////////////////////////////////////////
// Copyright (c) Electronic Arts Inc. All rights reserved.
///////////////////////////////////////////////////////////////////////////////


#include <EASTL/internal/config.h>
#include <EASTL/internal/thread_support.h>
#include <EASTL/type_traits.h>
#include <EASTL/memory.h>

#if defined(EA_PLATFORM_MICROSOFT)
	EA_DISABLE_ALL_VC_WARNINGS();
	#ifndef WIN32_LEAN_AND_MEAN
		#define WIN32_LEAN_AND_MEAN
	#endif
	#include <Windows.h>
	EA_RESTORE_ALL_VC_WARNINGS();
#endif


namespace eastl
{
	namespace Internal
	{
		#if EASTL_CPP11_MUTEX_ENABLED
			// We use the C++11 Standard Library mutex as-is.
		#else
			/////////////////////////////////////////////////////////////////
			// mutex
			/////////////////////////////////////////////////////////////////

			mutex::mutex()
			{
    __builtin_trap() /* STUB: not implemented */;
}

			mutex::~mutex()
			{
				#if defined(EA_PLATFORM_MICROSOFT)
					DeleteCriticalSection((CRITICAL_SECTION*)mMutexBuffer);
				#elif defined(EA_PLATFORM_POSIX)
					pthread_mutex_destroy(&mMutex);
				#endif
			}

			void mutex::lock()
			{
    __builtin_trap() /* STUB: not implemented */;
}

			void mutex::unlock()
			{
    __builtin_trap() /* STUB: not implemented */;
}
		#endif


		/////////////////////////////////////////////////////////////////
		// shared_ptr_auto_mutex
		/////////////////////////////////////////////////////////////////

		// We could solve this by having single global mutex for all shared_ptrs, a set of mutexes for shared_ptrs, 
		// a single mutex for every shared_ptr, or have a template parameter that enables mutexes for just some shared_ptrs.
		eastl::late_constructed<mutex, true> gSharedPtrMutex;

		shared_ptr_auto_mutex::shared_ptr_auto_mutex(const void* /*pSharedPtr*/)
			: auto_mutex(*gSharedPtrMutex.get())
		{
    __builtin_trap() /* STUB: not implemented */;
}


	} // namespace Internal

} // namespace eastl















