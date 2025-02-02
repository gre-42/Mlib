#pragma once

namespace Mlib {

class Gun;
class SceneNode;
template <class T>
class DanglingRef;

Gun& get_gun(DanglingRef<SceneNode> node);

}
