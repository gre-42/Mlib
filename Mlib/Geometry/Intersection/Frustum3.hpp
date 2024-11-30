#pragma once
#include <Mlib/Geometry/Intersection/Axis_Aligned_Bounding_Box.hpp>
#include <Mlib/Geometry/Plane_Nd.hpp>
#include <ostream>

namespace Mlib {

template <class TData>
class Frustum3 {
public:
    static const size_t NEAR = 0;
    static const size_t FAR = 1;
    static const size_t TOP = 2;
    static const size_t BOTTOM = 3;
    static const size_t LEFT = 4;
    static const size_t RIGHT = 5;
    static Frustum3 from_projection_matrix(const FixedArray<TData, 4, 4>& mvp) {
        // From: https://stackoverflow.com/questions/11770262/glm-calculating-the-frustum-from-the-projection-matrix
        //       http://web.archive.org/web/20120531231005/http://crazyjoke.free.fr/doc/3D/plane%20extraction.pdf
        //       https://cgvr.cs.uni-bremen.de/teaching/cg_literatur/lighthouse3d_view_frustum_culling/index.html

        Frustum3 result{ .planes = uninitialized };
        // Near
        result.planes(NEAR).normal = {    mvp(2, 0) + mvp(3, 0),
                                          mvp(2, 1) + mvp(3, 1),
                                          mvp(2, 2) + mvp(3, 2)};
        result.planes(NEAR).intercept =   mvp(2, 3) + mvp(3, 3);
        // Far
        result.planes(FAR).normal = {    -mvp(2, 0) + mvp(3, 0),
                                         -mvp(2, 1) + mvp(3, 1),
                                         -mvp(2, 2) + mvp(3, 2)};
        result.planes(FAR).intercept =   -mvp(2, 3) + mvp(3, 3);
        // Bottom
        result.planes(BOTTOM).normal = {  mvp(1, 0) + mvp(3, 0),
                                          mvp(1, 1) + mvp(3, 1),
                                          mvp(1, 2) + mvp(3, 2)};
        result.planes(BOTTOM).intercept = mvp(1, 3) + mvp(3, 3);
        // Top
        result.planes(TOP).normal = {    -mvp(1, 0) + mvp(3, 0),
                                         -mvp(1, 1) + mvp(3, 1),
                                         -mvp(1, 2) + mvp(3, 2)};
        result.planes(TOP).intercept =   -mvp(1, 3) + mvp(3, 3);
        // Left
        result.planes(LEFT).normal = {    mvp(0, 0) + mvp(3, 0),
                                          mvp(0, 1) + mvp(3, 1),
                                          mvp(0, 2) + mvp(3, 2)};
        result.planes(LEFT).intercept =   mvp(0, 3) + mvp(3, 3);
        // Right
        result.planes(RIGHT).normal = {  -mvp(0, 0) + mvp(3, 0),
                                         -mvp(0, 1) + mvp(3, 1),
                                         -mvp(0, 2) + mvp(3, 2)};
        result.planes(RIGHT).intercept = -mvp(0, 3) + mvp(3, 3);

        return result;
    }
    void normalize() {
        for (auto& plane : planes.flat_iterable()) {
            auto len = std::sqrt(sum(squared(plane.normal)));
            plane.normal /= len;
            plane.intercept /= len;
        }
    }
    bool intersects(const AxisAlignedBoundingBox<TData, 3>& aabb) const {
        if ((all(aabb.min == fixed_full<TData, 3>(-INFINITY))) &&
            (all(aabb.max == fixed_full<TData, 3>(INFINITY))))
        {
            return true;
        }
        // From: https://learnopengl.com/Guest-Articles/2021/Scene/Frustum-Culling
        auto center = aabb.center();
        auto extents = aabb.max - center;
        for (const auto& plane : planes.flat_iterable()) {
            // Compute the projection interval radius of b onto L(t) = b.c + t * p.n
            TData r = dot0d(extents, abs(plane.normal));
            if (!(-r <= dot0d(plane.normal, center) + plane.intercept)) {
                return false;
            }
        }
        return true;
    }
    const PlaneNd<TData, TData, 3>& near_plane() const {
        return planes(NEAR);
    }
    const PlaneNd<TData, TData, 3>& far_plane() const {
        return planes(FAR);
    }
    const PlaneNd<TData, TData, 3>& bottom_plane() const {
        return planes(BOTTOM);
    }
    const PlaneNd<TData, TData, 3>& top_plane() const {
        return planes(TOP);
    }
    const PlaneNd<TData, TData, 3>& left_plane() const {
        return planes(LEFT);
    }
    const PlaneNd<TData, TData, 3>& right_plane() const {
        return planes(RIGHT);
    }
    FixedArray<PlaneNd<TData, TData, 3>, 6> planes;
};

template <class TData>
std::ostream& operator << (std::ostream& ostr, const Frustum3<TData>& frustum) {
    ostr << "frustum: " << frustum.planes;
    return ostr;
}

}
