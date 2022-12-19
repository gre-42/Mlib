#pragma once
#include <cstdint>
#include <filesystem>
#include <istream>
#include <memory>
#include <sstream>
#include <vector>

#ifdef __ANDROID__
#include <NDKHelper.h>
#endif

namespace Mlib {

enum class LogLevel {
    DISABLED,
    ERROR,
    WARNING,
    INFO
};

LogLevel log_level_from_string(const std::string& s);

void set_log_level(LogLevel log_level);

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
    const std::filesystem::path& filename,
    std::ios_base::openmode mode = std::ios_base::in);

std::unique_ptr<std::ostream> create_ofstream(
    const std::filesystem::path& filename,
    std::ios_base::openmode mode = std::ios_base::out);

std::vector<uint8_t> read_file_bytes(const std::filesystem::path& filename);

bool path_exists(const std::filesystem::path& filename);

void remove_path(const std::filesystem::path& path);

void rename_path(const std::filesystem::path& from, const std::filesystem::path& to);

void create_directories(const std::filesystem::path& dirname);

#ifdef __ANDROID__
ndk_helper::DirectoryIterator list_dir(const std::filesystem::path& path);
#else
std::filesystem::directory_iterator list_dir(const std::filesystem::path& path);
#endif

[[ noreturn ]] void verbose_abort(const std::string& message);

}
