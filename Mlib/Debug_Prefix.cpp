#include "Debug_Prefix.hpp"

using namespace Mlib;

std::string Mlib::debug_prefix(const std::string& debugger) {
    const char* prefix = getenv((debugger + "_PREFIX").c_str());
    if (prefix != nullptr) {
        static std::map<std::string, size_t> numbers_;
        return prefix + std::to_string(numbers_[debugger]++);
    } else {
        return "";
    }
}