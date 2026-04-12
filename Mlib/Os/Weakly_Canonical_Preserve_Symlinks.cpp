#include "Weakly_Canonical_Preserve_Symlinks.hpp"

using namespace Mlib;

Utf8Path Mlib::weakly_canonical_preserve_symlinks(const Utf8Path& path) {
    return path.absolute().lexically_normal();
}
