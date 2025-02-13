#include "Base_Key_Combination.hpp"
#include <Mlib/Strings/String.hpp>

using namespace Mlib;

std::string BaseKeyCombination::to_string(InputType filter) const {
    std::list<std::string> lst;
    for (const auto& k : key_bindings) {
        auto s = k.to_string(filter);
        if (!s.empty()) {
            lst.emplace_back(std::move(s));
        }
    }
    auto plus = join(" & ", lst);
    if (lst.size() > 1) {
        plus = '(' + plus + ')';
    }
    auto minus = not_key_binding.to_string(filter);
    if (!minus.empty()) {
        return '(' + plus + " without " + minus + ')';
    }
    return plus;
}
