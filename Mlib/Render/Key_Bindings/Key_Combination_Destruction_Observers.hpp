#pragma once

namespace Mlib {

template <class T>
class DestructionObservers;
struct BaseKeyCombination;

using KeyCombinationDestructionObservers = DestructionObservers<const BaseKeyCombination&>;

}
