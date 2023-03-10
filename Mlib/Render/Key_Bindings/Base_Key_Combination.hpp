#pragma once
#include <Mlib/Memory/Destruction_Observers.hpp>
#include <Mlib/Object.hpp>
#include <Mlib/Render/Key_Bindings/Base_Key_Binding.hpp>
#include <list>

namespace Mlib {

struct BaseKeyCombination: public Object {
public:
    BaseKeyCombination() = default;
    BaseKeyCombination(
        std::list<BaseKeyBinding>&& key_bindings,
        BaseKeyBinding&& not_key_binding = BaseKeyBinding())
    : key_bindings{std::move(key_bindings)},
      not_key_binding{std::move(not_key_binding)}
    {}
    BaseKeyCombination(BaseKeyCombination&& other)
    : key_bindings{std::move(other.key_bindings)},
      not_key_binding{std::move(other.not_key_binding)},
      destruction_observers{std::move(other.destruction_observers)}
    {}
    BaseKeyCombination& operator = (BaseKeyCombination&& other) {
        key_bindings = std::move(other.key_bindings);
        not_key_binding = std::move(other.not_key_binding);
        destruction_observers = std::move(other.destruction_observers);
        return *this;
    }
    std::list<BaseKeyBinding> key_bindings;
    BaseKeyBinding not_key_binding;
    mutable std::unique_ptr<DestructionObservers> destruction_observers;
};

}
