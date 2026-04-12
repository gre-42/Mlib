#pragma once
#include <Mlib/Os/Utf8_Path.hpp>

namespace Mlib {

Utf8Path weakly_canonical_preserve_symlinks(const Utf8Path& path);

}
