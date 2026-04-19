#pragma once
#include <Mlib/Misc/Pragma_Gcc.hpp>

namespace Mlib {

struct Uninitialized {};

PRAGMA_GCC(diagnostic push)
PRAGMA_GCC(diagnostic ignored "-Wunused-variable")
static Uninitialized uninitialized;
PRAGMA_GCC(diagnostic pop)

}
