#pragma once
#include <Mlib/Misc/Pragma_Gcc.hpp>

namespace Mlib {

struct Uninitialized {};

PRAGMA_GCC_DIAGNOSTIC_PUSH
PRAGMA_GCC_DIAGNOSTIC_IGNORED(-Wunused-variable)
static Uninitialized uninitialized;
PRAGMA_GCC_DIAGNOSTIC_POP

}
