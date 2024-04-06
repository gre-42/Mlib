#pragma once

namespace Mlib {

class WeaponCycle;
template <class T>
class DanglingRef;
class SceneNode;

const WeaponCycle& get_weapon_cycle(DanglingRef<const SceneNode> node);

WeaponCycle& get_weapon_cycle(DanglingRef<SceneNode> node);

bool has_weapon_cycle(DanglingRef<const SceneNode> node);

}
