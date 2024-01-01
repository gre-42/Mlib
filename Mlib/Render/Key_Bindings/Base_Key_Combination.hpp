#pragma once
#include <Mlib/Render/Key_Bindings/Base_Key_Binding.hpp>
#include <list>

namespace Mlib {

struct BaseKeyCombination {
    std::list<BaseKeyBinding> key_bindings;
    BaseKeyBinding not_key_binding;
};

}
