#pragma once
#include <Mlib/Regex.hpp>
#include <compare>
#include <string>
#include <vector>

namespace Mlib {

struct ReplacementParameter {
    static ReplacementParameter from_json(const std::string& filename);
    std::string name;
    std::vector<std::string> on_init;
    SubstitutionMap variables;
    std::vector<std::string> requires_;
    inline std::strong_ordering operator <=> (const ReplacementParameter& other) const {
        return name <=> other.name;
    }
};

}
