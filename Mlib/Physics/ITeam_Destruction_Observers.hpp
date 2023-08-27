#pragma once

namespace Mlib {

template <class T>
class DestructionObservers;
class ITeam;

using ITeamDestructionObservers = DestructionObservers<const ITeam&>;

}
