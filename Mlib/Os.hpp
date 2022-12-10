#pragma once
#include <istream>
#include <memory>

#ifdef __ANDROID__
#include <NDKHelper.h>
#else
#include <filesystem>
#endif

namespace Mlib {

std::unique_ptr<std::istream> create_ifstream(
    const std::string& filename,
    std::ios_base::openmode mode = std::ios_base::in);

bool path_exists(const std::string& filename);

#ifdef __ANDROID__
ndk_helper::DirectoryIterator list_dir(const std::filesystem::path& path);
#else
std::filesystem::directory_iterator list_dir(const std::filesystem::path& path);
#endif

void verbose_abort(const std::string& message);

}
