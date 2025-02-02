#pragma once
#include <Mlib/Scene_Precision.hpp>
#include <memory>

namespace Mlib {

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
bool intersect(
    const CollisionPolygonSphere<CompressedScenePos, 4>& q0,
    const CollisionRidgeSphere<CompressedScenePos>& r1,
    IntersectionInfo& intersection_info);

// Triangle - ridge
bool intersect(
    const CollisionPolygonSphere<CompressedScenePos, 3>& t0,
    const CollisionRidgeSphere<CompressedScenePos>& r1,
    IntersectionInfo& intersection_info);

// Quad - line
bool intersect(
    const CollisionPolygonSphere<CompressedScenePos, 4>& q0,
    const CollisionLineSphere<CompressedScenePos>& l1,
    IntersectionInfo& intersection_info);

// Triangle - line
bool intersect(
    const CollisionPolygonSphere<CompressedScenePos, 3>& t0,
    const CollisionLineSphere<CompressedScenePos>& l1,
    IntersectionInfo& intersection_info);

// Quad - intersectable
bool intersect(
    const CollisionPolygonSphere<CompressedScenePos, 4>& q0,
    const IIntersectable& i1,
    IntersectionInfo& intersection_info);

// Triangle - intersectable
bool intersect(
    const CollisionPolygonSphere<CompressedScenePos, 3>& t0,
    const IIntersectable& i1,
    IntersectionInfo& intersection_info);

// Intersectable - ridge
bool intersect(
    const IIntersectable& i0,
    const CollisionRidgeSphere<CompressedScenePos>& r1,
    IntersectionInfo& intersection_info);

// Intersectable - line
bool intersect(
    const IIntersectable& i0,
    const CollisionLineSphere<CompressedScenePos>& l1,
    IntersectionInfo& intersection_info);

// Intersectable - intersectable
bool intersect(
    const IIntersectable& i0,
    const IIntersectable& i1,
    IntersectionInfo& intersection_info);

}
