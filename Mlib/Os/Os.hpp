#pragma once
#include <cstdint>
#include <filesystem>
#include <functional>
#include <istream>
#include <memory>
#include <sstream>
#include <vector>

#ifdef __ANDROID__
#include <Mlib/Android/ndk_helper/JNIHelper.h>
#endif

namespace Mlib {

enum class LogLevel {
    ALWAYS,
    ERROR,
    WARNING,
    INFO
};

LogLevel log_level_from_string(const std::string& s);

void set_log_level(LogLevel log_level);

class LogBuf: public std::stringbuf {
public:
    explicit LogBuf(std::function<void(const std::string&)> write);
    virtual int sync() override;
private:
    std::function<void(const std::string&)> write_;
};

class LLog: public std::ostream {
    LLog(const LLog&) = delete;
    LLog& operator = (const LLog&) = delete;
public:
    LLog(LogLevel log_level, const char* prefix);
    ~LLog() override;
    void destroy();
    std::ostream& ref() const;
private:
    std::function<void(const std::string&)> write_;
    LogBuf buf_;
    bool destroyed_;
};

LLog linfo();
LLog lwarn();
LLog lerr();
LLog lraw();

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
bool is_listable(const ndk_helper::DirectoryEntry& entry);
#else
std::filesystem::directory_iterator list_dir(const std::filesystem::path& path);
bool is_listable(const std::filesystem::directory_entry& entry);
#endif

[[ noreturn ]] void verbose_abort(const std::string& message);

}
