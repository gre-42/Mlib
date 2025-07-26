#include "Os.hpp"
#include <Mlib/Threads/Containers/Thread_Safe_String.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <filesystem>
#include <fstream>
#include <map>
#include <mutex>
#include <sstream>
#include <string>

#ifdef __ANDROID__
#include <Mlib/Android/ndk_helper/AUi.hpp>
#include <Mlib/Android/ndk_helper/NDKHelper.h>
#else
#include <Mlib/Env.hpp>
#include <Mlib/Os/Set_Thread_Name_Native.hpp>
#include <iostream>
#endif

namespace fs = std::filesystem;

using namespace Mlib;

static LogLevel g_log_level = LogLevel::INFO;

LogLevel Mlib::log_level_from_string(const std::string& s) {
    static const std::map<std::string, LogLevel> m{
        {"info", LogLevel::INFO},
        {"warn", LogLevel::WARNING},
        {"error", LogLevel::ERROR}
    };
    auto it = m.find(s);
    if (it == m.end()) {
        THROW_OR_ABORT("Unknown log level: \"" + s + '"');
    }
    return it->second;
}

void Mlib::set_log_level(LogLevel log_level) {
    g_log_level = log_level;
}

LogBuf::LogBuf(std::function<void(const std::string&)> write)
    : write_{ std::move(write) }
{}

int LogBuf::sync() {
    // From: https://stackoverflow.com/questions/14265581/parse-split-a-string-in-c-using-string-delimiter-standard-c
    char delimiter = '\n';
    std::string s = str();
    size_t last = 0;
    size_t next = 0;
    while ((next = s.find(delimiter, last)) != std::string::npos) {
        write_(s.substr(last, next-last));
        last = next + 1;
    }
    str(s.substr(last));
    return 0;
}

LLog::LLog(std::function<void(const std::string&)> write)
    : std::ostream{ &buf_ }
    , write_{ std::move(write) }
    , buf_{ write_ }
    , destroyed_{ false }
{}

LLog::~LLog() {
    if (destroyed_ && !buf_.str().empty()) {
        verbose_abort("Destroyed log is not empty");
    }
    destroy();
}

void LLog::destroy() {
    flush();
    write_(buf_.str());
    destroyed_ = true;
}

std::ostream& LLog::ref() const {
    return *const_cast<LLog*>(this);
}

#ifdef __ANDROID__

static std::string get_path_in_files_dir(
    const std::initializer_list<std::string>& child_path,
    FileStorageType storage_type)
{
    ndk_helper::StorageType st = [&](){
        switch (storage_type) {
        case FileStorageType::EXTERNAL:
            return ndk_helper::StorageType::EXTERNAL;
        case FileStorageType::CACHE:
            return ndk_helper::StorageType::CACHE;
        }
        THROW_OR_ABORT("Unknown storage type");
    }();
    std::string res = AUi::GetFilesDir(st);
    for (const auto& s : child_path) {
        res += '/' + s;
    }
    return std::filesystem::weakly_canonical(res);
}

LLog Mlib::linfo(LogFlags flags) {
    static std::mutex mutex;
    static std::string last_message;
    return LLog{
        [&, flags](const std::string& s) {
            if (g_log_level >= LogLevel::INFO) {
                std::scoped_lock lock{ mutex };
                if (!any(flags & LogFlags::SUPPRESS_DUPLICATES) || (s != last_message)) {
                    LOGI("%s", s.c_str());
                    last_message = s;
                }
            }
        }};
}

LLog Mlib::lwarn(LogFlags flags) {
    static std::mutex mutex;
    static std::string last_message;
    return LLog{
        [&, flags](const std::string& s) {
            if (g_log_level >= LogLevel::WARNING) {
                std::scoped_lock lock{ mutex };
                if (!any(flags & LogFlags::SUPPRESS_DUPLICATES) || (s != last_message)) {
                    LOGW("%s", s.c_str());
                    last_message = s;
                }
            }
        }};
}

