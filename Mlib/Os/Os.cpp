#include "Os.hpp"
#include <Mlib/Throw_Or_Abort.hpp>
#include <filesystem>
#include <fstream>
#include <sstream>

#ifdef __ANDROID__
#include <Mlib/Android/ndk_helper/AUi.hpp>
#include <NDKHelper.h>
#else
#include <Mlib/Env.hpp>
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

static std::string get_path_in_external_files_dir(const std::initializer_list<std::string>& child_path) {
    std::string res = AUi::GetExternalFilesDir();
    for (const auto& s : child_path) {
        res += '/' + s;
    }
    return std::filesystem::weakly_canonical(res);
}

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

std::unique_ptr<std::ostream> Mlib::create_ofstream(
    const std::string& filename,
    std::ios_base::openmode mode)
{
    auto fn = get_path_in_external_files_dir({filename});
    LOGI("Opening file \"%s\" for writing", fn.c_str());
    return std::make_unique<std::ofstream>(fn, mode);
}

bool Mlib::path_exists(const std::string& filename) {
    return AUi::PathExists(filename);
}

void Mlib::remove_path(const std::string& path) {
    std::error_code ec;
    if (!fs::remove(get_path_in_external_files_dir({path}), ec)) {
        THROW_OR_ABORT("Could not delete path \"" + path + "\". " + ec.message());
    }
}

void Mlib::rename_path(const std::string& from, const std::string& to) {
    std::error_code ec;
    fs::rename(
        get_path_in_external_files_dir({from}),
        get_path_in_external_files_dir({to}),
        ec);
    if (ec) {
        THROW_OR_ABORT("Could not rename path \"" + from + "\" to " + to + ". " + ec.message());
    }
}

void Mlib::create_directories(const std::string& dirname) {
    std::error_code ec;
    fs::create_directories(get_path_in_external_files_dir({dirname}), ec);
    if (ec) {
        THROW_OR_ABORT("Could not create directories \"" + dirname + "\". " + ec.message());
    }
}

ndk_helper::DirectoryIterator Mlib::list_dir(const std::filesystem::path& path) {
    return AUi::ListDir(path.c_str());
}

#else

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
        THROW_OR_ABORT("Could not open file for read: \"" + filename + '"');
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

std::unique_ptr<std::ostream> Mlib::create_ofstream(
    const std::string& filename,
    std::ios_base::openmode mode)
{
    return std::make_unique<std::ofstream>(filename, mode);
}

bool Mlib::path_exists(const std::string& path) {
    std::error_code ec;
    bool exists = fs::exists(path, ec);
    if (ec) {
        THROW_OR_ABORT("Could not check if path \"" + path + "\" exists. " + ec.message());
    }
    return exists;
}

void Mlib::remove_path(const std::string& path) {
    std::error_code ec;
    if (!fs::remove(path, ec)) {
        THROW_OR_ABORT("Could not delete path \"" + path + "\". " + ec.message());
    }
}

void Mlib::rename_path(const std::string& from, const std::string& to) {
    std::error_code ec;
    fs::rename(from, to, ec);
    if (ec) {
        THROW_OR_ABORT("Could not rename path \"" + from + "\" to " + to + ". " + ec.message());
    }
}

void Mlib::create_directories(const std::string& dirname) {
    std::error_code ec;
    fs::create_directories(dirname, ec);
    if (ec) {
        THROW_OR_ABORT("Could not create directories \"" + dirname + "\". " + ec.message());
    }
}

std::filesystem::directory_iterator Mlib::list_dir(const std::filesystem::path& path) {
    return std::filesystem::directory_iterator(path);
}

#endif

void Mlib::verbose_abort(const std::string& message) {
    lerr() << "Aborting: " << message;
    std::abort();
}
