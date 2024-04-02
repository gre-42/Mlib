#include "Save_Polygon_To_Obj.hpp"
#include <Mlib/Geometry/Mesh/Save_Obj.hpp>

using namespace Mlib;

void Mlib::save_triangle_to_obj(
    const std::string& filename,
    const FixedArray<FixedArray<double, 3>, 3>& highlighted_triangle)
{
    FixedArray<ColoredVertex<double>, 3> ctri{
        ColoredVertex<double>{
            .position = highlighted_triangle(0),
            .color = {1.f, 0.f, 0.f},
            .uv = {0.f, 0.f},
            .normal = {0.f, 0.f, 0.f},
            .tangent = {0.f, 0.f, 0.f}},
        ColoredVertex<double>{
            .position = highlighted_triangle(1),
            .color = {0.f, 1.f, 0.f},
            .uv = {0.f, 0.f},
            .normal = {0.f, 0.f, 0.f},
            .tangent = {0.f, 0.f, 0.f}},
        ColoredVertex<double>{
            .position = highlighted_triangle(2),
            .color = {0.f, 0.f, 1.f},
            .uv = {0.f, 0.f},
            .normal = {0.f, 0.f, 0.f},
            .tangent = {0.f, 0.f, 0.f}}
    };
    save_obj(filename, IndexedFaceSet<float, double, size_t>{ std::vector{ ctri } }, nullptr);
}

void Mlib::save_quad_to_obj(
    const std::string& filename,
    const FixedArray<FixedArray<double, 3>, 4>& highlighted_quad)
{
    FixedArray<ColoredVertex<double>, 4> cquad{
        ColoredVertex<double>{
            .position = highlighted_quad(0),
            .color = {1.f, 0.f, 0.f},
            .uv = {0.f, 0.f},
            .normal = {0.f, 0.f, 0.f},
            .tangent = {0.f, 0.f, 0.f}},
        ColoredVertex<double>{
            .position = highlighted_quad(1),
            .color = {0.f, 1.f, 0.f},
            .uv = {0.f, 0.f},
            .normal = {0.f, 0.f, 0.f},
            .tangent = {0.f, 0.f, 0.f}},
        ColoredVertex<double>{
            .position = highlighted_quad(2),
            .color = {0.f, 0.f, 1.f},
            .uv = {0.f, 0.f},
            .normal = {0.f, 0.f, 0.f},
            .tangent = {0.f, 0.f, 0.f}},
        ColoredVertex<double>{
            .position = highlighted_quad(3),
            .color = {1.f, 0.f, 1.f},
            .uv = {0.f, 0.f},
            .normal = {0.f, 0.f, 0.f},
            .tangent = {0.f, 0.f, 0.f}}
    };
    save_obj(filename, IndexedFaceSet<float, double, size_t>{ {}, std::vector{ cquad } }, nullptr);
}
