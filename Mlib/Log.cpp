#include "Log.hpp"
#include <iostream>
#include <thread>

using namespace Mlib;

thread_local size_t Log::level_ = 0;

Log::Log(const std::string& message) {
    info(message);
    ++level_;
}

Log::~Log() {
    --level_;
    info("done");
}

void Log::info(const std::string& message) {
    std::cerr <<
        std::this_thread::get_id() << " " <<
        std::string(level_ * 4, ' ') <<
        message << std::endl;
}
