#pragma once
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Regex/Misc.hpp>
#include <string>
#include <vector>

namespace Mlib {

struct MacroManifest {
    static MacroManifest load_from_json(const std::string& filename);
    std::string name;
    nlohmann::json macro;
    JsonMacroArguments globals;
    std::vector<std::string> required;
};

void from_json(const nlohmann::json& j, MacroManifest& mm);

struct MacroManifestAndFilename {
    std::string filename;
    MacroManifest manifest;
    inline bool operator < (const MacroManifestAndFilename& other) const {
        return manifest.name < other.manifest.name;
    }
};

}
