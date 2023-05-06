#pragma once
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <string>
#include <vector>

namespace Mlib {

struct ReplacementParameter {
    std::string title;
    JsonMacroArguments globals;
    std::vector<std::string> required;
};

void from_json(const nlohmann::json& j, ReplacementParameter& rp);

}
