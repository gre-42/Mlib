#pragma once
#include <Mlib/Misc/Source_Location.hpp>

namespace Mlib {

class Gun;
class SceneNode;
template <class T>
class DanglingBaseClassRef;

DanglingBaseClassRef<Gun> get_gun(SceneNode& node, SourceLocation loc);

}
