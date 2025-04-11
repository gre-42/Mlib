#pragma once
#include <Mlib/Render/Key_Bindings/Key_Descriptions.hpp>
#include <Mlib/Threads/Lockable_Object.hpp>
#include <optional>

namespace Mlib {

using LockableKeyDescriptions = LockableObject<std::optional<KeyDescriptions>>;

}
