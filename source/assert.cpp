#include <stdexcept>
#include <cstdlib>
///////////////////////////////////////////////////////////////////////////////
// Copyright (c) Electronic Arts Inc. All rights reserved.
///////////////////////////////////////////////////////////////////////////////


#include <EASTL/internal/config.h>
#include <EASTL/string.h>
#include <EABase/eabase.h>

#if defined(EA_PLATFORM_WINDOWS_KERNEL)
	#include <Wdm.h>
#elif defined(EA_PLATFORM_MICROSOFT)
	EA_DISABLE_ALL_VC_WARNINGS();
	#if defined(EA_COMPILER_MSVC)
		#include <crtdbg.h>
	#endif
	#ifndef WIN32_LEAN_AND_MEAN
		#define WIN32_LEAN_AND_MEAN
	#endif
	#include <Windows.h>
	EA_RESTORE_ALL_VC_WARNINGS();
#elif defined(EA_PLATFORM_ANDROID)
	#include <android/log.h>
#else
	#include <stdio.h>
#endif




namespace eastl
{
	void AssertionFailureFunctionDefault(void* instructionPointer, const char* pExpression, void* pContext);

	/// gpAssertionFailureFunction
	///
	/// Global assertion failure function pointer. Set by SetAssertionFailureFunction.
	///
	EASTL_API EASTL_AssertionFailureFunctionEx gpAssertionFailureFunction        = AssertionFailureFunctionDefault;
	EASTL_API void*                          gpAssertionFailureFunctionContext = NULL;



	/// SetAssertionFailureFunction
	///
	/// Sets the function called when an assertion fails. If this function is not called
	/// by the user, a default function will be used. The user may supply a context parameter
	/// which will be passed back to the user in the function call. This is typically used
	/// to store a C++ 'this' pointer, though other things are possible.
	///
	/// There is no thread safety here, so the user needs to externally make sure that
	/// this function is not called in a thread-unsafe way. The easiest way to do this is
	/// to just call this function once from the main thread on application startup.
	///
	EASTL_API void SetAssertionFailureFunction(EASTL_AssertionFailureFunction pAssertionFailureFunction, void* pContext)
	{
    __builtin_trap() /* STUB: not implemented */;
}
	EASTL_API void SetAssertionFailureFunction(EASTL_AssertionFailureFunctionEx pAssertionFailureFunction, void* pContext)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	/// AssertionFailureFunctionDefault
	///
	void AssertionFailureFunctionDefault(void* instructionPointer, const char* pExpression, void* pContext)
	{
    __builtin_trap() /* STUB: not implemented */;
}
	EASTL_API void AssertionFailureFunctionDefault(const char* pExpression, void* /*pContext*/)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	/// AssertionFailure
	///
	EASTL_API void AssertionFailure(const char* pExpression)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	EASTL_API void AssertionFailure(void* instructionPointer, const char* pExpression)
	{
    __builtin_trap() /* STUB: not implemented */;
}


} // namespace eastl