LLog Mlib::lerr(LogFlags flags) {
    static std::mutex mutex;
    static std::string last_message;
    return LLog{
        [&, flags](const std::string& s) {
            if (g_log_level >= LogLevel::ERROR) {
                std::scoped_lock lock{ mutex };
                if (!any(flags & LogFlags::SUPPRESS_DUPLICATES) || (s != last_message)) {
                    LOGE("%s", s.c_str());
                    last_message = s;
                }
            }}};
}

LLog Mlib::lraw(LogFlags flags) {
    if (any(flags & LogFlags::SUPPRESS_DUPLICATES)) {
        THROW_OR_ABORT("Raw logger cannot suppress duplicates");
    }
    return LLog{
        [](const std::string& s) {
            LOGI("%s", s.c_str());
        }};
}

LLog Mlib::lout(LogFlags flags) {
    if (any(flags & LogFlags::SUPPRESS_DUPLICATES)) {
        THROW_OR_ABORT("Out logger cannot suppress duplicates");
    }
    return LLog{
        [](const std::string& s) {
            LOGI("%s", s.c_str());
        }};
}

std::unique_ptr<std::istream> Mlib::create_ifstream(
    const std::filesystem::path& filename,
    std::ios_base::openmode mode)
{
    return AUi::OpenFile(filename, mode);
}

std::vector<uint8_t> Mlib::read_file_bytes(const std::filesystem::path& filename) {
    return AUi::ReadFile(filename);
}

std::unique_ptr<std::ostream> Mlib::create_ofstream(
    const std::filesystem::path& filename,
    std::ios_base::openmode mode,
    FileStorageType storage_type)
{
    auto fn = get_path_in_files_dir({filename}, storage_type);
    return std::make_unique<std::ofstream>(fn, mode);
}

bool Mlib::path_exists(const std::filesystem::path& filename) {
    return AUi::PathExists(filename);
}

void Mlib::remove_path(
    const std::filesystem::path& path,
    FileStorageType storage_type)
 {
    std::error_code ec;
    if (!fs::remove(get_path_in_files_dir({path}, storage_type), ec)) {
        THROW_OR_ABORT("Could not delete path \"" + path.string() + "\". " + ec.message());
    }
}

void Mlib::rename_path(
    const std::filesystem::path& from,
    const std::filesystem::path& to,
    FileStorageType storage_type)
{
    std::error_code ec;
    fs::rename(
        get_path_in_files_dir({from}, storage_type),
        get_path_in_files_dir({to}, storage_type),
        ec);
    if (ec) {
        THROW_OR_ABORT("Could not rename path \"" + from.string() + "\" to " + to.string() + ". " + ec.message());
    }
}

void Mlib::create_directories(
    const std::filesystem::path& dirname,
    FileStorageType storage_type)
{
    std::error_code ec;
    fs::create_directories(get_path_in_files_dir({dirname}, storage_type), ec);
    if (ec) {
        THROW_OR_ABORT("Could not create directories \"" + dirname.string() + "\". " + ec.message());
    }
}

ndk_helper::DirectoryIterator Mlib::list_dir(const std::filesystem::path& path) {
    return AUi::ListDir(path.c_str());
}

bool Mlib::is_listable(const ndk_helper::DirectoryEntry& entry) {
    return entry.is_listable();
}

void Mlib::set_thread_name(const std::string& name) {
    AUi::SetThreadName(name);
}

#else

LLog Mlib::linfo(LogFlags flags) {
    static std::mutex mutex;
    static std::string last_message;
    return LLog{
        [&, flags](const std::string& s) {
        if (g_log_level >= LogLevel::INFO) {
            std::scoped_lock lock{ mutex };
            if (!any(flags & LogFlags::SUPPRESS_DUPLICATES) || (s != last_message)) {
                std::cerr << "Info: " << s << std::endl;
                last_message = s;
            }
        }}};
}

LLog Mlib::lwarn(LogFlags flags) {
    static std::mutex mutex;
    static std::string last_message;
    return LLog{
        [&, flags](const std::string& s) {
            std::scoped_lock lock{ mutex };
            if (!any(flags & LogFlags::SUPPRESS_DUPLICATES) || (s != last_message)) {
                std::cerr << "Warning: " << s << std::endl;
                last_message = s;
            }
        }};
}

