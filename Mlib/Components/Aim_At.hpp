#pragma once

namespace Mlib {

class AimAt;
class SceneNode;
template <class T>
class DanglingRef;

bool has_aim_at(DanglingRef<SceneNode> node);
AimAt& get_aim_at(DanglingRef<SceneNode> node);

}
