#include "Polygon_Line_Intersector.hpp"
#include <Mlib/Geometry/Interfaces/IIntersectable.hpp>
#include <Mlib/Geometry/Intersection/Collision_Line.hpp>
#include <Mlib/Geometry/Intersection/Collision_Polygon.hpp>
#include <Mlib/Geometry/Intersection/Collision_Ridge.hpp>
#include <Mlib/Geometry/Intersection/Intersectors/IIntersection_Info.hpp>

using namespace Mlib;

template <size_t tnvertices>
class PolygonLineIntersectionInfo: public IIntersectionInfo {
public:
    PolygonLineIntersectionInfo(
        const ConvexPolygon3D<ScenePos, tnvertices>& p0,
        const RaySegment3D<ScenePos>& r1)
        : p0_{ p0 }
        , r1_{ r1 }
        , intersection_point_{ uninitialized }
    {
        intersects_ = r1.intersects(p0_, &t_, &intersection_point_);
    }
    virtual bool intersects() const override {
        return intersects_;
    }
    virtual bool has_normal_and_overlap() const override {
        return false;
    }
    virtual ScenePos ray_t() const {
        return t_;
    }
    virtual FixedArray<ScenePos, 3> intersection_point() const override {
        return intersection_point_;
    }
    virtual FixedArray<ScenePos, 3> normal0() const override {
        return p0_.plane().normal;
    }
    virtual FixedArray<ScenePos, 3> normal() const override {
        THROW_OR_ABORT("PolygonLineIntersectionInfo::normal() not implemented");
    }
    virtual ScenePos overlap() const override {
        THROW_OR_ABORT("PolygonLineIntersectionInfo::overlap() not implemented");
    }
private:
    const ConvexPolygon3D<ScenePos, tnvertices>& p0_;
    const RaySegment3D<ScenePos>& r1_;
    bool intersects_;
    ScenePos t_;
    FixedArray<ScenePos, 3> intersection_point_;
};

class StaticIntersectionInfo: public IIntersectionInfo {
public:
    StaticIntersectionInfo(
        bool intersects,
        ScenePos overlap,
        ScenePos ray_t,
        const FixedArray<ScenePos, 3>& intersection_point,
        const FixedArray<ScenePos, 3>& normal)
        : intersects_{ intersects }
        , intersection_point_{ intersection_point }
        , normal_{ normal }
        , overlap_{ overlap }
        , ray_t_{ ray_t }
    {}
    virtual bool intersects() const override {
        return intersects_;
    }
    virtual bool has_normal_and_overlap() const override {
        return true;
    }
    virtual ScenePos ray_t() const {
        if (std::isnan(ray_t_)) {
            THROW_OR_ABORT("ray t is NaN");
        }
        return ray_t_;
    }
    virtual FixedArray<ScenePos, 3> intersection_point() const override {
        return intersection_point_;
    }
    virtual FixedArray<ScenePos, 3> normal0() const override {
        return normal_;
    }
    virtual FixedArray<ScenePos, 3> normal() const override {
        return normal_;
    }
    virtual ScenePos overlap() const override {
        return overlap_;
    }
private:
    bool intersects_;
    FixedArray<ScenePos, 3> intersection_point_;
    FixedArray<ScenePos, 3> normal_;
    ScenePos overlap_;
    ScenePos ray_t_;
};

// Quad - ridge
std::unique_ptr<IIntersectionInfo> Mlib::intersect(
    const CollisionPolygonSphere<ScenePos, 4>& q0,
    const CollisionRidgeSphere<ScenePos>& r1)
{
    return std::unique_ptr<IIntersectionInfo>{ new PolygonLineIntersectionInfo{q0.polygon, r1.ray} };
}

// Triangle - ridge
std::unique_ptr<IIntersectionInfo> Mlib::intersect(
    const CollisionPolygonSphere<ScenePos, 3>& t0,
    const CollisionRidgeSphere<ScenePos>& r1)
{
    return std::unique_ptr<IIntersectionInfo>{ new PolygonLineIntersectionInfo{t0.polygon, r1.ray} };
}

