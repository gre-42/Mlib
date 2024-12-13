#include "Depth_Map_Resource.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Coordinates/Coordinate_Conversion.hpp>
#include <Mlib/Geometry/Coordinates/Homogeneous.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Geometry/Triangle_Normal.hpp>
#include <Mlib/Images/Coordinates_Fixed.hpp>
#include <Mlib/Math/Fixed_Cholesky.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>

using namespace Mlib;
using namespace Mlib::Cv;

DepthMapResource::DepthMapResource(
    const Array<float>& rgb_picture,
    const Array<float>& depth_picture,
    const TransformationMatrix<float, float, 2>& intrinsic_matrix,
    float cos_threshold)
{
    TransformationMatrix<float, float, 2> iim{ inv(intrinsic_matrix.affine()).value() };
    UUVector<FixedArray<ColoredVertex<float>, 3>> triangles;
    triangles.reserve(2 * depth_picture.nelements());
    assert(rgb_picture.ndim() == 3);
    assert(rgb_picture.shape(0) == 3);
    Array<float> R = rgb_picture[0];
    Array<float> G = rgb_picture[1];
    Array<float> B = rgb_picture[2];
    const Array<float>& Z = depth_picture;
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
            FixedArray<float, 2> pos0 = iim.transform(i2a(id0));
            FixedArray<float, 2> pos1 = iim.transform(i2a(id1));
            ColoredVertex<float> v00{
                    cv_to_opengl_coordinates({
                        pos0(0) * Z(r, c),
                        pos0(1) * Z(r, c),
                        Z(r, c)}),
                    Colors::from_rgb({
                        R(r, c),
                        G(r, c),
                        B(r, c)})};
            ColoredVertex<float> v01{
                    cv_to_opengl_coordinates({
                        pos1(0) * Z(r, c + 1),
                        pos0(1) * Z(r, c + 1),
                        Z(r, c + 1)}),
                    Colors::from_rgb({
                        R(r, c + 1),
                        G(r, c + 1),
                        B(r, c + 1)})};
            ColoredVertex<float> v10{
                    cv_to_opengl_coordinates({
                        pos0(0) * Z(r + 1, c),
                        pos1(1) * Z(r + 1, c),
                        Z(r + 1, c)}),
                    Colors::from_rgb({
                        R(r + 1, c),
                        G(r + 1, c),
                        B(r + 1, c)})};
            ColoredVertex<float> v11{
                    cv_to_opengl_coordinates({
                        pos1(0) * Z(r + 1, c + 1),
                        pos1(1) * Z(r + 1, c + 1),
                        Z(r + 1, c + 1)}),
                    Colors::from_rgb({
                        R(r + 1, c + 1),
                        G(r + 1, c + 1),
                        B(r + 1, c + 1)})};

            auto add_triangle = [&triangles, &cos_threshold](const ColoredVertex<float>& a, const ColoredVertex<float>& b, const ColoredVertex<float>& c) {
                FixedArray<float, 3> normal = triangle_normal<float>({a.position, b.position, c.position});
                if ((cos_threshold != 0.f) && (dot0d(normal, a.position) > -std::sqrt(sum(squared(a.position))) * cos_threshold)) {
                    return;
                }
                triangles.push_back(FixedArray<ColoredVertex<float>, 3>{a, b, c});
                triangles.back()(0).normal = normal;
                triangles.back()(1).normal = normal;
                triangles.back()(2).normal = normal;
            };
            add_triangle(v00, v11, v01);
            add_triangle(v11, v00, v10);
        }
    }
    rva_ = std::make_shared<ColoredVertexArrayResource>(
        std::make_shared<ColoredVertexArray<float>>(
            "DepthMapResource",
            Material{},
            Morphology{ PhysicsMaterial::ATTR_VISIBLE },
            ModifierBacklog{},
            UUVector<FixedArray<ColoredVertex<float>, 4>>(),
            std::move(triangles),
            UUVector<FixedArray<ColoredVertex<float>, 2>>(),
            UUVector<FixedArray<std::vector<BoneWeight>, 3>>(),
            UUVector<FixedArray<float, 3>>(),
            UUVector<FixedArray<uint8_t, 3>>(),
            std::vector<UUVector<FixedArray<float, 3, 2>>>(),
            std::vector<UUVector<FixedArray<float, 3>>>(),
            UUVector<FixedArray<float, 3>>()));
}

void DepthMapResource::instantiate_child_renderable(const ChildInstantiationOptions& options) const
{
    rva_->instantiate_child_renderable(options);
}

std::shared_ptr<AnimatedColoredVertexArrays> DepthMapResource::get_physics_arrays() const
{
    return rva_->get_physics_arrays();
}

void DepthMapResource::generate_triangle_rays(size_t npoints, const FixedArray<float, 3>& lengths, bool delete_triangles) {
    return rva_->generate_triangle_rays(npoints, lengths, delete_triangles);
}
