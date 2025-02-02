#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Intersection/Collision_Line.hpp>
#include <Mlib/Geometry/Intersection/Collision_Polygon.hpp>
#include <Mlib/Geometry/Intersection/Collision_Ridge.hpp>
#include <Mlib/Geometry/Intersection/Intersectors/Intersection_Info.hpp>
#include <Mlib/Scene_Precision.hpp>
#include <cstdint>
#include <memory>
#include <vector>

namespace Mlib {

class RigidBodyVehicle;
class IIntersectableMesh;
template <class TDir, class TPos, size_t tndim>
class PlaneNd;
enum class CollisionType;
enum class PhysicsMaterial: uint32_t;
struct IntersectionSceneAndContact;
struct CollisionHistory;
template <class TPosition, size_t tnvertices>
struct CollisionPolygonSphere;
template <class TPosition>
struct CollisionRidgeSphere;
template <class TPosition>
struct CollisionLineSphere;
struct SurfaceContactInfo;
class IIntersectable;

struct IntersectionScene {
    RigidBodyVehicle& o0;
    RigidBodyVehicle& o1;
    const IIntersectableMesh* mesh0;
    const IIntersectableMesh* mesh1;
    const std::optional<CollisionLineSphere<CompressedScenePos>> l1;
    const std::optional<CollisionRidgeSphere<CompressedScenePos>> r1;
    const IIntersectable* i1 = nullptr;
    std::optional<CollisionPolygonSphere<CompressedScenePos, 4>> q0;
    std::optional<CollisionPolygonSphere<CompressedScenePos, 3>> t0;
    const IIntersectable* i0 = nullptr;
    size_t tire_id1;
    PhysicsMaterial mesh0_material;
    PhysicsMaterial mesh1_material;
    bool l1_is_normal;
    const SurfaceContactInfo* surface_contact_info;
    CollisionType default_collision_type;
    const CollisionHistory& history;
};

struct IntersectionSceneAndContact {
    IntersectionScene scene;
    IntersectionInfo iinfo;
};

}
