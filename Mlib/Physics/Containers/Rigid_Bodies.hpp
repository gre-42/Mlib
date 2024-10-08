#pragma once
#include <Mlib/Geometry/Intersection/Bvh.hpp>
#include <Mlib/Geometry/Intersection/Collision_Line.hpp>
#include <Mlib/Geometry/Intersection/Collision_Polygon.hpp>
#include <Mlib/Geometry/Intersection/Collision_Ridge.hpp>
#include <Mlib/Geometry/Mesh/Collision_Ridges_Rigid_Body.hpp>
#include <Mlib/Iterator/Iterable_Wrapper.hpp>
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Physics/Collision/Typed_Mesh.hpp>
#include <Mlib/Regex/Regex_Select.hpp>
#include <Mlib/Scene_Pos.hpp>
#include <functional>
#include <map>
#include <memory>
#include <set>
#include <unordered_map>
#include <variant>

namespace Mlib {

enum class CollidableMode;
template <class TPos>
class ColoredVertexArray;
class RigidBodyVehicle;
class IIntersectableMesh;
struct PhysicsEngineConfig;
template <class T>
class DestructionFunctionsTokensObject;
template <class TData>
class CollisionMesh;

struct RigidBodyAndMeshes {
    DanglingBaseClassRef<RigidBodyVehicle> rigid_body;
    std::list<TypedMesh<std::pair<BoundingSphere<float, 3>, std::shared_ptr<CollisionMesh<float>>>>> smeshes;
    std::list<TypedMesh<std::pair<BoundingSphere<double, 3>, std::shared_ptr<CollisionMesh<double>>>>> dmeshes;
    inline bool has_meshes() const {
        return !smeshes.empty() || !dmeshes.empty();
    }
};

struct RigidBodyAndIntersectableMeshes {
    DanglingBaseClassRef<RigidBodyVehicle> rigid_body;
    std::list<TypedMesh<std::shared_ptr<IIntersectableMesh>>> meshes;
};

struct RigidBodyAndIntersectableMesh {
    DanglingBaseClassRef<RigidBodyVehicle> rb;
    TypedMesh<std::shared_ptr<IIntersectableMesh>> mesh;
};

struct RigidBodyAndCollisionTriangleSphere {
    RigidBodyVehicle& rb;
    std::variant<CollisionPolygonSphere<ScenePos, 3>, CollisionPolygonSphere<ScenePos, 4>> ctp;
};

struct RigidBodyAndCollisionLineSphere {
    RigidBodyVehicle& rb;
    CollisionLineSphere<ScenePos> clp;
};

struct RigidBodyAndCollisionRidgeSphere {
    RigidBodyVehicle& rb;
    CollisionRidgeSphere crp;
};

enum class CollisionRidgeBakingStatus {
    NOT_BAKED,
    BAKED,
    BAKING
};

class RigidBodies {
    friend class PhysicsEngine;
public:
    explicit RigidBodies(const PhysicsEngineConfig& cfg);
    ~RigidBodies();
    void add_rigid_body(
        RigidBodyVehicle& rigid_body,
        const std::list<std::shared_ptr<ColoredVertexArray<float>>>& s_hitboxes,
        const std::list<std::shared_ptr<ColoredVertexArray<double>>>& d_hitboxes,
        CollidableMode collidable_mode);
    void delete_rigid_body(const RigidBodyVehicle& rigid_body);
    void optimize_search_time(std::ostream& ostr) const;
    void print_search_time() const;
    void plot_convex_mesh_bvh_svg(const std::string& filename, size_t axis0, size_t axis1) const;
    void plot_triangle_bvh_svg(const std::string& filename, size_t axis0, size_t axis1) const;
    void plot_line_bvh_svg(const std::string& filename, size_t axis0, size_t axis1) const;
    IterableWrapper<std::list<RigidBodyAndMeshes>> objects() const;
    IterableWrapper<std::list<RigidBodyAndIntersectableMeshes>> transformed_objects() const;
    const Bvh<ScenePos, RigidBodyAndIntersectableMesh, 3>& convex_mesh_bvh() const;
    const Bvh<ScenePos, RigidBodyAndCollisionTriangleSphere, 3>& triangle_bvh() const;
    const Bvh<ScenePos, RigidBodyAndCollisionRidgeSphere, 3>& ridge_bvh() const;
    const std::map<std::pair<OrderableFixedArray<ScenePos, 3>, OrderableFixedArray<ScenePos, 3>>, const CollisionRidgeSphere*>& ridge_map();
    const Bvh<ScenePos, RigidBodyAndCollisionLineSphere, 3>& line_bvh() const;
    bool empty() const;
private:
    void transform_object_and_add(const RigidBodyAndMeshes& o);
    void bake_collision_ridges() const;
    void bake_collision_ridges_if_necessary() const;
    const PhysicsEngineConfig& cfg_;
    std::unordered_map<const RigidBodyVehicle*, DestructionFunctionsTokensObject<RigidBodyVehicle>> rigid_bodies_;
    std::list<RigidBodyAndMeshes> objects_;
    std::list<RigidBodyAndIntersectableMeshes> transformed_objects_;
    std::map<const RigidBodyVehicle*, CollidableMode> collidable_modes_;
    // BVHs. Do not forget to .clear() the BVHs in the "delete_rigid_body" method.
    Bvh<ScenePos, RigidBodyAndIntersectableMesh, 3> convex_mesh_bvh_;
    Bvh<ScenePos, RigidBodyAndCollisionTriangleSphere, 3> triangle_bvh_;
    mutable Bvh<ScenePos, RigidBodyAndCollisionRidgeSphere, 3> ridge_bvh_;
    mutable std::map<std::pair<OrderableFixedArray<ScenePos, 3>, OrderableFixedArray<ScenePos, 3>>, const CollisionRidgeSphere*> ridge_map_;
    Bvh<ScenePos, RigidBodyAndCollisionLineSphere, 3> line_bvh_;
    mutable CollisionRidgesRigidBody collision_ridges_;
    mutable CollisionRidgeBakingStatus collision_ridges_baking_status_;
};

}
