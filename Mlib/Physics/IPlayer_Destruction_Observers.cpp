#include "IPlayer_Destruction_Observers.hpp"
#include <Mlib/Memory/Destruction_Observers.hpp>
#include <Mlib/Physics/Interfaces/IPlayer.hpp>
#include <Mlib/Memory/Destruction_Observers.impl.hpp>

namespace Mlib {

template class DestructionObservers<const IPlayer&>;

}
