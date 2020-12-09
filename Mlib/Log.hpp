#pragma once
#include <iostream>
#include <string>

namespace Mlib {

class Log {
public:
    Log(const std::string& message, std::ostream* ostr = &std::cerr);
    Log(const std::string& message, const std::string& env_var);
    ~Log();
    void info(const std::string& message);
private:
    static thread_local size_t level_;
    std::ostream* ostr_;
};

#ifdef MLIB_ENABLE_LOG
#define LOG_FUNCTION(msg) ::Mlib::Log log(msg)
#define LOG_INFO(msg) log.info(msg)
#else
#define LOG_FUNCTION(msg)
#define LOG_INFO(msg)
#endif

}
