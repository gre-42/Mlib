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

enum class FileStorageType {
    EXTERNAL,
    CACHE
};

enum class LogLevel {
    ALWAYS,
    ERROR,
    WARNING,
    INFO
};

enum class LogFlags {
    NONE = 0,
    SUPPRESS_DUPLICATES = (1 << 0),
    NO_APPEND_NEWLINE = (1 << 1)
};

inline bool any(LogFlags f) {
    return f != LogFlags::NONE;
}

inline LogFlags operator & (LogFlags a, LogFlags b) {
    return (LogFlags)((int)a & (int)b);
}

inline LogFlags operator | (LogFlags a, LogFlags b) {
    return (LogFlags)((int)a | (int)b);
}

inline LogFlags& operator |= (LogFlags& a, LogFlags b) {
    (int&)a |= (int)b;
    return a;
}

LogLevel log_level_from_string(const std::string& s);

void set_log_level(LogLevel log_level);

class LogBuf: public std::stringbuf {
    LogBuf(const LogBuf&) = delete;
    LogBuf& operator = (const LogBuf&) = delete;
public:
    explicit LogBuf(const std::function<void(const std::string&)>& write);
    virtual int sync() override;
private:
    const std::function<void(const std::string&)>& write_;
};

class LLog: public std::ostream {
    LLog(const LLog&) = delete;
    LLog& operator = (const LLog&) = delete;
public:
    explicit LLog(
        LogFlags flags,
        std::function<void(const std::string&)> write);
    ~LLog() override;
    std::ostream& ref() const;
private:
    LogFlags flags_;
    std::function<void(const std::string&)> write_;
    LogBuf buf_;
};

LLog linfo(LogFlags flags = LogFlags::NONE);
LLog lwarn(LogFlags flags = LogFlags::NONE);
LLog lerr(LogFlags flags = LogFlags::NONE);
LLog lraw(LogFlags flags = LogFlags::NONE);
LLog lout(LogFlags flags = LogFlags::NONE);

std::unique_ptr<std::istream> create_ifstream(
    const std::filesystem::path& filename,
    std::ios_base::openmode mode = std::ios_base::in);

std::unique_ptr<std::ostream> create_ofstream(
    const std::filesystem::path& filename,
    std::ios_base::openmode mode = std::ios_base::out,
    FileStorageType storage_type = FileStorageType::EXTERNAL);

std::vector<uint8_t> read_file_bytes(const std::filesystem::path& filename);

bool path_exists(const std::filesystem::path& filename);

void remove_path(
    const std::filesystem::path& path,
    FileStorageType storage_type = FileStorageType::EXTERNAL);

void rename_path(
    const std::filesystem::path& from,
    const std::filesystem::path& to,
    FileStorageType storage_type = FileStorageType::EXTERNAL);

void create_directories(
    const std::filesystem::path& dirname,
    FileStorageType storage_type = FileStorageType::EXTERNAL);

#ifdef __ANDROID__
ndk_helper::DirectoryIterator list_dir(const std::filesystem::path& path);
bool is_listable(const ndk_helper::DirectoryEntry& entry);
#else
std::filesystem::directory_iterator list_dir(const std::filesystem::path& path);
bool is_listable(const std::filesystem::directory_entry& entry);
#endif

void set_thread_name(const std::string& name);

[[ noreturn ]] void verbose_abort(const std::string& message);

}
