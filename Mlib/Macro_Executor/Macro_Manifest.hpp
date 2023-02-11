#pragma once
#include <Mlib/Regex.hpp>
#include <string>
#include <vector>

namespace Mlib {

struct MacroManifest {
    explicit MacroManifest(const std::string& filename);
    std::string script_file;
    SubstitutionMap variables;
    std::vector<std::string> requires_;
};

}
