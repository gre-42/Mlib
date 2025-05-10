#include "Thread_Safe_String_Json.hpp"
#include <Mlib/Json/Base.hpp>

using namespace Mlib;

void Mlib::from_json(const nlohmann::json& j, ThreadSafeString& t) {
    std::string s;
    nlohmann::detail::from_json(j, s);
    t = s;
}

void Mlib::to_json(nlohmann::json& j, const ThreadSafeString& t) {
    nlohmann::detail::to_json(j, (std::string)t);
}
