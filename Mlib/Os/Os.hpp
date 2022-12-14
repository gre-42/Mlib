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

std::unique_ptr<std::ostream> create_ofstream(
    const std::string& filename,
    std::ios_base::openmode mode = std::ios_base::out);

std::vector<uint8_t> read_file_bytes(const std::string& filename);

bool path_exists(const std::string& filename);

void remove_path(const std::string& path);

void rename_path(const std::string& from, const std::string& to);

void create_directories(const std::string& dirname);

#ifdef __ANDROID__
ndk_helper::DirectoryIterator list_dir(const std::filesystem::path& path);
#else
std::filesystem::directory_iterator list_dir(const std::filesystem::path& path);
#endif

[[ noreturn ]] void verbose_abort(const std::string& message);

}
