#pragma once
#include <Mlib/Regex.hpp>
#include <optional>
#include <string>

namespace Mlib {

struct MacroManifest {
    explicit MacroManifest(const std::string& filename);
    std::string script_file;
    SubstitutionMap variables;
    std::optional<std::vector<std::string>> requires_;
};

}
