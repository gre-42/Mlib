#pragma once
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Regex.hpp>
#include <string>
#include <vector>

namespace Mlib {

struct MacroManifest {
    static MacroManifest from_json(const std::string& filename);
    std::string name;
    std::string script_file;
    JsonMacroArguments json_variables;
    std::vector<std::string> requires_;
};

struct MacroManifestAndFilename {
    std::string filename;
    MacroManifest manifest;
    inline bool operator < (const MacroManifestAndFilename& other) const {
        return manifest.name < other.manifest.name;
    }
};

}
