#include "Base_Key_Combination.hpp"
#include <Mlib/Strings/String.hpp>

using namespace Mlib;

std::string BaseKeyCombination::to_string() const {
    auto plus = join(", ", key_bindings, [](const auto& k){ return k.to_string(); });
    if (key_bindings.size() > 1) {
        plus = '(' + plus + ')';
    }
    auto minus = not_key_binding.to_string();
    if (!minus.empty()) {
        return '(' + plus + " without " + minus + ')';
    }
    return plus;
}
