#pragma once
#include <Mlib/Threads/Thread_Local.hpp>
#include <string>

namespace Mlib {

class Log {
public:
    explicit Log(const std::string& message);
    ~Log();
    static void info(const std::string& message);
private:
    static THREAD_LOCAL(size_t) level_;
};

#ifdef MLIB_ENABLE_LOG
#define LOG_FUNCTION(msg) ::Mlib::Log log(msg)
#define LOG_INFO(msg) ::Mlib::Log::info(msg)
#else
#define LOG_FUNCTION(msg) do {} while (false)
#define LOG_INFO(msg) do {} while (false)
#endif

}
