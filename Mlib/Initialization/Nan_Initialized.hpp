#pragma once
#include <Mlib/Misc/Pragma_Gcc.hpp>

namespace Mlib {

struct NanInitialized {};

PRAGMA_GCC(diagnostic push)
PRAGMA_GCC(diagnostic ignored "-Wunused-variable")
static NanInitialized nan_initialized;
PRAGMA_GCC(diagnostic pop)

}
