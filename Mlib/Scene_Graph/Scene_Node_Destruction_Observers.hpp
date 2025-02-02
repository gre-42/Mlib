#pragma once

namespace Mlib {

template <class T>
class DestructionObservers;
template <class T>
class DanglingRef;
class SceneNode;

using SceneNodeDestructionObservers = DestructionObservers<DanglingRef<const SceneNode>>;

}
