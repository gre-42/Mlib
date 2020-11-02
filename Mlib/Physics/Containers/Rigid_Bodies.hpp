#pragma once
#include <Mlib/Geometry/Intersection/Bvh.hpp>
#include <Mlib/Geometry/Intersection/Collision_Triangle.hpp>
#include <Mlib/Physics/Physics_Engine_Config.hpp>
#include <Mlib/Physics/Typed_Mesh.hpp>
#include <map>
#include <memory>
#include <set>

namespace Mlib {

class ColoredVertexArray;
class RigidBody;
class TransformedMesh;

struct RigidBodyAndMeshes {
    std::shared_ptr<RigidBody> rigid_body;
    std::list<TypedMesh<std::pair<BoundingSphere<float, 3>, std::shared_ptr<ColoredVertexArray>>>> meshes;
};

struct RigidBodyAndTransformedMeshes {
    std::shared_ptr<RigidBody> rigid_body;
    std::list<TypedMesh<std::shared_ptr<TransformedMesh>>> meshes;
};

class RigidBodies {
    friend class PhysicsEngine;
    friend class CollisionQuery;
public:
    explicit RigidBodies(const PhysicsEngineConfig& cfg);
    void add_rigid_body(
        const std::shared_ptr<RigidBody>& rigid_body,
        const std::list<std::shared_ptr<ColoredVertexArray>>& hitbox,
        const std::list<std::shared_ptr<ColoredVertexArray>>& tirelines);
    void delete_rigid_body(const RigidBody* rigid_body);
private:
    std::list<std::shared_ptr<RigidBody>> static_rigid_bodies_;
    std::list<RigidBodyAndMeshes> objects_;
    std::list<RigidBodyAndTransformedMeshes> transformed_objects_;
    Bvh<float, CollisionTriangleSphere, 3> bvh_;
    PhysicsEngineConfig cfg_;
};

}
