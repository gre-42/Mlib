#include "Scene_Node_Destruction_Observers.hpp"
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <Mlib/Memory/Destruction_Observers.hpp>
#include <Mlib/Memory/Destruction_Observers.impl.hpp>

namespace Mlib {

template class DestructionObservers<SceneNode&>;

}
