#include "Thread_Safe_String_Json.hpp"

using namespace Mlib;

void Mlib::from_json(const nlohmann::json& j, ThreadSafeString& t) {
    std::string s;
    from_json(j, s);
    t = s;
}

void Mlib::to_json(nlohmann::json& j, const ThreadSafeString& t) {
    to_json(j, (std::string)t);
}
