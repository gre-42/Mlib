#include "Plot.hpp"
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Homogeneous.hpp>
#include <Mlib/Geometry/Normalized_Points_Fixed.hpp>
#include <Mlib/Images/Coordinates.hpp>
#include <Mlib/Images/PpmImage.hpp>
#include <Mlib/Math/Fixed_Math.hpp>

using namespace Mlib;

PpmImage Mlib::plot_mesh(
    const ArrayShape& image_size,
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
        return 0.5f + dot1d(normalization_matrix, homogenized_3(p)).to_array() * (Array<float>::from_shape(im.shape()) - 1.f);
    };
    for (const auto& t : triangles) {
        auto a = trafo(t(0));
        auto b = trafo(t(1));
        auto c = trafo(t(2));
        im.draw_line(a, b, 1, Rgb24::black());
        im.draw_line(b, c, 1, Rgb24::black());
        im.draw_line(c, a, 1, Rgb24::black());
        im.draw_fill_rect(ArrayShape{a2i(a(0)), a2i(a(1))}, 4, Rgb24::blue());
        im.draw_fill_rect(ArrayShape{a2i(b(0)), a2i(b(1))}, 4, Rgb24::blue());
        im.draw_fill_rect(ArrayShape{a2i(c(0)), a2i(c(1))}, 4, Rgb24::blue());
    }
    for (auto it = contour.begin(); ; ) {
        auto it0 = it++;
        if (it == contour.end()) {
            break;
        }
        im.draw_line(trafo(*it0), trafo(*it), 1, Rgb24::red());

    }
    if (!contour.empty()) {
        auto a = trafo(contour.front());
        im.draw_fill_rect(ArrayShape{a2i(a(0)), a2i(a(1))}, 4, Rgb24::green());
    }
    for (const auto& n : highlighted_nodes) {
        auto a = trafo(n);
        im.draw_fill_rect(ArrayShape{a2i(a(0)), a2i(a(1))}, 4, Rgb24::red());
    }
    return im;
}

PpmImage Mlib::plot_mesh(
    const ArrayShape& image_size,
    const std::list<FixedArray<ColoredVertex, 3>>& triangles,
    const std::list<FixedArray<float, 3>>& contour,
    const std::list<FixedArray<float, 3>>& highlighted_nodes)
{
    std::list<FixedArray<FixedArray<float, 2>, 3>> triangles2d;
    for (const auto& t : triangles) {
        triangles2d.push_back({
            FixedArray<float, 2>{t(0).position(0), t(0).position(1)},
            FixedArray<float, 2>{t(1).position(0), t(1).position(1)},
            FixedArray<float, 2>{t(2).position(0), t(2).position(1)}});
    }
    std::list<FixedArray<float, 2>> contour2d;
    for (const auto& c : contour) {
        contour2d.push_back({c(0), c(1)});
    }
    std::list<FixedArray<float, 2>> highlighted_nodes2d;
    for (const auto& n : highlighted_nodes) {
        highlighted_nodes2d.push_back(FixedArray<float, 2>{n(0), n(1)});
    }
    return plot_mesh(image_size, triangles2d, contour2d, highlighted_nodes2d);
}
