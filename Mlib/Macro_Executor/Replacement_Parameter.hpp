#pragma once
#include <Mlib/Macro_Executor/Focus.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>

namespace Mlib {

enum class Focus;

struct ReplacementParameterRequired {
    std::vector<std::string> fixed;
    std::vector<std::string> dynamic;
    Focus focus_mask = Focus::ALWAYS;
};

void from_json(const nlohmann::json& j, ReplacementParameterRequired& rp);

struct ReplacementParameter {
    std::string id;
    std::string title;
    ReplacementParameterRequired required;
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
