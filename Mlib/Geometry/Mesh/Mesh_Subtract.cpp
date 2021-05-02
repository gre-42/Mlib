#if !defined(__MINGW32__) && !defined(_MSC_VER)

#include "Mesh_Subtract.hpp"
#include <Mlib/Geometry/Mesh/Indexed_Face_Set.hpp>
#include <cork/cork.h>

using namespace Mlib;

static CorkTriMesh ifs_to_cork(const IndexedFaceSet<float, unsigned int>& a)
{
    return CorkTriMesh{
        .n_triangles = (unsigned int)a.triangles.size(),
        .n_vertices = (unsigned int)a.vertices.size(),
        .triangles = (unsigned int*)a.triangles.data(),
        .vertices = (float*)a.vertices.data()};
}

void Mlib::mesh_subtract(
    std::list<FixedArray<ColoredVertex, 3>>& a,
    const std::list<FixedArray<ColoredVertex, 3>>& b)
{
    IndexedFaceSet<float, unsigned int> ifs_a{a};
    IndexedFaceSet<float, unsigned int> ifs_b{b};
    CorkTriMesh in0 = ifs_to_cork(ifs_a);
    CorkTriMesh in1 = ifs_to_cork(ifs_b);
    CorkTriMesh out;
    computeDifference(in0, in1, &out);
    a.clear();
    for (unsigned int* ctri = out.triangles; ctri != out.triangles + out.n_triangles; ctri += 3)
    {
        FixedArray<ColoredVertex, 3> tri;
        for (size_t i = 0; i < 3; ++i) {
            tri(i).position = FixedArray<float, 3>{out.vertices[*(ctri + i)]};
            tri(i).color = FixedArray<float, 3>{1, 1, 1};
            tri(i).uv = FixedArray<float, 2>{0.f, 0.f};
            tri(i).normal = FixedArray<float, 3>{0.f, 1.f, 0.f};
            tri(i).tangent = FixedArray<float, 3>{1.f, 0.f, 0.f};
        }
    }
    freeCorkTriMesh(&out);
}

#endif
