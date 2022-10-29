#pragma once
#include <Mlib/Regex.hpp>
#include <string>

namespace Mlib {

struct MacroManifest {
    explicit MacroManifest(const std::string& filename);
    std::string script_file;
    SubstitutionMap variables;
};

}
