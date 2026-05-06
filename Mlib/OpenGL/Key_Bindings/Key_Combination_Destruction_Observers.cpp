#include "Key_Combination_Destruction_Observers.hpp"
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Memory/Destruction_Observers.hpp>
#include <Mlib/Memory/Destruction_Observers.impl.hpp>

namespace Mlib {

template class DestructionObservers<const BaseKeyCombination&>;

}
