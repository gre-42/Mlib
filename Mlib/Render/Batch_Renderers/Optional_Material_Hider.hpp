#pragma once
#include <Mlib/Variable_And_Hash.hpp>
#include <optional>
#include <string>
#include <unordered_set>

namespace Mlib {

struct Material;

class OptionalMaterialHider {
public:
    OptionalMaterialHider();
    ~OptionalMaterialHider();
    bool is_hidden(const Material& material) const;
private:
    std::optional<std::unordered_set<VariableAndHash<std::string>>> hidden_names_;
    bool print_materials_;
};

}
