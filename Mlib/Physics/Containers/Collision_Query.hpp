#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Scene_Precision.hpp>
#include <list>
#include <variant>

namespace Mlib {

class RigidBodyVehicle;
class PhysicsEngine;
class IIntersectable;
class IIntersectableMesh;
template <class TPosition, size_t tnvertices>
struct CollisionPolygonSphere;
template <class T>
struct TypedMesh;
template <class TDir, class TPos, size_t n>
class TransformationMatrix;

class CollisionQuery {
public:
    explicit CollisionQuery(PhysicsEngine& physics_engine);
    bool can_see(
        const FixedArray<ScenePos, 3>& watcher,
        const FixedArray<ScenePos, 3>& watched,
        const RigidBodyVehicle* excluded0 = nullptr,
        const RigidBodyVehicle* excluded1 = nullptr,
        bool only_terrain = false,
        PhysicsMaterial collidable_mask = PhysicsMaterial::OBJ_BULLET_COLLIDABLE_MASK,
        FixedArray<ScenePos, 3>* intersection_point = nullptr,
        std::variant<const CollisionPolygonSphere<CompressedScenePos, 3>*, const CollisionPolygonSphere<CompressedScenePos, 4>*>* intersection_polygon = nullptr,
        const RigidBodyVehicle** seen_object = nullptr,
        const IIntersectableMesh** seen_mesh = nullptr) const;
    bool can_see(
        const RigidBodyVehicle& watcher,
        const RigidBodyVehicle& watched,
        bool only_terrain = false,
        PhysicsMaterial collidable_mask = PhysicsMaterial::OBJ_BULLET_COLLIDABLE_MASK,
        float time_offset = 0,
        FixedArray<ScenePos, 3>* intersection_point = nullptr,
        std::variant<const CollisionPolygonSphere<CompressedScenePos, 3>*, const CollisionPolygonSphere<CompressedScenePos, 4>*>* intersection_polygon = nullptr,
        const RigidBodyVehicle** seen_object = nullptr,
        const IIntersectableMesh** seen_mesh = nullptr) const;
    bool can_see(
        const RigidBodyVehicle& watcher,
        const FixedArray<ScenePos, 3>& watched_position,
        const FixedArray<SceneDir, 3>& watched_velocity = {0.f, 0.f, 0.f},
        bool only_terrain = false,
        PhysicsMaterial collidable_mask = PhysicsMaterial::OBJ_BULLET_COLLIDABLE_MASK,
        ScenePos can_be_seen_height_offset = 0,
        float time_offset = 0,
        FixedArray<ScenePos, 3>* intersection_point = nullptr,
        std::variant<const CollisionPolygonSphere<CompressedScenePos, 3>*, const CollisionPolygonSphere<CompressedScenePos, 4>*>* intersection_polygon = nullptr,
        const RigidBodyVehicle** seen_object = nullptr,
        const IIntersectableMesh** seen_mesh = nullptr) const;
    bool visit_spawn_preventers(
        const TransformationMatrix<SceneDir, ScenePos, 3>& trafo1,
        const std::list<TypedMesh<std::shared_ptr<IIntersectable>>>& intersectables1,
        PhysicsMaterial collidable_mask0,
        PhysicsMaterial collidable_mask1,
        const std::function<bool(const RigidBodyVehicle& vehicle0)>& visit) const;
    bool can_spawn_at(
        const TransformationMatrix<SceneDir, ScenePos, 3>& trafo1,
        const std::list<TypedMesh<std::shared_ptr<IIntersectable>>>& intersectables1,
        PhysicsMaterial collidable_mask0 =
            PhysicsMaterial::OBJ_CHASSIS |
            PhysicsMaterial::OBJ_BULLET_LINE_SEGMENT |
            PhysicsMaterial::OBJ_DISTANCEBOX,
        PhysicsMaterial collidable_mask1 =
            PhysicsMaterial::OBJ_CHASSIS |
            PhysicsMaterial::OBJ_BULLET_LINE_SEGMENT |
            PhysicsMaterial::OBJ_DISTANCEBOX) const;
private:
    const PhysicsEngine& physics_engine_;
};

}
