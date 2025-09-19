#pragma once

namespace Mlib {

class Gun;
class SceneNode;
template <class T>
class DanglingBaseClassRef;

Gun& get_gun(const DanglingBaseClassRef<SceneNode>& node);

}
