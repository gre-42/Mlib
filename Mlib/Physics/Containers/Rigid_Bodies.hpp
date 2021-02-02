#pragma once
#include <Mlib/Geometry/Intersection/Bvh.hpp>
#include <Mlib/Geometry/Intersection/Collision_Triangle.hpp>
#include <Mlib/Physics/Collision/Typed_Mesh.hpp>
#include <Mlib/Physics/Physics_Engine_Config.hpp>
#include <map>
#include <memory>
#include <set>

namespace Mlib {

enum class CollidableMode;
struct ColoredVertexArray;
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

struct RigidBodyAndCollisionTriangleSphere {
    RigidBody& rb;
    CollisionTriangleSphere ctp;
};

class RigidBodies {
    friend class PhysicsEngine;
    friend class CollisionQuery;
public:
    explicit RigidBodies(const PhysicsEngineConfig& cfg);
    void add_rigid_body(
        const std::shared_ptr<RigidBody>& rigid_body,
        const std::list<std::shared_ptr<ColoredVertexArray>>& hitbox,
        const std::list<std::shared_ptr<ColoredVertexArray>>& tirelines,
        CollidableMode collidable_mode);
    void delete_rigid_body(const RigidBody* rigid_body);
    void optimize_search_time(std::ostream& ostr) const;
    void print_search_time() const;
private:
    void transform_object_and_add(const RigidBodyAndMeshes& o);
    std::list<std::shared_ptr<RigidBody>> static_rigid_bodies_;
    std::list<RigidBodyAndMeshes> objects_;
    std::list<RigidBodyAndTransformedMeshes> transformed_objects_;
    std::map<const RigidBody*, CollidableMode> collidable_modes_;
    Bvh<float, RigidBodyAndCollisionTriangleSphere, 3> bvh_;
    PhysicsEngineConfig cfg_;
};

}
