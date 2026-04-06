#include "Weakly_Canonical_Preserve_Symlinks.hpp"

using namespace Mlib;

std::filesystem::path Mlib::weakly_canonical_preserve_symlinks(const std::filesystem::path& path) {
    return std::filesystem::absolute(path).lexically_normal();
}
