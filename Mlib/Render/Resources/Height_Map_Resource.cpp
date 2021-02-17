#include "Height_Map_Resource.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Homogeneous.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Images/Coordinates_Fixed.hpp>
#include <Mlib/Math/Fixed_Math.hpp>

using namespace Mlib;

HeightMapResource::HeightMapResource(
    const Array<float>& rgb_picture,
    const Array<float>& height_picture,
    const TransformationMatrix<float, 2>& normalization_matrix)
{
    std::vector<FixedArray<ColoredVertex, 3>> triangles;
    triangles.reserve(2 * height_picture.nelements());
    assert(rgb_picture.ndim() == 3);
    assert(rgb_picture.shape(0) == 3);
    Array<float> R = rgb_picture[0];
    Array<float> G = rgb_picture[1];
    Array<float> B = rgb_picture[2];
    const Array<float>& Z = height_picture;
    for (size_t r = 0; r < rgb_picture.shape(1) - 1; ++r) {
        for (size_t c = 0; c < rgb_picture.shape(2) - 1; ++c) {
            if (std::isnan(Z(r, c)) ||
                std::isnan(Z(r, c + 1)) ||
                std::isnan(Z(r + 1, c)) ||
                std::isnan(Z(r + 1, c + 1)))
            {
                continue;
            }
            FixedArray<size_t, 2> id0{r, c};
            FixedArray<size_t, 2> id1{r + 1, c + 1};
            FixedArray<float, 2> pos0 = normalization_matrix.transform(i2a(id0));
            FixedArray<float, 2> pos1 = normalization_matrix.transform(i2a(id1));
            ColoredVertex v00{
                FixedArray<float, 3>{
                    pos0(0),
                    -pos0(1),
                    Z(r, c)},
                FixedArray<float, 3>{
                    R(r, c),
                    G(r, c),
                    B(r, c)}};
            ColoredVertex v01{
                FixedArray<float, 3>{
                    pos1(0),
                    -pos0(1),
                    Z(r, c + 1)},
                FixedArray<float, 3>{
                    R(r, c + 1),
                    G(r, c + 1),
                    B(r, c + 1)}};
            ColoredVertex v10{
                FixedArray<float, 3>{
                    pos0(0),
                    -pos1(1),
                    Z(r + 1, c)},
                FixedArray<float, 3>{
                    R(r + 1, c),
                    G(r + 1, c),
                    B(r + 1, c)}};
            ColoredVertex v11{
                FixedArray<float, 3>{
                    pos1(0),
                    -pos1(1),
                    Z(r + 1, c + 1)},
                FixedArray<float, 3>{
                    R(r + 1, c + 1),
                    G(r + 1, c + 1),
                    B(r + 1, c + 1)}};

            auto add_triangle = [&triangles](const ColoredVertex& a, const ColoredVertex& b, const ColoredVertex& c) {
                triangles.push_back(FixedArray<ColoredVertex, 3>{a, b, c});
                FixedArray<float, 3> normal = triangle_normal({a.position, b.position, c.position});
                triangles.back()(0).normal = normal;
                triangles.back()(1).normal = normal;
                triangles.back()(2).normal = normal;
            };
            add_triangle(v00, v11, v01);
            add_triangle(v11, v00, v10);
        }
    }
    rva_ = std::make_shared<ColoredVertexArrayResource>(
        std::make_shared<ColoredVertexArray>(
            "HeightMapResource",
            Material{},
            std::move(triangles),
            std::move(std::vector<FixedArray<ColoredVertex, 2>>()),
            std::move(std::vector<FixedArray<std::vector<BoneWeight>, 3>>()),
            std::move(std::vector<FixedArray<std::vector<BoneWeight>, 2>>())),
        nullptr);
}

void HeightMapResource::instantiate_renderable(const std::string& name, SceneNode& scene_node, const SceneNodeResourceFilter& resource_filter) const
{
    rva_->instantiate_renderable(name, scene_node, resource_filter);
}

std::shared_ptr<AnimatedColoredVertexArrays> HeightMapResource::get_animated_arrays() const
{
    return rva_->get_animated_arrays();
}

void HeightMapResource::generate_triangle_rays(size_t npoints, const FixedArray<float, 3>& lengths, bool delete_triangles) {
    return rva_->generate_triangle_rays(npoints, lengths, delete_triangles);
}
