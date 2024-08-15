#pragma once
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>

namespace Mlib {

struct ReplacementParameter {
    std::string id;
    std::string title;
    std::vector<std::string> required;
    JsonMacroArguments database;
    nlohmann::json on_init;
    nlohmann::json on_before_select;
    nlohmann::json on_execute;
};

void from_json(const nlohmann::json& j, ReplacementParameter& rp);

struct ReplacementParameterAndFilename {
    static ReplacementParameterAndFilename from_json(const std::string& filename);
    ReplacementParameter rp;
    std::string filename;
};

}
