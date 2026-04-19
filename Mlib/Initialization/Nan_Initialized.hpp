#pragma once
#include <Mlib/Misc/Pragma_Gcc.hpp>

namespace Mlib {

struct NanInitialized {};

PRAGMA_GCC_DIAGNOSTIC_PUSH
PRAGMA_GCC_DIAGNOSTIC_IGNORED(-Wunused-variable)
static NanInitialized nan_initialized;
PRAGMA_GCC_DIAGNOSTIC_POP

}
