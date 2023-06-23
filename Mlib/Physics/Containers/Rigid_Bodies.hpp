#pragma once
#include <Mlib/Geometry/Intersection/Bvh.hpp>
#include <Mlib/Geometry/Intersection/Collision_Line.hpp>
#include <Mlib/Geometry/Intersection/Collision_Ridge.hpp>
#include <Mlib/Geometry/Intersection/Collision_Triangle.hpp>
#include <Mlib/Geometry/Mesh/Collision_Ridges_Rigid_Body.hpp>
#include <Mlib/Iterator/Iterable_Wrapper.hpp>
#include <Mlib/Physics/Collision/Typed_Mesh.hpp>
#include <Mlib/Regex/Regex_Select.hpp>
#include <functional>
#include <map>
#include <memory>
#include <set>
#include <unordered_map>

namespace Mlib {

enum class CollidableMode;
template <class TPos>
class ColoredVertexArray;
class RigidBodyVehicle;
class IIntersectableMesh;
struct PhysicsResourceFilter;
struct PhysicsEngineConfig;

struct RigidBodyAndMeshes {
    RigidBodyVehicle& rigid_body;
    std::list<TypedMesh<std::pair<BoundingSphere<float, 3>, std::shared_ptr<ColoredVertexArray<float>>>>> smeshes;
    std::list<TypedMesh<std::pair<BoundingSphere<double, 3>, std::shared_ptr<ColoredVertexArray<double>>>>> dmeshes;
};

struct RigidBodyAndIntersectableMeshes {
    RigidBodyVehicle& rigid_body;
    std::list<TypedMesh<std::shared_ptr<IIntersectableMesh>>> meshes;
};

struct RigidBodyAndIntersectableMesh {
    RigidBodyVehicle& rb;
    TypedMesh<std::shared_ptr<IIntersectableMesh>> mesh;
};

struct RigidBodyAndCollisionTriangleSphere {
    RigidBodyVehicle& rb;
    CollisionTriangleSphere ctp;
};

struct RigidBodyAndCollisionLineSphere {
    RigidBodyVehicle& rb;
    CollisionLineSphere clp;
};

struct RigidBodyAndCollisionRidgeSphere {
    RigidBodyVehicle& rb;
    CollisionRidgeSphere crp;
};

class RigidBodies {
    friend class PhysicsEngine;
    friend class CollisionQuery;
public:
    explicit RigidBodies(const PhysicsEngineConfig& cfg);
    ~RigidBodies();
    void add_rigid_body(
        std::unique_ptr<RigidBodyVehicle>&& rigid_body,
        const std::list<std::shared_ptr<ColoredVertexArray<float>>>& s_hitboxes,
        const std::list<std::shared_ptr<ColoredVertexArray<double>>>& d_hitboxes,
        CollidableMode collidable_mode,
        const PhysicsResourceFilter& physics_resource_filter);
    void delete_rigid_body(const RigidBodyVehicle* rigid_body);
    void optimize_search_time(std::ostream& ostr) const;
    void print_search_time() const;
    void plot_convex_mesh_bvh_svg(const std::string& filename, size_t axis0, size_t axis1) const;
    void plot_triangle_bvh_svg(const std::string& filename, size_t axis0, size_t axis1) const;
    void plot_line_bvh_svg(const std::string& filename, size_t axis0, size_t axis1) const;
    IterableWrapper<std::list<RigidBodyAndMeshes>> objects() const;
    IterableWrapper<std::list<RigidBodyAndIntersectableMeshes>> transformed_objects() const;
    const Bvh<double, RigidBodyAndIntersectableMesh, 3>& convex_mesh_bvh() const;
    const Bvh<double, RigidBodyAndCollisionTriangleSphere, 3>& triangle_bvh() const;
    const Bvh<double, RigidBodyAndCollisionRidgeSphere, 3>& ridge_bvh() const;
    const Bvh<double, RigidBodyAndCollisionLineSphere, 3>& line_bvh() const;
private:
    void transform_object_and_add(const RigidBodyAndMeshes& o);
    const PhysicsEngineConfig& cfg_;
    std::unordered_map<const RigidBodyVehicle*, std::unique_ptr<RigidBodyVehicle>> rigid_bodies_;
    std::list<RigidBodyVehicle*> static_rigid_bodies_;
    std::list<RigidBodyAndMeshes> objects_;
    std::list<RigidBodyAndIntersectableMeshes> transformed_objects_;
    std::map<const RigidBodyVehicle*, CollidableMode> collidable_modes_;
    // BVHs. Do not forget to .clear() the BVHs in the "delete_rigid_body" method.
    Bvh<double, RigidBodyAndIntersectableMesh, 3> convex_mesh_bvh_;
    Bvh<double, RigidBodyAndCollisionTriangleSphere, 3> triangle_bvh_;
    mutable Bvh<double, RigidBodyAndCollisionRidgeSphere, 3> ridge_bvh_;
    Bvh<double, RigidBodyAndCollisionLineSphere, 3> line_bvh_;
    CollisionRidgesRigidBody collision_ridges_;
    mutable bool collision_ridges_dirty_;
};

}
