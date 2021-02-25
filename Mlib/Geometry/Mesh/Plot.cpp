#include "Plot.hpp"
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Homogeneous.hpp>
#include <Mlib/Geometry/Normalized_Points_Fixed.hpp>
#include <Mlib/Images/Coordinates.hpp>
#include <Mlib/Images/PpmImage.hpp>
#include <Mlib/Images/Svg.hpp>
#include <Mlib/Math/Fixed_Math.hpp>

using namespace Mlib;

static void convert_to_2d(
    std::list<FixedArray<FixedArray<float, 2>, 3>>& triangles_2d,
    std::list<FixedArray<float, 2>>& contour_2d,
    std::list<FixedArray<float, 2>>& highlighted_nodes_2d,
    const std::list<const FixedArray<ColoredVertex, 3>*>& triangles_3d,
    const std::list<FixedArray<float, 3>>& contour_3d,
    const std::list<FixedArray<float, 3>>& highlighted_nodes)
{
    for (const auto& t : triangles_3d) {
        triangles_2d.push_back({
            FixedArray<float, 2>{(*t)(0).position(0), (*t)(0).position(1)},
            FixedArray<float, 2>{(*t)(1).position(0), (*t)(1).position(1)},
            FixedArray<float, 2>{(*t)(2).position(0), (*t)(2).position(1)}});
    }
    for (const auto& c : contour_3d) {
        contour_2d.push_back({c(0), c(1)});
    }
    for (const auto& n : highlighted_nodes) {
        highlighted_nodes_2d.push_back(FixedArray<float, 2>{n(0), n(1)});
    }
}

PpmImage Mlib::plot_mesh(
    const ArrayShape& image_size,
    size_t line_thickness,
    size_t point_size,
    const std::list<FixedArray<FixedArray<float, 2>, 3>>& triangles,
    const std::list<FixedArray<float, 2>>& contour,
    const std::list<FixedArray<float, 2>>& highlighted_nodes)
{
    NormalizedPointsFixed np{ScaleMode::PRESERVE_ASPECT_RATIO, OffsetMode::MINIMUM};
    for (const auto& t : triangles) {
        np.add_point(t(0));
        np.add_point(t(1));
        np.add_point(t(2));
    }
    for (const auto& c : contour) {
        np.add_point(c);
    }
    for (const auto& n : highlighted_nodes) {
        np.add_point(n);
    }
    PpmImage im{image_size, Rgb24::white()};
    auto normalization_matrix = np.normalization_matrix();
    auto trafo = [&](const FixedArray<float, 2>& p){
        return 0.5f + (normalization_matrix.transform(p)).to_array() * (Array<float>::from_shape(im.shape()) - 1.f);
    };
    for (const auto& t : triangles) {
        auto a = trafo(t(0));
        auto b = trafo(t(1));
        auto c = trafo(t(2));
        im.draw_line(a, b, line_thickness, Rgb24::black(), rvalue_address(Rgb24::nan()));
        im.draw_line(b, c, line_thickness, Rgb24::black(), rvalue_address(Rgb24::nan()));
        im.draw_line(c, a, line_thickness, Rgb24::black(), rvalue_address(Rgb24::nan()));
        im.draw_fill_rect(ArrayShape{a2i(a(0)), a2i(a(1))}, point_size, Rgb24::blue());
        im.draw_fill_rect(ArrayShape{a2i(b(0)), a2i(b(1))}, point_size, Rgb24::blue());
        im.draw_fill_rect(ArrayShape{a2i(c(0)), a2i(c(1))}, point_size, Rgb24::blue());
    }
    for (auto it = contour.begin(); ; ) {
        auto it0 = it++;
        if (it == contour.end()) {
            break;
        }
        im.draw_line(trafo(*it0), trafo(*it), 1, Rgb24::red(), rvalue_address(Rgb24::nan()));

    }
    if (!contour.empty()) {
        auto a = trafo(contour.front());
        im.draw_fill_rect(ArrayShape{a2i(a(0)), a2i(a(1))}, point_size, Rgb24::green());
    }
    for (const auto& n : highlighted_nodes) {
        auto a = trafo(n);
        im.draw_fill_rect(ArrayShape{a2i(a(0)), a2i(a(1))}, point_size, Rgb24::red());
    }
    return im;
}

