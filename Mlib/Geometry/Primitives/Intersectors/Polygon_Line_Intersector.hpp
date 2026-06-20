#pragma once
#include <Mlib/Scene_Config/Scene_Precision.hpp>
#include <memory>

namespace Mlib {

enum class ClosestPointOnIntersection;
enum class IntersectionStatus;

struct IntersectionInfo;
template <class TPosition, size_t tnvertices>
struct CollisionPolygonSphere;
template <class TPosition>
struct CollisionRidgeSphere;
template <class TPosition>
struct CollisionLineSphere;
template <class TDir, class TPos, size_t n>
class TransformationMatrix;
class IIntersectable;

// Quad - ridge
IntersectionStatus intersect(
    const CollisionPolygonSphere<CompressedScenePos, 4>& q0,
    const CollisionRidgeSphere<CompressedScenePos>& r1,
    IntersectionInfo& intersection_info,
    ClosestPointOnIntersection on_intersection);

// Triangle - ridge
IntersectionStatus intersect(
    const CollisionPolygonSphere<CompressedScenePos, 3>& t0,
    const CollisionRidgeSphere<CompressedScenePos>& r1,
    IntersectionInfo& intersection_info,
    ClosestPointOnIntersection on_intersection);

// Quad - line
IntersectionStatus intersect(
    const CollisionPolygonSphere<CompressedScenePos, 4>& q0,
    const CollisionLineSphere<CompressedScenePos>& l1,
    IntersectionInfo& intersection_info,
    ClosestPointOnIntersection on_intersection);

// Triangle - line
IntersectionStatus intersect(
    const CollisionPolygonSphere<CompressedScenePos, 3>& t0,
    const CollisionLineSphere<CompressedScenePos>& l1,
    IntersectionInfo& intersection_info,
    ClosestPointOnIntersection on_intersection);

// Quad - intersectable
IntersectionStatus intersect(
    const CollisionPolygonSphere<CompressedScenePos, 4>& q0,
    const IIntersectable& i1,
    IntersectionInfo& intersection_info,
    ClosestPointOnIntersection on_intersection);

// Triangle - intersectable
IntersectionStatus intersect(
    const CollisionPolygonSphere<CompressedScenePos, 3>& t0,
    const IIntersectable& i1,
    IntersectionInfo& intersection_info,
    ClosestPointOnIntersection on_intersection);

// Intersectable - ridge
IntersectionStatus intersect(
    const IIntersectable& i0,
    const CollisionRidgeSphere<CompressedScenePos>& r1,
    IntersectionInfo& intersection_info,
    ClosestPointOnIntersection on_intersection);

// Intersectable - line
IntersectionStatus intersect(
    const IIntersectable& i0,
    const CollisionLineSphere<CompressedScenePos>& l1,
    IntersectionInfo& intersection_info,
    ClosestPointOnIntersection on_intersection);

// Intersectable - intersectable
IntersectionStatus intersect(
    const IIntersectable& i0,
    const IIntersectable& i1,
    IntersectionInfo& intersection_info,
    ClosestPointOnIntersection on_intersection);

}
