#pragma once
#include <cstdint>
#include <istream>
#include <memory>
#include <sstream>
#include <vector>

#ifdef __ANDROID__
#include <NDKHelper.h>
#else
#include <filesystem>
#endif

namespace Mlib {

class LInfo: public std::ostringstream {
public:
    ~LInfo() override;
};

class LWarn: public std::ostringstream {
public:
    ~LWarn() override;
};

class LErr: public std::ostringstream {
public:
    ~LErr() override;
};

LInfo linfo();

LWarn lwarn();

LErr lerr();

std::unique_ptr<std::istream> create_ifstream(
    const std::string& filename,
    std::ios_base::openmode mode = std::ios_base::in);

std::vector<uint8_t> read_file_bytes(const std::string& filename);

bool path_exists(const std::string& filename);

#ifndef __ANDROID__
void set_app_reldir(const std::string& app_reldir);
#endif

std::string get_path_in_external_files_dir(const std::initializer_list<std::string>& child_path);

#ifdef __ANDROID__
ndk_helper::DirectoryIterator list_dir(const std::filesystem::path& path);
#else
std::filesystem::directory_iterator list_dir(const std::filesystem::path& path);
#endif

[[ noreturn ]] void verbose_abort(const std::string& message);

}