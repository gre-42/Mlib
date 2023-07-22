#pragma once
#include <optional>
#include <set>
#include <string>

namespace Mlib {

struct Material;

class OptionalMaterialHider {
public:
    OptionalMaterialHider();
    bool is_hidden(const Material& material) const;
private:
    std::optional<std::set<std::string>> hidden_names_;
};

}
