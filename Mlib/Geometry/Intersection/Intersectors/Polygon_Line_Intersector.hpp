#pragma once
#include <Mlib/Scene_Pos.hpp>
#include <memory>

namespace Mlib {

struct IntersectionInfo;
template <class TData, size_t tnvertices>
struct CollisionPolygonSphere;
template <class TData>
struct CollisionRidgeSphere;
template <class TData>
class IIntersectable;
template <class TData>
struct CollisionLineSphere;
template <class TDir, class TPos, size_t n>
class TransformationMatrix;

// Quad - ridge
bool intersect(
    const CollisionPolygonSphere<ScenePos, 4>& q0,
    const CollisionRidgeSphere<ScenePos>& r1,
    IntersectionInfo& intersection_info);

// Triangle - ridge
bool intersect(
    const CollisionPolygonSphere<ScenePos, 3>& t0,
    const CollisionRidgeSphere<ScenePos>& r1,
    IntersectionInfo& intersection_info);

// Quad - line
bool intersect(
    const CollisionPolygonSphere<ScenePos, 4>& q0,
    const CollisionLineSphere<ScenePos>& l1,
    IntersectionInfo& intersection_info);

// Triangle - line
bool intersect(
    const CollisionPolygonSphere<ScenePos, 3>& t0,
    const CollisionLineSphere<ScenePos>& l1,
    IntersectionInfo& intersection_info);

// Quad - intersectable
bool intersect(
    const CollisionPolygonSphere<ScenePos, 4>& q0,
    const IIntersectable<ScenePos>& i1,
    IntersectionInfo& intersection_info);

// Triangle - intersectable
bool intersect(
    const CollisionPolygonSphere<ScenePos, 3>& t0,
    const IIntersectable<ScenePos>& i1,
    IntersectionInfo& intersection_info);

// Intersectable - ridge
bool intersect(
    const IIntersectable<ScenePos>& i0,
    const CollisionRidgeSphere<ScenePos>& r1,
    IntersectionInfo& intersection_info);

// Intersectable - line
bool intersect(
    const IIntersectable<ScenePos>& i0,
    const CollisionLineSphere<ScenePos>& l1,
    IntersectionInfo& intersection_info);

// Intersectable - intersectable
bool intersect(
    const IIntersectable<ScenePos>& i0,
    const IIntersectable<ScenePos>& i1,
    IntersectionInfo& intersection_info);

}
