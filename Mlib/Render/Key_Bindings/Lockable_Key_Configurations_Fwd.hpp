#pragma once
#include <optional>

namespace Mlib {

class KeyConfigurations;
template <class T>
class LockableObject;

using LockableKeyConfigurations = LockableObject<std::optional<KeyConfigurations>>;

}
