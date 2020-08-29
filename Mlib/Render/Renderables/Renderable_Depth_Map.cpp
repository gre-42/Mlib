#include "Renderable_Depth_Map.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Homogeneous.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Images/Coordinates_Fixed.hpp>
#include <Mlib/Math/Fixed_Math.hpp>

using namespace Mlib;

RenderableDepthMap::RenderableDepthMap(
    const Array<float>& rgb_picture,
    const Array<float>& depth_picture,
    const Array<float>& intrinsic_matrix)
{
    FixedArray<float, 3, 3> iim{inv(intrinsic_matrix)};
    std::vector<FixedArray<ColoredVertex, 3>> triangles;
    triangles.reserve(2 * depth_picture.nelements());
    assert(rgb_picture.ndim() == 3);
    assert(rgb_picture.shape(0) == 3);
    Array<float> R = rgb_picture[0];
    Array<float> G = rgb_picture[1];
    Array<float> B = rgb_picture[2];
    const Array<float>& Z = depth_picture;
    for(size_t r = 0; r < rgb_picture.shape(1) - 1; ++r) {
        for(size_t c = 0; c < rgb_picture.shape(2) - 1; ++c) {
            if (std::isnan(Z(r, c)) ||
                std::isnan(Z(r, c + 1)) ||
                std::isnan(Z(r + 1, c)) ||
                std::isnan(Z(r + 1, c + 1)))
            {
                continue;
            }
            FixedArray<size_t, 2> id0{r, c};
            FixedArray<size_t, 2> id1{r + 1, c + 1};
            FixedArray<float, 3> pos0 = dot(iim, homogenized_3(i2a(id0)));
            FixedArray<float, 3> pos1 = dot(iim, homogenized_3(i2a(id1)));
            pos0 /= pos0(2);
            pos1 /= pos1(2);
            float z_offset = 1;
            ColoredVertex v00{
                    FixedArray<float, 3> {
                        pos0(0) * Z(r, c),
                        -pos0(1) * Z(r, c),
                        -Z(r, c) + z_offset},
                    FixedArray<float, 3>{
                        R(r, c),
                        G(r, c),
                        B(r, c)}};
            ColoredVertex v01{
                    FixedArray<float, 3>{
                        pos1(0) * Z(r, c + 1),
                        -pos0(1) * Z(r, c + 1),
                        -Z(r, c + 1) + z_offset},
                    FixedArray<float, 3> {
                        R(r, c + 1),
                        G(r, c + 1),
                        B(r, c + 1)}};
            ColoredVertex v10{
                    FixedArray<float, 3> {
                        pos0(0) * Z(r + 1, c),
                        -pos1(1) * Z(r + 1, c),
                        -Z(r + 1, c) + z_offset},
                     FixedArray<float, 3> {
                        R(r + 1, c),
                        G(r + 1, c),
                        B(r + 1, c)}};
            ColoredVertex v11{
                    FixedArray<float, 3>{
                        pos1(0) * Z(r + 1, c + 1),
                        -pos1(1) * Z(r + 1, c + 1),
                        -Z(r + 1, c + 1) + z_offset},
                    FixedArray<float, 3> {
                        R(r + 1, c + 1),
                        G(r + 1, c + 1),
                        B(r + 1, c + 1)}};

            triangles.push_back(FixedArray<ColoredVertex, 3>{v00, v11, v01});
            triangles.push_back(FixedArray<ColoredVertex, 3>{v11, v00, v10});
        }
    }
    rva_ = std::make_shared<RenderableColoredVertexArray>(
        std::make_shared<ColoredVertexArray>(
            "",
            Material{},
            std::move(triangles),
            std::move(std::vector<FixedArray<ColoredVertex, 2>>())),
        nullptr);  // rendering_resources
}

void RenderableDepthMap::initialize() {
    rva_->initialize();
}

void RenderableDepthMap::instantiate_renderable(const std::string& name, SceneNode& scene_node, const SceneNodeResourceFilter& resource_filter)
{
    rva_->instantiate_renderable(name, scene_node, resource_filter);
}

std::list<std::shared_ptr<ColoredVertexArray>> RenderableDepthMap::get_triangle_meshes()
{
    return rva_->get_triangle_meshes();
}

void RenderableDepthMap::generate_triangle_rays(size_t npoints, const FixedArray<float, 3>& lengths, bool delete_triangles) {
    return rva_->generate_triangle_rays(npoints, lengths, delete_triangles);
}
