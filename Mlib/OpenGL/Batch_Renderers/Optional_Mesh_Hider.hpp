#pragma once
#include <optional>
#include <set>
#include <string>

namespace Mlib {

class OptionalMeshHider {
public:
    OptionalMeshHider();
    bool is_hidden(const std::string& name) const;
private:
    std::optional<std::set<std::string>> hidden_names_;
    bool print_names_;
};

}
