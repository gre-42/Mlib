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

std::vector<uint8_t> Mlib::read_file_bytes(const std::string& filename) {
    return AUi::ReadFile(filename);
}

bool Mlib::path_exists(const std::string& filename) {
    return AUi::PathExists(filename);
}

ndk_helper::DirectoryIterator Mlib::list_dir(const std::filesystem::path& path) {
    return AUi::ListDir(path.c_str());
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

std::vector<uint8_t> Mlib::read_file_bytes(const std::string& filename) {
    // https://stackoverflow.com/a/26589421/2292832
    std::basic_ifstream<uint8_t> f{filename, std::ios::binary};
    if (f.fail()) {
        throw std::runtime_error("Could not open file for read: \"" + filename + '"');
    }
    auto eos = std::istreambuf_iterator<uint8_t>();
    auto res = std::vector<uint8_t>(std::istreambuf_iterator<uint8_t>(f), eos);
    if (f.fail() && !f.eof()) {
        throw std::runtime_error("Could not read from file: \"" + filename + '"');
    }
    return res;
}

bool Mlib::path_exists(const std::string& filename) {
    return fs::exists(filename);
}

std::filesystem::directory_iterator Mlib::list_dir(const std::filesystem::path& path) {
    return std::filesystem::directory_iterator(path);
}

void Mlib::verbose_abort(const std::string& message) {
    std::cerr << "Aborting: " << message.c_str() << std::endl;
    std::abort();
}
#endif
