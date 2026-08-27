#include <stdexcept>
#include <cstdlib>
/////////////////////////////////////////////////////////////////////////////////
// Copyright (c) Electronic Arts Inc. All rights reserved.
/////////////////////////////////////////////////////////////////////////////////


#include <EASTL/atomic.h>


namespace eastl
{

namespace internal
{


static void EastlCompilerBarrierDataDependencyFunc(void*)
{
    __builtin_trap() /* STUB: not implemented */;
}

volatile CompilerBarrierDataDependencyFuncPtr gCompilerBarrierDataDependencyFunc = &EastlCompilerBarrierDataDependencyFunc;


} // namespace internal

} // namespace eastl
