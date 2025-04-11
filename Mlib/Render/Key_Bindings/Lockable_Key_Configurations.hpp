#pragma once
#include <Mlib/Render/Key_Bindings/Key_Configurations.hpp>
#include <Mlib/Threads/Lockable_Object.hpp>
#include <optional>

namespace Mlib {

using LockableKeyConfigurations = LockableObject<std::optional<KeyConfigurations>>;

}