// Quad - line
std::unique_ptr<IIntersectionInfo> Mlib::intersect(
    const CollisionPolygonSphere<ScenePos, 4>& q0,
    const CollisionLineSphere<ScenePos>& l1)
{
    return std::unique_ptr<IIntersectionInfo>{ new PolygonLineIntersectionInfo{q0.polygon, l1.ray} };
}

// Triangle - line
std::unique_ptr<IIntersectionInfo> Mlib::intersect(
    const CollisionPolygonSphere<ScenePos, 3>& t0,
    const CollisionLineSphere<ScenePos>& l1)
{
    return std::unique_ptr<IIntersectionInfo>{ new PolygonLineIntersectionInfo{t0.polygon, l1.ray} };
}

// Quad - intersectable
std::unique_ptr<IIntersectionInfo> Mlib::intersect(
    const CollisionPolygonSphere<ScenePos, 4>& q0,
    const IIntersectable<ScenePos>& i1)
{
    FixedArray<ScenePos, 3> intersection_point = uninitialized;
    FixedArray<ScenePos, 3> normal = uninitialized;
    ScenePos ray_t = NAN;
    ScenePos overlap;
    bool intersects = i1.intersects(q0, overlap, intersection_point, normal);
    return std::unique_ptr<IIntersectionInfo>(new StaticIntersectionInfo{intersects, overlap, ray_t, intersection_point, normal});
}

// Triangle - intersectable
std::unique_ptr<IIntersectionInfo> Mlib::intersect(
    const CollisionPolygonSphere<ScenePos, 3>& t0,
    const IIntersectable<ScenePos>& i1)
{
    FixedArray<ScenePos, 3> intersection_point = uninitialized;
    FixedArray<ScenePos, 3> normal = uninitialized;
    ScenePos ray_t = NAN;
    ScenePos overlap;
    bool intersects = i1.intersects(t0, overlap, intersection_point, normal);
    return std::unique_ptr<IIntersectionInfo>(new StaticIntersectionInfo{intersects, overlap, ray_t, intersection_point, normal});
}

// Intersectable - ridge
std::unique_ptr<IIntersectionInfo> Mlib::intersect(
    const IIntersectable<ScenePos>& i0,
    const CollisionRidgeSphere<ScenePos>& r1)
{
    FixedArray<ScenePos, 3> intersection_point = uninitialized;
    FixedArray<ScenePos, 3> normal = uninitialized;
    ScenePos ray_t = NAN;
    ScenePos overlap;
    bool intersects = i0.intersects(r1, overlap, intersection_point, normal);
    return std::unique_ptr<IIntersectionInfo>(new StaticIntersectionInfo{intersects, overlap, ray_t, intersection_point, -normal});
}

// Intersectable - line
std::unique_ptr<IIntersectionInfo> Mlib::intersect(
    const IIntersectable<ScenePos>& i0,
    const CollisionLineSphere<ScenePos>& l1)
{
    FixedArray<ScenePos, 3> intersection_point = uninitialized;
    FixedArray<ScenePos, 3> normal = uninitialized;
    ScenePos ray_t;
    ScenePos overlap;
    bool intersects = i0.intersects(l1, overlap, ray_t, intersection_point, normal);
    return std::unique_ptr<IIntersectionInfo>(new StaticIntersectionInfo{intersects, overlap, ray_t, intersection_point, -normal});
}

// Intersectable - intersectable
std::unique_ptr<IIntersectionInfo> Mlib::intersect(
    const IIntersectable<ScenePos>& i0,
    const IIntersectable<ScenePos>& i1)
{
    FixedArray<ScenePos, 3> intersection_point = uninitialized;
    FixedArray<ScenePos, 3> normal = uninitialized;
    ScenePos ray_t = NAN;
    ScenePos overlap;
    bool intersects = i0.intersects(i1, overlap, intersection_point, normal);
    return std::unique_ptr<IIntersectionInfo>(new StaticIntersectionInfo{intersects, overlap, ray_t, intersection_point, -normal});
}
