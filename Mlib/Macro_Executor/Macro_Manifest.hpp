#pragma once
#include <Mlib/Regex.hpp>
#include <string>
#include <vector>

namespace Mlib {

struct MacroManifest {
    static MacroManifest from_json(const std::string& filename);
    std::string name;
    std::string script_file;
    SubstitutionMap variables;
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
