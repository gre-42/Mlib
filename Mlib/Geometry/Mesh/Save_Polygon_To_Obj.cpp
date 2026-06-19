#include "Save_Polygon_To_Obj.hpp"
#include <Mlib/Geometry/Mesh/Save_Obj.hpp>
#include <Mlib/Initialization/Default_Uninitialized_Vector.hpp>

using namespace Mlib;

void Mlib::save_polygon_to_obj(
    const std::string& filename,
    const FixedArray<double, 2, 3>& edge)
{
    throw std::runtime_error("\"save_polygon_to_obj\" not implemented for edges");
}

void Mlib::save_polygon_to_obj(
    const std::string& filename,
    const FixedArray<double, 3, 3>& triangle)
{
    DefaultUnitialized<FixedArray<ColoredVertex<double>, 3>> ctri{
        ColoredVertex<double>{
            triangle[0],
            Colors::RED},
        ColoredVertex<double>{
            triangle[1],
            Colors::GREEN},
        ColoredVertex<double>{
            triangle[2],
            Colors::BLUE}
    };
    save_obj(filename, IndexedFaceSet<float, double, size_t>{ std::vector{ ctri } }, nullptr);
}

void Mlib::save_polygon_to_obj(
    const std::string& filename,
    const FixedArray<double, 4, 3>& quad)
{
    DefaultUnitialized<FixedArray<ColoredVertex<double>, 4>> cquad{
        ColoredVertex<double>{
            quad[0],
            Colors::RED},
        ColoredVertex<double>{
            quad[1],
            Colors::GREEN},
        ColoredVertex<double>{
            quad[2],
            Colors::BLUE},
        ColoredVertex<double>{
            quad[3],
            Colors::PURPLE}
    };
    save_obj(filename, IndexedFaceSet<float, double, size_t>{ {}, std::vector{ cquad } }, nullptr);
}
