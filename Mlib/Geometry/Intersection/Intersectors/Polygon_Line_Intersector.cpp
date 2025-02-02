#include "Polygon_Line_Intersector.hpp"
#include <Mlib/Geometry/Interfaces/IIntersectable.hpp>
#include <Mlib/Geometry/Intersection/Collision_Line.hpp>
#include <Mlib/Geometry/Intersection/Collision_Polygon.hpp>
#include <Mlib/Geometry/Intersection/Collision_Ridge.hpp>
#include <Mlib/Geometry/Intersection/Intersectors/Intersection_Info.hpp>

#ifdef __GNUC__
    #pragma GCC push_options
    #pragma GCC optimize ("O3")
#endif

using namespace Mlib;

// Quad - ridge
bool Mlib::intersect(
    const CollisionPolygonSphere<CompressedScenePos, 4>& q0,
    const CollisionRidgeSphere<CompressedScenePos>& r1,
    IntersectionInfo& intersection_info)
{
    ScenePos ray_t;
    auto q0c_polygon = q0.polygon.casted<ScenePos, ScenePos>();
    if (!r1.ray.casted<ScenePos, ScenePos>().intersects(q0c_polygon, &ray_t, &intersection_info.intersection_point)) {
        return false;
    }
    intersection_info.normal0 = q0.polygon.plane.normal;
    intersection_info.ray_t = ray_t;
    return true;
}

// Triangle - ridge
bool Mlib::intersect(
    const CollisionPolygonSphere<CompressedScenePos, 3>& t0,
    const CollisionRidgeSphere<CompressedScenePos>& r1,
    IntersectionInfo& intersection_info)
{
    ScenePos ray_t;
    auto t0c_polygon = t0.polygon.casted<ScenePos, ScenePos>();
    if (!r1.ray.casted<ScenePos, ScenePos>().intersects(t0c_polygon, &ray_t, &intersection_info.intersection_point)) {
        return false;
    }
    intersection_info.normal0 = t0.polygon.plane.normal;
    intersection_info.ray_t = ray_t;
    return true;
}

// Quad - line
bool Mlib::intersect(
    const CollisionPolygonSphere<CompressedScenePos, 4>& q0,
    const CollisionLineSphere<CompressedScenePos>& l1,
    IntersectionInfo& intersection_info)
{
    ScenePos ray_t;
    auto q0c_polygon = q0.polygon.casted<ScenePos, ScenePos>();
    if (!l1.ray.casted<ScenePos, ScenePos>().intersects(q0c_polygon, &ray_t, &intersection_info.intersection_point)) {
        return false;
    }
    intersection_info.normal0 = q0.polygon.plane.normal;
    intersection_info.ray_t = ray_t;
    return true;
}

// Triangle - line
bool Mlib::intersect(
    const CollisionPolygonSphere<CompressedScenePos, 3>& t0,
    const CollisionLineSphere<CompressedScenePos>& l1,
    IntersectionInfo& intersection_info)
{
    ScenePos ray_t;
    auto t0c_polygon = t0.polygon.casted<ScenePos, ScenePos>();
    if (!l1.ray.casted<ScenePos, ScenePos>().intersects(t0c_polygon, &ray_t, &intersection_info.intersection_point)) {
        return false;
    }
    intersection_info.normal0 = t0.polygon.plane.normal;
    intersection_info.ray_t = ray_t;
    return true;
}

// Quad - intersectable
bool Mlib::intersect(
    const CollisionPolygonSphere<CompressedScenePos, 4>& q0,
    const IIntersectable& i1,
    IntersectionInfo& intersection_info)
{
    FixedArray<SceneDir, 3> normal = uninitialized;
    ScenePos ray_t = NAN;
    ScenePos overlap;
    if (!i1.intersects(q0, overlap, intersection_info.intersection_point, normal)) {
        return false;
    }
    intersection_info.normal0 = q0.polygon.plane.normal;
    intersection_info.ray_t = ray_t;
    intersection_info.no = NormalAndOverlap{ -normal, overlap };
    return true;
}

// Triangle - intersectable
bool Mlib::intersect(
    const CollisionPolygonSphere<CompressedScenePos, 3>& t0,
    const IIntersectable& i1,
    IntersectionInfo& intersection_info)
{
    FixedArray<SceneDir, 3> normal = uninitialized;
    ScenePos ray_t = NAN;
    ScenePos overlap;
    if (!i1.intersects(t0, overlap, intersection_info.intersection_point, normal)) {
        return false;
    }
    intersection_info.normal0 = t0.polygon.plane.normal;
    intersection_info.ray_t = ray_t;
    intersection_info.no = NormalAndOverlap{ -normal, overlap };
    return true;
}

// Intersectable - ridge
bool Mlib::intersect(
    const IIntersectable& i0,
    const CollisionRidgeSphere<CompressedScenePos>& r1,
    IntersectionInfo& intersection_info)
{
    FixedArray<ScenePos, 3> intersection_point = uninitialized;
    FixedArray<SceneDir, 3> normal = uninitialized;
    ScenePos overlap;
    if (!i0.intersects(r1, overlap, intersection_point, normal)) {
        return false;
    }
    intersection_info.intersection_point = intersection_point;
    intersection_info.normal0 = normal;
    intersection_info.no = NormalAndOverlap{ normal, overlap };
    return true;
}

// Intersectable - line
bool Mlib::intersect(
    const IIntersectable& i0,
    const CollisionLineSphere<CompressedScenePos>& l1,
    IntersectionInfo& intersection_info)
{
    FixedArray<ScenePos, 3> intersection_point = uninitialized;
    FixedArray<SceneDir, 3> normal = uninitialized;
    ScenePos ray_t;
    ScenePos overlap;
    if (!i0.intersects(l1, overlap, ray_t, intersection_point, normal)) {
        return false;
    }
    intersection_info.intersection_point = intersection_point;
    intersection_info.normal0 = normal;
    intersection_info.no = NormalAndOverlap{ normal, overlap };
    return true;
}

// Intersectable - intersectable
bool Mlib::intersect(
    const IIntersectable& i0,
    const IIntersectable& i1,
    IntersectionInfo& intersection_info)
{
    FixedArray<ScenePos, 3> intersection_point = uninitialized;
    FixedArray<SceneDir, 3> normal = uninitialized;
    ScenePos overlap;
    if (!i0.intersects(i1, overlap, intersection_point, normal)) {
        return false;
    }
    intersection_info.intersection_point = intersection_point;
    intersection_info.normal0 = normal;
    intersection_info.no = NormalAndOverlap{ normal, overlap };
    return true;
}

#ifdef __GNUC__
    #pragma GCC pop_options
#endif
