#include "Point_Cloud_Resource.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Cv/Matrix_Conversion.hpp>
#include <Mlib/Geometry/Homogeneous.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Triangle_List.hpp>
#include <Mlib/Images/Coordinates_Fixed.hpp>
#include <Mlib/Math/Fixed_Cholesky.hpp>
#include <Mlib/Math/Fixed_Math.hpp>

using namespace Mlib;
using namespace Mlib::Cv;

PointCloudResource::PointCloudResource(const Array<FixedArray<float, 3>>& points)
{
    TriangleList tris{ "Point cloud", Material() };
    FixedArray<float, 3> d0{0.1f, 0.f, 0.f};
    FixedArray<float, 3> d1{0.f, 0.1f, 0.f};
    for (const FixedArray<float, 3>& p : points.flat_iterable()) {
        tris.draw_rectangle_wo_normals(
            Cv::cv_to_opengl_coordinates(p - d0 - d1),
            Cv::cv_to_opengl_coordinates(p + d0 - d1),
            Cv::cv_to_opengl_coordinates(p + d0 + d1),
            Cv::cv_to_opengl_coordinates(p - d0 + d1));
    }
    rva_ = std::make_shared<ColoredVertexArrayResource>(
        std::make_shared<ColoredVertexArray>(
            "DepthMapResource",
            Material{ .cull_faces = false },
            std::move(std::vector(tris.triangles_.begin(), tris.triangles_.end())),
            std::move(std::vector<FixedArray<ColoredVertex, 2>>()),
            std::move(std::vector<FixedArray<std::vector<BoneWeight>, 3>>()),
            std::move(std::vector<FixedArray<std::vector<BoneWeight>, 2>>())),
        nullptr);
}

void PointCloudResource::instantiate_renderable(const std::string& name, SceneNode& scene_node, const SceneNodeResourceFilter& resource_filter) const
{
    rva_->instantiate_renderable(name, scene_node, resource_filter);
}

std::shared_ptr<AnimatedColoredVertexArrays> PointCloudResource::get_animated_arrays() const
{
    return rva_->get_animated_arrays();
}

void PointCloudResource::generate_triangle_rays(size_t npoints, const FixedArray<float, 3>& lengths, bool delete_triangles) {
    return rva_->generate_triangle_rays(npoints, lengths, delete_triangles);
}
