#pragma once
#include <Mlib/Render/Key_Bindings/Key_Descriptions.hpp>
#include <Mlib/Threads/Lockable_Object.hpp>

namespace Mlib {

using LockableKeyDescriptions = LockableObject<KeyDescriptions>;

}
