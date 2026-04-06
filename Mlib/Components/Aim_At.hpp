#pragma once
#include <Mlib/Misc/Source_Location.hpp>

namespace Mlib {

class AimAt;
class SceneNode;
template <class T>
class DanglingBaseClassRef;

bool has_aim_at(const DanglingBaseClassRef<SceneNode>& node);
DanglingBaseClassRef<AimAt> get_aim_at(const SceneNode& node, SourceLocation loc);

}
