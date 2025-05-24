#pragma once
#include <Mlib/Scene/Generic_Scenes.hpp>
#include <Mlib/Scene/Physics_Scene.hpp>

namespace Mlib {

class PhysicsScenes: public GenericScenes<PhysicsScene> {
    PhysicsScenes(const PhysicsScenes&) = delete;
    PhysicsScenes& operator = (const PhysicsScenes&) = delete;
public:
    PhysicsScenes();
    ~PhysicsScenes();
};

}
