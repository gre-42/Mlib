#pragma once
#include <Mlib/Regex.hpp>
#include <string>
#include <vector>

namespace Mlib {

struct ReplacementParameter {
    static ReplacementParameter from_json(const std::string& filename);
    std::string name;
    std::vector<std::string> on_init;
    SubstitutionMap variables;
    std::vector<std::string> requires_;
    inline bool operator < (const ReplacementParameter& other) const {
        return name < other.name;
    }
};

}
