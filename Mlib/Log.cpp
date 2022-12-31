#include "Log.hpp"
#include <Mlib/Env.hpp>
#include <Mlib/Os/Os.hpp>
#include <thread>

using namespace Mlib;

THREAD_LOCAL(size_t) Log::level_ = 0;

Log::Log(const std::string& message)
{
    info(message);
    ++level_;
}

Log::~Log() {
    --level_;
    info("done");
}

void Log::info(const std::string& message) {
    linfo() <<
            std::this_thread::get_id() << " " <<
            std::string(level_ * 4, ' ') <<
            message;
}
