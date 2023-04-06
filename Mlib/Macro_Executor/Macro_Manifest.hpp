#pragma once
#include <Mlib/Json.hpp>
#include <Mlib/Regex.hpp>
#include <map>
#include <string>
#include <vector>

namespace Mlib {

struct MacroManifest {
    static MacroManifest from_json(const std::string& filename);
    std::string name;
    std::string script_file;
    std::map<std::string, nlohmann::json> json_variables;
    SubstitutionMap text_variables;
    std::vector<std::string> json_requires_;
    std::vector<std::string> text_requires_;
};

struct MacroManifestAndFilename {
    std::string filename;
    MacroManifest manifest;
    inline bool operator < (const MacroManifestAndFilename& other) const {
        return manifest.name < other.manifest.name;
    }
};

}
