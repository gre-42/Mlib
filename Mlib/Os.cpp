#include "Os.hpp"

#ifdef __ANDROID__
#include <Mlib/Android/ndk_helper/AUi.hpp>
#include <NDKHelper.h>
#else
#include <fstream>
#include <filesystem>
#include <iostream>
#endif

namespace fs = std::filesystem;

using namespace Mlib;

#ifdef __ANDROID__
std::unique_ptr<std::istream> Mlib::create_ifstream(
    const std::string& filename,
    std::ios_base::openmode mode)
{
    return AUi::OpenFile(filename);
}

bool Mlib::file_exists(const std::string& filename) {
    return AUi::FileExists(filename);
}

void Mlib::verbose_abort(const std::string& message) {
    LOGE("Aborting: %s", message.c_str());
    std::abort();
}
#else
std::unique_ptr<std::istream> Mlib::create_ifstream(
    const std::string& filename,
    std::ios_base::openmode mode)
{
    return std::make_unique<std::ifstream>(filename, mode);
}

bool Mlib::file_exists(const std::string& filename) {
    return fs::exists(filename);
}

void Mlib::verbose_abort(const std::string& message) {
    std::cerr << "Aborting: " << message.c_str() << std::endl;
    std::abort();
}
#endif
