#include <stdexcept>
#include <cstdlib>
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) Electronic Arts Inc. All rights reserved.
/////////////////////////////////////////////////////////////////////////////

#ifndef EASTL_INTERNAL_FUNCTION_HELP_H
#define EASTL_INTERNAL_FUNCTION_HELP_H

#if defined(EA_PRAGMA_ONCE_SUPPORTED)
	#pragma once
#endif

#include <EASTL/internal/config.h>
#include <EASTL/type_traits.h>

namespace eastl
{
	namespace internal
	{

		//////////////////////////////////////////////////////////////////////
		// is_null
		//
		template <typename T>
		bool is_null(const T&)
		{
    __builtin_trap() /* STUB: not implemented */;
}

		template <typename Result, typename... Arguments>
		bool is_null(Result (*const& function_pointer)(Arguments...))
		{
    __builtin_trap() /* STUB: not implemented */;
}

		template <typename Result, typename Class, typename... Arguments>
		bool is_null(Result (Class::*const& function_pointer)(Arguments...))
		{
    __builtin_trap() /* STUB: not implemented */;
}

		template <typename Result, typename Class, typename... Arguments>
		bool is_null(Result (Class::*const& function_pointer)(Arguments...) const)
		{
    __builtin_trap() /* STUB: not implemented */;
}

	} // namespace internal
} // namespace eastl

#endif // Header include guard

