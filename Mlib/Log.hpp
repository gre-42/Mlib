#pragma once
#include <string>

namespace Mlib {

class Log {
public:
    Log(const std::string& message);
    ~Log();
    static void info(const std::string& message);
private:
    static thread_local size_t level_;
};

#ifdef MLIB_ENABLE_LOG
#define LOG_FUNCTION(msg) ::Mlib::Log log(msg)
#define LOG_INFO(msg) log.info(msg)
#else
#define LOG_FUNCTION(msg)
#define LOG_INFO(msg)
#endif

}
