#pragma once
#include <Mlib/Array/Array_Forward.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Scene_Precision.hpp>
#include <variant>

namespace Mlib {

class RigidBodyVehicle;
class PhysicsEngine;
class IIntersectableMesh;
template <class TPosition, size_t tnvertices>
struct CollisionPolygonSphere;

class CollisionQuery {
public:
    CollisionQuery(PhysicsEngine& physics_engine);
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
        ScenePos height_offset = 0,
        float time_offset = 0,
        FixedArray<ScenePos, 3>* intersection_point = nullptr,
        std::variant<const CollisionPolygonSphere<CompressedScenePos, 3>*, const CollisionPolygonSphere<CompressedScenePos, 4>*>* intersection_polygon = nullptr,
        const RigidBodyVehicle** seen_object = nullptr,
        const IIntersectableMesh** seen_mesh = nullptr) const;
    bool can_see(
        const RigidBodyVehicle& watcher,
        const FixedArray<ScenePos, 3>& watched,
        bool only_terrain = false,
        PhysicsMaterial collidable_mask = PhysicsMaterial::OBJ_BULLET_COLLIDABLE_MASK,
        ScenePos height_offset = 0,
        float time_offset = 0,
        FixedArray<ScenePos, 3>* intersection_point = nullptr,
        std::variant<const CollisionPolygonSphere<CompressedScenePos, 3>*, const CollisionPolygonSphere<CompressedScenePos, 4>*>* intersection_polygon = nullptr,
        const RigidBodyVehicle** seen_object = nullptr,
        const IIntersectableMesh** seen_mesh = nullptr) const;
private:
    const PhysicsEngine& physics_engine_;
};

}
