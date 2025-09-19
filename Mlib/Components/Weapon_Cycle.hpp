#pragma once

namespace Mlib {

class WeaponCycle;
template <class T>
class DanglingBaseClassRef;
class SceneNode;

const WeaponCycle& get_weapon_cycle(const DanglingBaseClassRef<const SceneNode>& node);

WeaponCycle& get_weapon_cycle(const DanglingBaseClassRef<SceneNode>& node);

bool has_weapon_cycle(const DanglingBaseClassRef<const SceneNode>& node);

}
