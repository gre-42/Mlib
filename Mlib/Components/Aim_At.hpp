#pragma once

namespace Mlib {

class AimAt;
class SceneNode;
template <class T>
class DanglingBaseClassRef;

bool has_aim_at(const DanglingBaseClassRef<SceneNode>& node);
AimAt& get_aim_at(const DanglingBaseClassRef<SceneNode>& node);

}
