#include "Point_Cloud_Resource.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Coordinates/Coordinate_Conversion.hpp>
#include <Mlib/Geometry/Coordinates/Homogeneous.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Triangle_List.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Images/Coordinates_Fixed.hpp>
#include <Mlib/Math/Fixed_Cholesky.hpp>
#include <Mlib/Math/Fixed_Math.hpp>

using namespace Mlib;
using namespace Mlib::Cv;

PointCloudResource::PointCloudResource(
    const Array<TransformationMatrix<float, float, 3>>& points,
    float point_radius)
{
    TriangleList<float> tris{ "Point cloud", Material(), PhysicsMaterial::ATTR_VISIBLE };
    FixedArray<float, 3> d0{point_radius, 0.f, 0.f};
    FixedArray<float, 3> d1{0.f, point_radius, 0.f};
    for (const TransformationMatrix<float, float, 3>& t : points.flat_iterable()) {
        if (all(t.R() == 0.f)) {
            tris.draw_rectangle_wo_normals(
                cv_to_opengl_coordinates(t.t() - d0 - d1),
                cv_to_opengl_coordinates(t.t() + d0 - d1),
                cv_to_opengl_coordinates(t.t() + d0 + d1),
                cv_to_opengl_coordinates(t.t() - d0 + d1));
        } else {
            tris.draw_rectangle_with_normals(
                cv_to_opengl_coordinates(t.transform(- d0 - d1)),
                cv_to_opengl_coordinates(t.transform(+ d0 - d1)),
                cv_to_opengl_coordinates(t.transform(+ d0 + d1)),
                cv_to_opengl_coordinates(t.transform(- d0 + d1)),
                cv_to_opengl_coordinates(t.inverted().R()[2]),
                cv_to_opengl_coordinates(t.inverted().R()[2]),
                cv_to_opengl_coordinates(t.inverted().R()[2]),
                cv_to_opengl_coordinates(t.inverted().R()[2]));
        }
    }
    rva_ = std::make_shared<ColoredVertexArrayResource>(
        std::make_shared<ColoredVertexArray<float>>(
            "DepthMapResource",
            Material{ .cull_faces = false },
            PhysicsMaterial::ATTR_VISIBLE,
            ModifierBacklog{},
            std::vector<FixedArray<ColoredVertex<float>, 4>>(),
            std::vector(tris.triangles.begin(), tris.triangles.end()),
            std::vector<FixedArray<ColoredVertex<float>, 2>>(),
            std::vector<FixedArray<std::vector<BoneWeight>, 3>>(),
            std::vector<FixedArray<float, 3>>(),
            std::vector<FixedArray<uint8_t, 3>>()));

}

void PointCloudResource::instantiate_renderable(const InstantiationOptions& options) const
{
    rva_->instantiate_renderable(options);
}

std::shared_ptr<AnimatedColoredVertexArrays> PointCloudResource::get_physics_arrays() const
{
    return rva_->get_physics_arrays();
}

void PointCloudResource::generate_triangle_rays(size_t npoints, const FixedArray<float, 3>& lengths, bool delete_triangles) {
    return rva_->generate_triangle_rays(npoints, lengths, delete_triangles);
}
