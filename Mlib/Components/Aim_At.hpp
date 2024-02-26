#pragma once

namespace Mlib {

class AimAt;
class SceneNode;
template <class T>
class DanglingRef;

AimAt& get_aim_at(DanglingRef<SceneNode> node);

}
