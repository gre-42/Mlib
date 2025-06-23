#include "Height_Map_Resource.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Coordinates/Homogeneous.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Vertex_Normals.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Geometry/Triangle_Normal.hpp>
#include <Mlib/Images/Coordinates_Fixed.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Render/Normal_Type.hpp>
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource.hpp>

using namespace Mlib;

HeightMapResource::HeightMapResource(
    const Array<float>& rgb_picture,
    const Array<float>& height_picture,
    const TransformationMatrix<float, float, 2>& normalization_matrix,
    NormalType normal_type)
{
    UUVector<FixedArray<ColoredVertex<float>, 3>> triangles;
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
            FixedArray<float, 3> p00{
                pos0(0),
                -pos0(1),
                Z(r, c)};
            FixedArray<float, 3> c00{
                R(r, c),
                G(r, c),
                B(r, c)};
            FixedArray<float, 3> p01{
                pos1(0),
                -pos0(1),
                Z(r, c + 1)};
            FixedArray<float, 3> c01{
                R(r, c + 1),
                G(r, c + 1),
                B(r, c + 1)};
            FixedArray<float, 3> p10{
                pos0(0),
                -pos1(1),
                Z(r + 1, c)};
            FixedArray<float, 3> c10{
                R(r + 1, c),
                G(r + 1, c),
                B(r + 1, c)};
            FixedArray<float, 3> p11{
                pos1(0),
                -pos1(1),
                Z(r + 1, c + 1)};
            FixedArray<float, 3> c11{
                R(r + 1, c + 1),
                G(r + 1, c + 1),
                B(r + 1, c + 1)};

            auto add_triangle = [&triangles](
                const FixedArray<float, 3>& p0,
                const FixedArray<float, 3>& c0,
                const FixedArray<float, 3>& p1,
                const FixedArray<float, 3>& c1,
                const FixedArray<float, 3>& p2,
                const FixedArray<float, 3>& c2) {
                FixedArray<float, 3> normal = triangle_normal<float>({p0, p1, p2});
                triangles.emplace_back(
                    ColoredVertex<float>{
                        p0,
                        Colors::from_rgb(c0),
                        fixed_zeros<float, 2>(),
                        normal},
                    ColoredVertex<float>{
                        p1,
                        Colors::from_rgb(c1),
                        fixed_zeros<float, 2>(),
                        normal},
                    ColoredVertex<float>{
                        p2,
                        Colors::from_rgb(c2),
                        fixed_zeros<float, 2>(),
                        normal});
            };
            add_triangle(p00, c00, p11, c11, p01, c01);
            add_triangle(p11, c11, p00, c00, p10, c10);
        }
    }
    if (normal_type == NormalType::VERTEX) {
        VertexNormals<float, float> vertex_normals;
        vertex_normals.add_triangles(
            triangles.begin(),
            triangles.end());
        vertex_normals.compute_vertex_normals(NormalVectorErrorBehavior::THROW);
        for (auto& it : triangles) {
            for (auto& v : it.flat_iterable()) {
                v.normal = vertex_normals.get_normal(v.position);
            }
        }
    }
    rva_ = std::make_shared<ColoredVertexArrayResource>(
        std::make_shared<ColoredVertexArray<float>>(
            "HeightMapResource",
            Material{},
            Morphology{ .physics_material = PhysicsMaterial::ATTR_VISIBLE },
            ModifierBacklog{},
            UUVector<FixedArray<ColoredVertex<float>, 4>>(),
            std::move(triangles),
            UUVector<FixedArray<ColoredVertex<float>, 2>>(),
            UUVector<FixedArray<std::vector<BoneWeight>, 3>>(),
            UUVector<FixedArray<float, 3>>(),
            UUVector<FixedArray<uint8_t, 3>>(),
            std::vector<UUVector<FixedArray<float, 3, 2>>>(),
            std::vector<UUVector<FixedArray<float, 3>>>(),
            UUVector<FixedArray<float, 3>>(),
            UUVector<FixedArray<float, 4>>()));
}

HeightMapResource::~HeightMapResource()
{}

void HeightMapResource::preload(const RenderableResourceFilter& filter) const {
    rva_->preload(filter);
}

void HeightMapResource::instantiate_child_renderable(const ChildInstantiationOptions& options) const
{
    rva_->instantiate_child_renderable(options);
}

std::shared_ptr<AnimatedColoredVertexArrays> HeightMapResource::get_arrays(
    const ColoredVertexArrayFilter& filter) const
{
    return rva_->get_arrays(filter);
}

void HeightMapResource::generate_triangle_rays(size_t npoints, const FixedArray<float, 3>& lengths, bool delete_triangles) {
    return rva_->generate_triangle_rays(npoints, lengths, delete_triangles);
}
