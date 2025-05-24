#pragma once
#include <Mlib/Render/Key_Bindings/Key_Configurations.hpp>
#include <Mlib/Threads/Lockable_Object.hpp>

namespace Mlib {

using LockableKeyConfigurations = LockableObject<KeyConfigurations>;

}
