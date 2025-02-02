#pragma once

namespace Mlib {

template <class T>
class DestructionObservers;
class IPlayer;

using IPlayerDestructionObservers = DestructionObservers<const IPlayer&>;

}
