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

        Frustum3 result;
        // Near
        result.planes(NEAR).normal = {    mvp(2u, 0u) + mvp(3u, 0u),
                                          mvp(2u, 1u) + mvp(3u, 1u),
                                          mvp(2u, 2u) + mvp(3u, 2u)};
        result.planes(NEAR).intercept =   mvp(2u, 3u) + mvp(3u, 3u);
        // Far
        result.planes(FAR).normal = {    -mvp(2u, 0u) + mvp(3u, 0u),
                                         -mvp(2u, 1u) + mvp(3u, 1u),
                                         -mvp(2u, 2u) + mvp(3u, 2u)};
        result.planes(FAR).intercept =   -mvp(2u, 3u) + mvp(3u, 3u);
        // Bottom
        result.planes(BOTTOM).normal = {  mvp(1u, 0u) + mvp(3u, 0u),
                                          mvp(1u, 1u) + mvp(3u, 1u),
                                          mvp(1u, 2u) + mvp(3u, 2u)};
        result.planes(BOTTOM).intercept = mvp(1u, 3u) + mvp(3u, 3u);
        // Top
        result.planes(TOP).normal = {    -mvp(1u, 0u) + mvp(3u, 0u),
                                         -mvp(1u, 1u) + mvp(3u, 1u),
                                         -mvp(1u, 2u) + mvp(3u, 2u)};
        result.planes(TOP).intercept =   -mvp(1u, 3u) + mvp(3u, 3u);
        // Left
        result.planes(LEFT).normal = {    mvp(0u, 0u) + mvp(3u, 0u),
                                          mvp(0u, 1u) + mvp(3u, 1u),
                                          mvp(0u, 2u) + mvp(3u, 2u)};
        result.planes(LEFT).intercept =   mvp(0u, 3u) + mvp(3u, 3u);
        // Right
        result.planes(RIGHT).normal = {  -mvp(0u, 0u) + mvp(3u, 0u),
                                         -mvp(0u, 1u) + mvp(3u, 1u),
                                         -mvp(0u, 2u) + mvp(3u, 2u)};
        result.planes(RIGHT).intercept = -mvp(0u, 3u) + mvp(3u, 3u);

        return result;
    }
    void normalize() {
        for (auto& plane : planes.flat_iterable()) {
            auto len = std::sqrt(sum(squared(plane.normal)));
            plane.normal /= len;
            plane.intercept /= len;
        }
    }
    bool contains(const AxisAlignedBoundingBox<TData, 3>& aabb) const {
        // From: https://learnopengl.com/Guest-Articles/2021/Scene/Frustum-Culling
        auto center = (aabb.min() + aabb.max()) / (TData)2;
        auto extents = aabb.max() - center;
        for (const auto& plane : planes.flat_iterable()) {
            // Compute the projection interval radius of b onto L(t) = b.c + t * p.n
            TData r = dot0d(extents, abs(plane.normal));
            if (!(-r <= dot0d(plane.normal, center) + plane.intercept)) {
                return false;
            }
        }
        return true;
    }
    const PlaneNd<TData, 3>& near_plane() const {
        return planes(NEAR);
    }
    const PlaneNd<TData, 3>& far_plane() const {
        return planes(FAR);
    }
    const PlaneNd<TData, 3>& bottom_plane() const {
        return planes(BOTTOM);
    }
    const PlaneNd<TData, 3>& top_plane() const {
        return planes(TOP);
    }
    const PlaneNd<TData, 3>& left_plane() const {
        return planes(LEFT);
    }
    const PlaneNd<TData, 3>& right_plane() const {
        return planes(RIGHT);
    }
    FixedArray<PlaneNd<TData, 3>, 6> planes;
};

template <class TData>
std::ostream& operator << (std::ostream& ostr, const Frustum3<TData>& frustum) {
    ostr << "frustum: " << frustum.planes;
    return ostr;
}

}
