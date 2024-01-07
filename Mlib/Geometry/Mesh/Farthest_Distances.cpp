#include "Farthest_Distances.hpp"
#include <Mlib/Geometry/Mesh/IIntersectable_Mesh.hpp>
#include <Mlib/Geometry/Plane_Nd.hpp>
#include <Mlib/Math/Orderable_Fixed_Array.hpp>
#include <limits>

using namespace Mlib;

VertexDistances Mlib::get_farthest_distances(
    const IIntersectableMesh& mesh,
    const PlaneNd<double, 3>& plane)
{
    VertexDistances res{
        .min = (double)INFINITY,
        .max = -(double)INFINITY
    };
    for (const auto& v : mesh.get_vertices()) {
        auto dist = dot0d(v, plane.normal) + plane.intercept;
        res.min = std::min(res.min, dist);
        res.max = std::max(res.max, dist);
    }
    return res;
}
