#pragma once
#include <Mlib/Strings/Utf8_Path.hpp>

namespace Mlib {

Utf8Path weakly_canonical_preserve_symlinks(const Utf8Path& path);

}
