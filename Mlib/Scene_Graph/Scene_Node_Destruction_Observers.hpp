#pragma once

namespace Mlib {

template <class T>
class DestructionObservers;
template <class T>
class DanglingBaseClassRef;
class SceneNode;

using SceneNodeDestructionObservers = DestructionObservers<DanglingBaseClassRef<const SceneNode>>;

}
