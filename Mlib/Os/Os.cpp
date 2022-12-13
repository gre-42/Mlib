#include "Os.hpp"
#include <Mlib/Throw_Or_Abort.hpp>
#include <sstream>

#ifdef __ANDROID__
#include <Mlib/Android/ndk_helper/AUi.hpp>
#include <NDKHelper.h>
#else
#include <Mlib/Env.hpp>
#include <fstream>
#include <filesystem>
#include <iostream>
#endif

namespace fs = std::filesystem;

using namespace Mlib;

LInfo Mlib::linfo() {
    return {};
}

LWarn Mlib::lwarn() {
    return {};
}

LErr Mlib::lerr() {
    return {};
}

#ifdef __ANDROID__

LInfo::~LInfo() {
    LOGI("%s", str().c_str());
};

LWarn::~LWarn() {
    LOGW("%s", str().c_str());
};

LErr::~LErr() {
    LOGE("%s", str().c_str());
};

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

std::string Mlib::get_path_in_external_files_dir(const std::initializer_list<std::string>& child_path) {
    fs::path res = AUi::GetExternalFilesDir();
    for (const auto& s : child_path) {
        res /= fs::path(s);
    }
    return res.string();
}

ndk_helper::DirectoryIterator Mlib::list_dir(const std::filesystem::path& path) {
    return AUi::ListDir(path.c_str());
}

#else

static std::string g_app_reldir;

LInfo::~LInfo() {
    std::cerr << "Info: " << str() << std::endl;
};

LWarn::~LWarn() {
    std::cerr << "Warning: " << str() << std::endl;
};

LErr::~LErr() {
    std::cerr << "Error: " << str() << std::endl;
};

std::unique_ptr<std::istream> Mlib::create_ifstream(
    const std::string& filename,
    std::ios_base::openmode mode)
{
    return std::make_unique<std::ifstream>(filename, mode);
}

std::vector<uint8_t> Mlib::read_file_bytes(const std::string& filename) {
    std::ifstream f{filename, std::ios::binary};
    if (f.fail()) {
        throw std::runtime_error("Could not open file for read: \"" + filename + '"');
    }
    f.seekg(0, std::ifstream::end);
    size_t file_size = f.tellg();
    f.seekg(0, std::ifstream::beg);
    std::vector<uint8_t> res;
    res.reserve(file_size);
    res.assign(std::istreambuf_iterator<char>(f),
               std::istreambuf_iterator<char>());
    if (f.fail() && !f.eof()) {
        THROW_OR_ABORT("Could not read from file: \"" + filename + '"');
    }
    return res;
}

bool Mlib::path_exists(const std::string& filename) {
    return fs::exists(filename);
}

void Mlib::set_app_reldir(const std::string& app_reldir) {
    if (!g_app_reldir.empty()) {
        THROW_OR_ABORT("App reldir already set");
    }
    if (app_reldir.empty()) {
        THROW_OR_ABORT("Trying to set empty app reldir");
    }
    g_app_reldir = app_reldir;
}

std::string get_path_in_external_files_dir(const std::initializer_list<std::string>& child_path) {
    if (g_app_reldir.empty()) {
        THROW_OR_ABORT("Relative app dir is empty");
    }
    return get_path_in_home_directory(g_app_reldir, child_path);
}

std::filesystem::directory_iterator Mlib::list_dir(const std::filesystem::path& path) {
    return std::filesystem::directory_iterator(path);
}

#endif

void Mlib::verbose_abort(const std::string& message) {
    lerr() << "Aborting: " << message;
    std::abort();
}
