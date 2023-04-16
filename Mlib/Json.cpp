#include "Json.hpp"
#include <Mlib/Strings/String.hpp>

using namespace Mlib;

std::string Mlib::get_multiline_string(const nlohmann::json& j) {
    return Mlib::join("\n", j.get<std::vector<std::string>>());
}
