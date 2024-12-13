#include "Shade_Auto.hpp"
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Vertex_Normals_With_Seams.hpp>
#include <Mlib/Geometry/Triangle_Normal.hpp>

using namespace Mlib;

template <class TPos>
void Mlib::shade_auto(ColoredVertexArray<TPos>& cva, float seam_angle)
{
    if (seam_angle == 0.f) {
        for (auto& t : cva.triangles) {
            using Triangle = FixedArray<TPos, 3, 3>;
            auto normal = triangle_normal(funpack(Triangle{
                t(0).position,
                t(1).position,
                t(2).position})).template casted<float>();
            t(0).normal = normal;
            t(1).normal = normal;
            t(2).normal = normal;
        }
    } else {
        float seam_threshold = std::cos(seam_angle);
        VertexNormalsWithSeams<TPos, float> vns;
        vns.add_triangles(cva.triangles.begin(), cva.triangles.end());
        for (auto& t : cva.triangles) {
            t = vns.triangle(t, seam_threshold);
        }
    }
}

template void Mlib::shade_auto<float>(
    ColoredVertexArray<float>& cvas,
    float seam_angle);
template void Mlib::shade_auto<CompressedScenePos>(
    ColoredVertexArray<CompressedScenePos>& cvas,
    float seam_angle);
