#pragma once
#include <filesystem>

namespace Mlib {

std::filesystem::path weakly_canonical_preserve_symlinks(const std::filesystem::path& path);

}
