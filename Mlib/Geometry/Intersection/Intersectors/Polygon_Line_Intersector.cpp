#include "Polygon_Line_Intersector.hpp"
#include <Mlib/Geometry/Interfaces/IIntersectable.hpp>
#include <Mlib/Geometry/Intersection/Collision_Line.hpp>
#include <Mlib/Geometry/Intersection/Collision_Polygon.hpp>
#include <Mlib/Geometry/Intersection/Collision_Ridge.hpp>
#include <Mlib/Geometry/Intersection/Intersectors/IIntersection_Info.hpp>

using namespace Mlib;

class IntersectionInfoWithoutNormalAndOverlap: public IIntersectionInfo {
public:
    IntersectionInfoWithoutNormalAndOverlap(
        ScenePos ray_t,
        const FixedArray<ScenePos, 3>& intersection_point,
        const FixedArray<ScenePos, 3>& normal0)
        : intersection_point_{ intersection_point }
        , normal0_{ normal0 }
        , ray_t_{ ray_t }
    {}
    virtual bool has_normal_and_overlap() const override {
        return false;
    }
    virtual ScenePos ray_t() const {
        return ray_t_;
    }
    virtual FixedArray<ScenePos, 3> intersection_point() const override {
        return intersection_point_;
    }
    virtual FixedArray<ScenePos, 3> normal0() const override {
        return normal0_;
    }
    virtual FixedArray<ScenePos, 3> normal() const override {
        THROW_OR_ABORT("IntersectionInfoWithoutNormalAndOverlap::normal() not implemented");
    }
    virtual ScenePos overlap() const override {
        THROW_OR_ABORT("IntersectionInfoWithoutNormalAndOverlap::overlap() not implemented");
    }
private:
    FixedArray<ScenePos, 3> intersection_point_;
    FixedArray<ScenePos, 3> normal0_;
    ScenePos ray_t_;
};

class IntersectionInfoWithNormalAndOverlap: public IIntersectionInfo {
public:
    IntersectionInfoWithNormalAndOverlap(
        ScenePos overlap,
        ScenePos ray_t,
        const FixedArray<ScenePos, 3>& intersection_point,
        const FixedArray<ScenePos, 3>& normal)
        : intersection_point_{ intersection_point }
        , normal_{ normal }
        , overlap_{ overlap }
        , ray_t_{ ray_t }
    {}
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
    FixedArray<ScenePos, 3> intersection_point_;
    FixedArray<ScenePos, 3> normal_;
    ScenePos overlap_;
    ScenePos ray_t_;
};

// class NegatedIntersectionInfo: public IIntersectionInfo {
// public:
//     NegatedIntersectionInfo(std::unique_ptr<IIntersectionInfo>&& iinfo)
//         : iinfo_(std::move(iinfo))
//     {}
//     virtual bool intersects() const override {
//         return iinfo_->intersects();
//     }
//     virtual bool has_normal_and_overlap() const override {
//         return iinfo_->has_normal_and_overlap();
//     }
//     virtual ScenePos ray_t() const override {
//         return iinfo_->ray_t();
//     }
//     virtual FixedArray<ScenePos, 3> intersection_point() const override {
//         return iinfo_->intersection_point();
//     }
//     virtual FixedArray<ScenePos, 3> normal0() const override {
//         return -iinfo_->normal0();
//     }
//     virtual FixedArray<ScenePos, 3> normal() const override {
//         return -iinfo_->normal();
//     }
//     virtual ScenePos overlap() const override {
//         return iinfo_->overlap();
//     }
// private:
//     std::unique_ptr<IIntersectionInfo> iinfo_;
// };

// Quad - ridge
std::unique_ptr<IIntersectionInfo> Mlib::intersect(
    const CollisionPolygonSphere<ScenePos, 4>& q0,
    const CollisionRidgeSphere<ScenePos>& r1)
{
    ScenePos ray_t;
    FixedArray<ScenePos, 3> intersection_point = uninitialized;
    if (!r1.ray.intersects(q0.polygon, &ray_t, &intersection_point)) {
        return nullptr;
    }
    return std::unique_ptr<IIntersectionInfo>{ new IntersectionInfoWithoutNormalAndOverlap{ray_t, intersection_point, q0.polygon.plane().normal} };
}

// Triangle - ridge
std::unique_ptr<IIntersectionInfo> Mlib::intersect(
    const CollisionPolygonSphere<ScenePos, 3>& t0,
    const CollisionRidgeSphere<ScenePos>& r1)
{
    ScenePos ray_t;
    FixedArray<ScenePos, 3> intersection_point = uninitialized;
    if (!r1.ray.intersects(t0.polygon, &ray_t, &intersection_point)) {
        return nullptr;
    }
    return std::unique_ptr<IIntersectionInfo>{ new IntersectionInfoWithoutNormalAndOverlap{ray_t, intersection_point, t0.polygon.plane().normal} };
}

// Quad - line
std::unique_ptr<IIntersectionInfo> Mlib::intersect(
    const CollisionPolygonSphere<ScenePos, 4>& q0,
    const CollisionLineSphere<ScenePos>& l1)
{
    ScenePos ray_t;
    FixedArray<ScenePos, 3> intersection_point = uninitialized;
    if (!l1.ray.intersects(q0.polygon, &ray_t, &intersection_point)) {
        return nullptr;
    }
    return std::unique_ptr<IIntersectionInfo>{ new IntersectionInfoWithoutNormalAndOverlap{ray_t, intersection_point, q0.polygon.plane().normal} };
}

// Triangle - line
std::unique_ptr<IIntersectionInfo> Mlib::intersect(
    const CollisionPolygonSphere<ScenePos, 3>& t0,
    const CollisionLineSphere<ScenePos>& l1)
{
    ScenePos ray_t;
    FixedArray<ScenePos, 3> intersection_point = uninitialized;
    if (!l1.ray.intersects(t0.polygon, &ray_t, &intersection_point)) {
        return nullptr;
    }
    return std::unique_ptr<IIntersectionInfo>{ new IntersectionInfoWithoutNormalAndOverlap{ray_t, intersection_point, t0.polygon.plane().normal} };
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
    if (!i1.intersects(q0, overlap, intersection_point, normal)) {
        return nullptr;
    }
    return std::unique_ptr<IIntersectionInfo>(new IntersectionInfoWithNormalAndOverlap{overlap, ray_t, intersection_point, -normal});
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
    if (!i1.intersects(t0, overlap, intersection_point, normal)) {
        return nullptr;
    }
    return std::unique_ptr<IIntersectionInfo>(new IntersectionInfoWithNormalAndOverlap{overlap, ray_t, intersection_point, -normal});
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
    if (!i0.intersects(r1, overlap, intersection_point, normal)) {
        return nullptr;
    }
    return std::unique_ptr<IIntersectionInfo>(new IntersectionInfoWithNormalAndOverlap{overlap, ray_t, intersection_point, normal});
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
    if (!i0.intersects(l1, overlap, ray_t, intersection_point, normal)) {
        return nullptr;
    }
    return std::unique_ptr<IIntersectionInfo>(new IntersectionInfoWithNormalAndOverlap{overlap, ray_t, intersection_point, normal});
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
    if (!i0.intersects(i1, overlap, intersection_point, normal)) {
        return nullptr;
    }
    return std::unique_ptr<IIntersectionInfo>(new IntersectionInfoWithNormalAndOverlap{overlap, ray_t, intersection_point, -normal});
}