LLog Mlib::lerr(LogFlags flags) {
    static std::mutex mutex;
    static std::string last_message;
    return LLog{
        [&, flags](const std::string& s) {
            std::scoped_lock lock{ mutex };
            if (!any(flags & LogFlags::SUPPRESS_DUPLICATES) || (s != last_message)) {
                std::cerr << "Error: " << s << std::endl;
                last_message = s;
            }
        }};
}

LLog Mlib::lraw(LogFlags flags) {
    if (any(flags & LogFlags::SUPPRESS_DUPLICATES)) {
        THROW_OR_ABORT("Raw logger cannot suppress duplicates");
    }
    return LLog{
        [](const std::string& s) {
            std::cerr << s << std::endl;
        }};
}

LLog Mlib::lout(LogFlags flags) {
    if (any(flags & LogFlags::SUPPRESS_DUPLICATES)) {
        THROW_OR_ABORT("Out logger cannot suppress duplicates");
    }
    return LLog{
        [](const std::string& s) {
            std::cout << s << std::endl;
        }};
}

std::unique_ptr<std::istream> Mlib::create_ifstream(
    const std::filesystem::path& filename,
    std::ios_base::openmode mode)
{
    return std::make_unique<std::ifstream>(filename, mode);
}

std::vector<uint8_t> Mlib::read_file_bytes(const std::filesystem::path& filename) {
    std::ifstream f{filename, std::ios::binary};
    if (f.fail()) {
        THROW_OR_ABORT("Could not open file for read: \"" + filename.string() + '"');
    }
    f.seekg(0, std::ifstream::end);
    std::streamoff file_size = f.tellg();
    f.seekg(0, std::ifstream::beg);
    std::vector<uint8_t> res;
    res.reserve((size_t)file_size);
    res.assign(std::istreambuf_iterator<char>(f),
               std::istreambuf_iterator<char>());
    if (f.fail() && !f.eof()) {
        THROW_OR_ABORT("Could not read from file: \"" + filename.string() + '"');
    }
    return res;
}

std::unique_ptr<std::ostream> Mlib::create_ofstream(
    const std::filesystem::path& filename,
    std::ios_base::openmode mode,
    FileStorageType storage_type)
{
    return std::make_unique<std::ofstream>(filename, mode);
}

bool Mlib::path_exists(const std::filesystem::path& path) {
    std::error_code ec;
    bool exists = fs::exists(path, ec);
    if (ec) {
        THROW_OR_ABORT("Could not check if path \"" + path.string() + "\" exists. " + ec.message());
    }
    return exists;
}

void Mlib::remove_path(
    const std::filesystem::path& path,
    FileStorageType storage_type)
{
    std::error_code ec;
    if (!fs::remove(path, ec)) {
        THROW_OR_ABORT("Could not delete path \"" + path.string() + "\". " + ec.message());
    }
}

void Mlib::rename_path(
    const std::filesystem::path& from,
    const std::filesystem::path& to,
    FileStorageType storage_type)
{
    std::error_code ec;
    fs::rename(from, to, ec);
    if (ec) {
        THROW_OR_ABORT("Could not rename path \"" + from.string() + "\" to " + to.string() + ". " + ec.message());
    }
}

void Mlib::create_directories(
    const std::filesystem::path& dirname,
    FileStorageType storage_type)
{
    std::error_code ec;
    fs::create_directories(dirname, ec);
    if (ec) {
        THROW_OR_ABORT("Could not create directories \"" + dirname.string() + "\". " + ec.message());
    }
}

std::filesystem::directory_iterator Mlib::list_dir(const std::filesystem::path& path) {
    return std::filesystem::directory_iterator(path);
}

bool Mlib::is_listable(const std::filesystem::directory_entry& entry) {
    std::error_code ec;
    bool is_directory = entry.is_directory(ec);
    if (ec) {
        THROW_OR_ABORT("Could not check if path \"" + entry.path().string() + "\" is a directory. " + ec.message());
    }
    return is_directory;
}

void Mlib::set_thread_name(const std::string& name) {
    set_thread_name_native(name);
}

#endif

void Mlib::verbose_abort(const std::string& message) {
    lerr() << "Aborting: " << message;
    std::abort();
}
