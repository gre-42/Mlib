#include "Log.hpp"
#include <Mlib/Env.hpp>
#include <iostream>
#include <thread>

using namespace Mlib;

thread_local size_t Log::level_ = 0;

Log::Log(const std::string& message, std::ostream* ostr)
: ostr_{ostr}
{
    info(message);
    ++level_;
}

Log::Log(const std::string& message, const std::string& env_var)
: Log{
    message,
    (::Mlib::getenv_default_bool(env_var.c_str(), false) ||
     ::Mlib::getenv_default_bool("ENABLE_LOG", false))
        ? &std::cerr
        : nullptr}
{}

Log::~Log() {
    --level_;
    info("done");
}

void Log::info(const std::string& message) {
    if (ostr_ != nullptr) {
        (*ostr_) <<
            std::this_thread::get_id() << " " <<
            std::string(level_ * 4, ' ') <<
            message << std::endl;
    }
}
