#pragma once
#include <Mlib/Misc/Source_Location.hpp>

namespace Mlib {

class WeaponCycle;
template <class T>
class DanglingBaseClassRef;
class SceneNode;

DanglingBaseClassRef<const WeaponCycle> get_weapon_cycle(const SceneNode& node, SourceLocation loc);

DanglingBaseClassRef<WeaponCycle> get_weapon_cycle(SceneNode& node, SourceLocation loc);

bool has_weapon_cycle(const DanglingBaseClassRef<const SceneNode>& node);

}
