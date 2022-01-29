#pragma once
#include <Mlib/Render/Key_Bindings/Base_Key_Binding.hpp>
#include <set>

namespace Mlib {

struct BaseKeyCombination {
public:
    std::set<BaseKeyBinding> key_bindings;
    BaseKeyBinding not_key_binding;
    std::partial_ordering operator <=> (const BaseKeyCombination&) const = default;
};

}
