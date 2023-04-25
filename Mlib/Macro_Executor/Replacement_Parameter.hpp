#pragma once
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <string>
#include <vector>

namespace Mlib {

struct ReplacementParameter {
    static ReplacementParameter from_json(const std::string& filename);
    std::string name;
    std::vector<nlohmann::json> on_init;
    JsonMacroArguments variables;
    std::vector<std::string> requires_;
};

}