void Mlib::plot_mesh(
    Svg<float>& svg,
    const std::list<FixedArray<FixedArray<float, 2>, 3>>& triangles,
    const std::list<FixedArray<float, 2>>& contour,
    const std::list<FixedArray<float, 2>>& highlighted_nodes)
{
    NormalizedPointsFixed np{ScaleMode::PRESERVE_ASPECT_RATIO, OffsetMode::MINIMUM};
    for (const auto& t : triangles) {
        np.add_point(t(0));
        np.add_point(t(1));
        np.add_point(t(2));
    }
    for (const auto& c : contour) {
        np.add_point(c);
    }
    for (const auto& n : highlighted_nodes) {
        np.add_point(n);
    }
    auto normalization_matrix = np.normalization_matrix();
    auto trafo = [&](const FixedArray<float, 2>& p){
        return 0.5f + (normalization_matrix.transform(p)) * FixedArray<float, 2>{svg.width(), svg.height()};
    };
    for (const auto& t : triangles) {
        auto a = trafo(t(0));
        auto b = trafo(t(1));
        auto c = trafo(t(2));
        svg.draw_line(a(0), a(1), b(0), b(1), 0.05f, "black");
        svg.draw_line(b(0), b(1), c(0), c(1), 0.05f, "black");
        svg.draw_line(c(0), c(1), a(0), a(1), 0.05f, "black");
        // im.draw_fill_rect(ArrayShape{a2i(a(0)), a2i(a(1))}, 4, Rgb24::blue());
        // im.draw_fill_rect(ArrayShape{a2i(b(0)), a2i(b(1))}, 4, Rgb24::blue());
        // im.draw_fill_rect(ArrayShape{a2i(c(0)), a2i(c(1))}, 4, Rgb24::blue());
    }
    for (auto it = contour.begin(); ; ) {
        auto it0 = it++;
        if (it == contour.end()) {
            break;
        }
        auto a = trafo(*it0);
        auto b = trafo(*it);
        svg.draw_line(a(0), a(1), b(0), b(1), 1.f, "red");

    }
    for (const auto& n : highlighted_nodes) {
        auto a = trafo(n);
        svg.draw_rectangle(
            a(0) - 0.05f, a(1) - 0.05f,
            a(0) + 0.05f, a(1) + 0.05f,
            0.05f);
    }
    // if (!contour.empty()) {
    //     auto a = trafo(contour.front());
    //     im.draw_fill_rect(ArrayShape{a2i(a(0)), a2i(a(1))}, 4, Rgb24::green());
    // }
    // for (const auto& n : highlighted_nodes) {
    //     auto a = trafo(n);
    //     im.draw_fill_rect(ArrayShape{a2i(a(0)), a2i(a(1))}, 4, Rgb24::red());
    // }
}

PpmImage Mlib::plot_mesh(
    const ArrayShape& image_size,
    size_t line_thickness,
    size_t point_size,
    const std::list<const FixedArray<ColoredVertex, 3>*>& triangles,
    const std::list<FixedArray<float, 3>>& contour,
    const std::list<FixedArray<float, 3>>& highlighted_nodes)
{
    std::list<FixedArray<FixedArray<float, 2>, 3>> triangles2d;
    std::list<FixedArray<float, 2>> contour2d;
    std::list<FixedArray<float, 2>> highlighted_nodes2d;
    convert_to_2d(
        triangles2d,
        contour2d,
        highlighted_nodes2d,
        triangles,
        contour,
        highlighted_nodes);
    return plot_mesh(image_size, line_thickness, point_size, triangles2d, contour2d, highlighted_nodes2d);
}

void Mlib::plot_mesh(
    Svg<float>& svg,
    const std::list<const FixedArray<ColoredVertex, 3>*>& triangles,
    const std::list<FixedArray<float, 3>>& contour,
    const std::list<FixedArray<float, 3>>& highlighted_nodes)
{
    std::list<FixedArray<FixedArray<float, 2>, 3>> triangles2d;
    std::list<FixedArray<float, 2>> contour2d;
    std::list<FixedArray<float, 2>> highlighted_nodes2d;
    convert_to_2d(
        triangles2d,
        contour2d,
        highlighted_nodes2d,
        triangles,
        contour,
        highlighted_nodes);
    plot_mesh(svg, triangles2d, contour2d, highlighted_nodes2d);
}

void Mlib::plot_mesh_svg(
    const std::string& filename,
    float width,
    float height,
    const std::list<const FixedArray<ColoredVertex, 3>*>& triangles,
    const std::list<FixedArray<float, 3>>& contour,
    const std::list<FixedArray<float, 3>>& highlighted_nodes)
{
    std::ofstream ofstr{filename};
    Svg<float> svg{ofstr, width, height};
    if (ofstr.fail()) {
        throw std::runtime_error("Could not open file \"" + filename + '"');
    }
    plot_mesh(svg, triangles, contour, highlighted_nodes);
    svg.finish();
    ofstr.flush();
    if (ofstr.fail()) {
        throw std::runtime_error("Could not write to file \"" + filename + '"');
    }
}
