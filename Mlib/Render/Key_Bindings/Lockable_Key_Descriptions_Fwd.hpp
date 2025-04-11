#pragma once
#include <optional>

namespace Mlib {

class KeyDescriptions;
template <class T>
class LockableObject;

using LockableKeyDescriptions = LockableObject<std::optional<KeyDescriptions>>;

}
