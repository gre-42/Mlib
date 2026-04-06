#pragma once

namespace Mlib {

struct Uninitialized {};

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
static Uninitialized uninitialized;
#pragma GCC diagnostic pop

}
