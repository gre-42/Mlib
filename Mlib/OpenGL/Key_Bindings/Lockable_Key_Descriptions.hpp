#pragma once
#include <Mlib/OpenGL/Key_Bindings/Key_Descriptions.hpp>
#include <Mlib/Os/Threads/Lockable_Object.hpp>

namespace Mlib {

using LockableKeyDescriptions = LockableObject<KeyDescriptions>;

}
