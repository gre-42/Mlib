#pragma once
#include <Mlib/Geometry/Intersection/Bvh.hpp>
#include <Mlib/Geometry/Intersection/Collision_Line.hpp>
#include <Mlib/Geometry/Intersection/Collision_Polygon.hpp>
#include <Mlib/Geometry/Intersection/Collision_Ridge.hpp>
#include <Mlib/Geometry/Mesh/Collision_Ridges_Rigid_Body.hpp>
#include <Mlib/Geometry/Mesh/Typed_Mesh.hpp>
#include <Mlib/Iterator/Iterable_Wrapper.hpp>
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Regex/Regex_Select.hpp>
#include <Mlib/Scene_Precision.hpp>
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
class CollisionMesh;
class IIntersectable;

struct RigidBodyAndMeshes {
    DanglingBaseClassRef<RigidBodyVehicle> rigid_body;
    std::list<TypedMesh<std::pair<BoundingSphere<CompressedScenePos, 3>, std::shared_ptr<CollisionMesh>>>> meshes;
    inline bool has_meshes() const {
        return !meshes.empty();
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
    std::variant<CollisionPolygonSphere<3>, CollisionPolygonSphere<4>> ctp;
};

struct RigidBodyAndCollisionLineSphere {
    RigidBodyVehicle& rb;
    CollisionLineSphere clp;
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
        const std::list<TypedMesh<std::shared_ptr<IIntersectable>>>& intersectables,
        CollidableMode collidable_mode);
    void delete_rigid_body(const RigidBodyVehicle& rigid_body);
    void optimize_search_time(std::ostream& ostr) const;
    void print_search_time() const;
    void plot_convex_mesh_bvh_svg(const std::string& filename, size_t axis0, size_t axis1) const;
    void plot_triangle_bvh_svg(const std::string& filename, size_t axis0, size_t axis1) const;
    void plot_line_bvh_svg(const std::string& filename, size_t axis0, size_t axis1) const;
    IterableWrapper<std::list<RigidBodyAndMeshes>> objects() const;
    IterableWrapper<std::list<RigidBodyAndIntersectableMeshes>> transformed_objects() const;
    const Bvh<CompressedScenePos, RigidBodyAndIntersectableMesh, 3>& convex_mesh_bvh() const;
    const Bvh<CompressedScenePos, RigidBodyAndCollisionTriangleSphere, 3>& triangle_bvh() const;
    const Bvh<CompressedScenePos, RigidBodyAndCollisionRidgeSphere, 3>& ridge_bvh() const;
    const std::map<std::pair<OrderableFixedArray<CompressedScenePos, 3>, OrderableFixedArray<CompressedScenePos, 3>>, const CollisionRidgeSphere*>& ridge_map();
    const Bvh<CompressedScenePos, RigidBodyAndCollisionLineSphere, 3>& line_bvh() const;
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
    Bvh<CompressedScenePos, RigidBodyAndIntersectableMesh, 3> convex_mesh_bvh_;
    Bvh<CompressedScenePos, RigidBodyAndCollisionTriangleSphere, 3> triangle_bvh_;
    mutable Bvh<CompressedScenePos, RigidBodyAndCollisionRidgeSphere, 3> ridge_bvh_;
    mutable std::map<std::pair<OrderableFixedArray<CompressedScenePos, 3>, OrderableFixedArray<CompressedScenePos, 3>>, const CollisionRidgeSphere*> ridge_map_;
    Bvh<CompressedScenePos, RigidBodyAndCollisionLineSphere, 3> line_bvh_;
    mutable CollisionRidgesRigidBody collision_ridges_;
    mutable CollisionRidgeBakingStatus collision_ridges_baking_status_;
};

}
