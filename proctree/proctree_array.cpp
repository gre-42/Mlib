#include "proctree_array.hpp"
#include <Mlib/Geometry/Mesh/Triangle_List.hpp>
#include <Mlib/Memory/Integral_Cast.hpp>
#include <proctree/proctree.hpp>

using namespace Mlib;

inline FixedArray<float, 2> to_array(const Proctree::fvec2& v) {
    return {v.u, v.v};
}

inline FixedArray<float, 3> to_array(const Proctree::fvec3& v) {
    return {v.x, v.y, v.z};
}

void draw_arrays(
    TriangleList<float>& tl,
    int vertCount,
    Proctree::fvec3 * vert,
    Proctree::fvec3 * normal,
    Proctree::fvec2 * uv,
    int faceCount,
    Proctree::ivec3 * face)
{
    for (const auto& face : std::span{face, integral_cast<size_t>(faceCount)}) {
        tl.draw_triangle_with_normals(
            to_array(vert[face.x]),
            to_array(vert[face.y]),
            to_array(vert[face.z]),
            to_array(normal[face.x]),
            to_array(normal[face.y]),
            to_array(normal[face.z]),
            Colors::WHITE,
            Colors::WHITE,
            Colors::WHITE,
            to_array(uv[face.x]),
            to_array(uv[face.y]),
            to_array(uv[face.z]));
    }
}

std::list<std::shared_ptr<ColoredVertexArray<float>>> Mlib::proctree_to_cvas(
    const Proctree::Tree& tree,
    const Material& tree_material,
    const Material& twig_material)
{
    auto result = std::list<std::shared_ptr<ColoredVertexArray<float>>>{};
    {
        TriangleList<float> tl{
            "tree",
            tree_material,
            Morphology{PhysicsMaterial::ATTR_VISIBLE},
            ModifierBacklog{}};
        draw_arrays(tl, tree.mVertCount, tree.mVert, tree.mNormal, tree.mUV, tree.mFaceCount, tree.mFace);
        result.emplace_back(tl.triangle_array());
    }
    {
        TriangleList<float> tl{
            "twigs",
            twig_material,
            Morphology{PhysicsMaterial::ATTR_VISIBLE},
            ModifierBacklog{}};
            draw_arrays(tl, tree.mTwigVertCount, tree.mTwigVert, tree.mTwigNormal, tree.mTwigUV, tree.mTwigFaceCount, tree.mTwigFace);
        result.emplace_back(tl.triangle_array());
    }
    return result;
}
