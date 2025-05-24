#include "Physics_Scenes.hpp"
#include <Mlib/Scene/Generic_Scenes_Impl.hpp>

using namespace Mlib;

PhysicsScenes::PhysicsScenes()
    : GenericScenes<PhysicsScene>{ "Physics scenes: "}
{}

PhysicsScenes::~PhysicsScenes() = default;

template GuardedIterable<GenericScenes<PhysicsScene>::map_type::iterator, std::shared_lock<SafeAtomicRecursiveSharedMutex>>
    GenericScenes<PhysicsScene>::guarded_iterable();
template GuardedIterable<GenericScenes<PhysicsScene>::map_type::const_iterator, std::shared_lock<SafeAtomicRecursiveSharedMutex>>
    GenericScenes<PhysicsScene>::guarded_iterable() const;
