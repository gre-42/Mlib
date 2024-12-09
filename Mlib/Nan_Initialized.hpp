#pragma once

namespace Mlib {

struct NanInitialized {};

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
static NanInitialized nan_initialized;
#pragma GCC diagnostic pop

}
