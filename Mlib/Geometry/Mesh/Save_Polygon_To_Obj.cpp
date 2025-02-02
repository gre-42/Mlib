#include "Save_Polygon_To_Obj.hpp"
#include <Mlib/Default_Uninitialized_Vector.hpp>
#include <Mlib/Geometry/Mesh/Save_Obj.hpp>

using namespace Mlib;

void Mlib::save_triangle_to_obj(
    const std::string& filename,
    const FixedArray<double, 3, 3>& highlighted_triangle)
{
    DefaultUnitialized<FixedArray<ColoredVertex<double>, 3>> ctri{
        ColoredVertex<double>{
            highlighted_triangle[0],
            Colors::RED},
        ColoredVertex<double>{
            highlighted_triangle[1],
            Colors::GREEN},
        ColoredVertex<double>{
            highlighted_triangle[2],
            Colors::BLUE}
    };
    save_obj(filename, IndexedFaceSet<float, double, size_t>{ std::vector{ ctri } }, nullptr);
}

void Mlib::save_quad_to_obj(
    const std::string& filename,
    const FixedArray<double, 4, 3>& highlighted_quad)
{
    DefaultUnitialized<FixedArray<ColoredVertex<double>, 4>> cquad{
        ColoredVertex<double>{
            highlighted_quad[0],
            Colors::RED},
        ColoredVertex<double>{
            highlighted_quad[1],
            Colors::GREEN},
        ColoredVertex<double>{
            highlighted_quad[2],
            Colors::BLUE},
        ColoredVertex<double>{
            highlighted_quad[3],
            Colors::PURPLE}
    };
    save_obj(filename, IndexedFaceSet<float, double, size_t>{ {}, std::vector{ cquad } }, nullptr);
}
