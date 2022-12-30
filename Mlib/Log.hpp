#pragma once
#include <Mlib/Threads/Thread_Local.hpp>
#include <iostream>
#include <string>

namespace Mlib {

#ifdef __ANDROID__
class Log {
public:
    explicit Log(const std::string& message);
    ~Log();
    static void info(const std::string& message);
private:
    static THREAD_LOCAL(size_t) level_;
};
#else
class Log {
public:
    Log(const std::string& message, std::ostream* ostr = &std::cerr);
    Log(const std::string& message, const std::string& env_var);
    ~Log();
    static void info(const std::string& message);
private:
    static THREAD_LOCAL(size_t) level_;
    std::ostream* ostr_;
};
#endif

#ifdef MLIB_ENABLE_LOG
#define LOG_FUNCTION(msg) ::Mlib::Log log(msg)
#define LOG_INFO(msg) ::Mlib::Log::info(msg)
#else
#define LOG_FUNCTION(msg)
#define LOG_INFO(msg)
#endif

}
