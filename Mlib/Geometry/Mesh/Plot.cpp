#include "Plot.hpp"
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Homogeneous.hpp>
#include <Mlib/Geometry/Normalized_Points_Fixed.hpp>
#include <Mlib/Images/Coordinates.hpp>
#include <Mlib/Images/PpmImage.hpp>
#include <Mlib/Images/Svg.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Orderable_Fixed_Array.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/PTri.hpp>

using namespace Mlib;

static void convert_to_2d(
    std::list<FixedArray<FixedArray<float, 2>, 3>>& triangles_2d,
    std::list<std::list<FixedArray<float, 2>>>& contours_2d,
    std::list<FixedArray<float, 2>>& highlighted_nodes_2d,
    std::list<FixedArray<float, 2>>& crossed_nodes_2d,
    const std::list<const FixedArray<ColoredVertex, 3>*>& triangles_3d,
    const std::list<std::list<FixedArray<float, 3>>>& contours_3d,
    const std::list<FixedArray<float, 3>>& highlighted_nodes,
    const std::list<FixedArray<float, 3>>& crossed_nodes)
{
    for (const auto& t : triangles_3d) {
        triangles_2d.push_back({
            FixedArray<float, 2>{(*t)(0).position(0), (*t)(0).position(1)},
            FixedArray<float, 2>{(*t)(1).position(0), (*t)(1).position(1)},
            FixedArray<float, 2>{(*t)(2).position(0), (*t)(2).position(1)}});
    }
    for (const auto& c : contours_3d) {
        contours_2d.emplace_back();
        for (const auto& p : c) {
            contours_2d.back().push_back({p(0), p(1)});
        }
    }
    for (const auto& n : highlighted_nodes) {
        highlighted_nodes_2d.push_back(FixedArray<float, 2>{n(0), n(1)});
    }
    for (const auto& n : crossed_nodes) {
        crossed_nodes_2d.push_back(FixedArray<float, 2>{n(0), n(1)});
    }
}

struct ConvPtri {
    ConvPtri(
        const std::list<PTri>& triangles,
        const std::list<std::vector<p2t::Point*>>& contours,
        const std::list<p2t::Point*>& highlighted_nodes,
        const std::list<p2t::Point*>& crossed_nodes)
    {
        typedef FixedArray<double, 2>* P;
        for (const auto& t : triangles) {
            triangles_2d.push_back(FixedArray<FixedArray<float, 2>, 3>{
                FixedArray<float, 2>{(float)t(0)->x, (float)t(0)->y},
                    FixedArray<float, 2>{(float)t(1)->x, (float)t(1)->y},
                    FixedArray<float, 2>{(float)t(2)->x, (float)t(2)->y}});
        }
        for (const auto& c : reinterpret_cast<const std::list<std::vector<P>>&>(contours)) {
            contours_2d.emplace_back();
            for (const auto& p : c) {
                contours_2d.back().push_back(FixedArray<float, 2>{p->casted<float>()});
            };
        }
        for (const auto& p : reinterpret_cast<const std::list<P>&>(highlighted_nodes)) {
            highlighted_nodes_2d.push_back(p->casted<float>());
        }
        for (const auto& p : reinterpret_cast<const std::list<P>&>(crossed_nodes)) {
            crossed_nodes_2d.push_back(p->casted<float>());
        }
    }
    std::list<FixedArray<FixedArray<float, 2>, 3>> triangles_2d;
    std::list<std::list<FixedArray<float, 2>>> contours_2d;
    std::list<FixedArray<float, 2>> highlighted_nodes_2d;
    std::list<FixedArray<float, 2>> crossed_nodes_2d;
};

PpmImage Mlib::plot_mesh(
    const ArrayShape& image_size,
    size_t line_thickness,
    size_t point_size,
    const std::list<FixedArray<FixedArray<float, 2>, 3>>& triangles,
    const std::list<std::list<FixedArray<float, 2>>>& contours,
    const std::list<FixedArray<float, 2>>& highlighted_nodes,
    const std::list<FixedArray<float, 2>>& crossed_nodes)
{
    NormalizedPointsFixed np{ScaleMode::PRESERVE_ASPECT_RATIO, OffsetMode::MINIMUM};
    for (const auto& t : triangles) {
        np.add_point(t(0));
        np.add_point(t(1));
        np.add_point(t(2));
    }
    for (const auto& c : contours) {
        for (const auto& p : c) {
            np.add_point(p);
        }
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
    size_t i = 0;
    for (const auto& c : contours) {
        for (auto it = c.begin(); ; ) {
            auto it0 = it++;
            if (it == c.end()) {
                break;
            }
            im.draw_line(trafo(*it0), trafo(*it), 1, (i % 2 == 0) ? Rgb24::red() : Rgb24::orange(), rvalue_address(Rgb24::nan()));
        }
        if (!c.empty()) {
            auto a = trafo(c.front());
            im.draw_fill_rect(ArrayShape{a2i(a(0)), a2i(a(1))}, point_size, Rgb24::green());
        }
        ++i;
    }
    for (const auto& n : highlighted_nodes) {
        auto a = trafo(n);
        im.draw_fill_rect(ArrayShape{a2i(a(0)), a2i(a(1))}, point_size, Rgb24::red());
    }
    for (const auto& n : crossed_nodes) {
        auto a = trafo(n);
        im.draw_line(Array<float>{a2fi(a(0)), 0.f}, Array<float>{a2fi(a(0)), (float)(im.shape(1) - 1)}, 1, Rgb24::red());
        im.draw_line(Array<float>{0.f, a2fi(a(1))}, Array<float>{(float)(im.shape(0) - 1), a2fi(a(1))}, 1, Rgb24::red());
    }
    return im;
}

void Mlib::plot_mesh(
    Svg<float>& svg,
    const std::list<FixedArray<FixedArray<float, 2>, 3>>& triangles,
    const std::list<std::list<FixedArray<float, 2>>>& contours,
    const std::list<FixedArray<float, 2>>& highlighted_nodes)
{
    NormalizedPointsFixed np{ScaleMode::PRESERVE_ASPECT_RATIO, OffsetMode::MINIMUM};
    for (const auto& t : triangles) {
        np.add_point(t(0));
        np.add_point(t(1));
        np.add_point(t(2));
    }
    for (const auto& c : contours) {
        for (const auto& p : c) {
            np.add_point(p);
        }
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
    for (const auto& c : contours) {
        for (auto it = c.begin(); ; ) {
            auto it0 = it++;
            if (it == c.end()) {
                break;
            }
            auto a = trafo(*it0);
            auto b = trafo(*it);
            svg.draw_line(a(0), a(1), b(0), b(1), 0.05f, "blue");
        }
    }
    for (const auto& n : highlighted_nodes) {
        auto a = trafo(n);
        svg.draw_rectangle(
            a(0) - 0.2f, a(1) - 0.2f,
            a(0) + 0.2f, a(1) + 0.2f,
            0.05f,
            "red",
            1.f);
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
    const std::list<std::list<FixedArray<float, 3>>>& contours,
    const std::list<FixedArray<float, 3>>& highlighted_nodes,
    const std::list<FixedArray<float, 3>>& crossed_nodes)
{
    std::list<FixedArray<FixedArray<float, 2>, 3>> triangles_2d;
    std::list<std::list<FixedArray<float, 2>>> contours_2d;
    std::list<FixedArray<float, 2>> highlighted_nodes_2d;
    std::list<FixedArray<float, 2>> crossed_nodes_2d;
    convert_to_2d(
        triangles_2d,
        contours_2d,
        highlighted_nodes_2d,
        crossed_nodes_2d,
        triangles,
        contours,
        highlighted_nodes,
        crossed_nodes);
    return plot_mesh(image_size, line_thickness, point_size, triangles_2d, contours_2d, highlighted_nodes_2d, crossed_nodes_2d);
}

void Mlib::plot_mesh(
    Svg<float>& svg,
    const std::list<const FixedArray<ColoredVertex, 3>*>& triangles,
    const std::list<std::list<FixedArray<float, 3>>>& contours,
    const std::list<FixedArray<float, 3>>& highlighted_nodes)
{
    std::list<FixedArray<FixedArray<float, 2>, 3>> triangles_2d;
    std::list<std::list<FixedArray<float, 2>>> contours_2d;
    std::list<FixedArray<float, 2>> highlighted_nodes_2d;
    std::list<FixedArray<float, 2>> crossed_nodes_2d;
    std::list<FixedArray<float, 3>> crossed_nodes;
    convert_to_2d(
        triangles_2d,
        contours_2d,
        highlighted_nodes_2d,
        crossed_nodes_2d,
        triangles,
        contours,
        highlighted_nodes,
        crossed_nodes);
    plot_mesh(svg, triangles_2d, contours_2d, highlighted_nodes_2d);
}

void Mlib::plot_mesh_svg(
    const std::string& filename,
    float width,
    float height,
    const std::list<const FixedArray<ColoredVertex, 3>*>& triangles,
    const std::list<std::list<FixedArray<float, 3>>>& contour,
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

void Mlib::plot_mesh_svg(
    const std::string& filename,
    float width,
    float height,
    const std::list<PTri>& triangles,
    const std::list<std::vector<p2t::Point*>>& contours,
    const std::list<p2t::Point*>& highlighted_nodes)
{
    ConvPtri c{
        triangles,
        contours,
        highlighted_nodes,
        {} };
    std::ofstream ofstr{ filename };
    Svg<float> svg{ ofstr, width, height };
    if (ofstr.fail()) {
        throw std::runtime_error("Could not open file \"" + filename + '"');
    }
    plot_mesh(
        svg,
        c.triangles_2d,
        c.contours_2d,
        c.highlighted_nodes_2d);
}

PpmImage Mlib::plot_mesh(
    const ArrayShape& image_size,
    size_t line_thickness,
    size_t point_size,
    const std::list<FixedArray<OrderableFixedArray<float, 2>, 3>>& triangles,
    const std::list<std::vector<OrderableFixedArray<float, 2>>>& contours,
    const std::list<OrderableFixedArray<float, 2>>& highlighted_nodes,
    const std::list<OrderableFixedArray<float, 2>>& crossed_nodes)
{
    std::list<std::list<FixedArray<float, 2>>> contoursR;
    for (const auto& c : contours) {
        contoursR.emplace_back();
        for (const auto& p : c) {
            contoursR.back().push_back(p);
        }
    }
    return plot_mesh(
        image_size,
        line_thickness,
        point_size,
        reinterpret_cast<const std::list<FixedArray<FixedArray<float, 2>, 3>>&>(triangles),
        contoursR,
        reinterpret_cast<const std::list<FixedArray<float, 2>>&>(highlighted_nodes),
        reinterpret_cast<const std::list<FixedArray<float, 2>>&>(crossed_nodes));
}

PpmImage Mlib::plot_mesh(
    const ArrayShape& image_size,
    size_t line_thickness,
    size_t point_size,
    const std::list<PTri>& triangles,
    const std::list<std::vector<p2t::Point*>>& contours,
    const std::list<p2t::Point*>& highlighted_nodes,
    const std::list<p2t::Point*>& crossed_nodes)
{
    ConvPtri c{
        triangles,
        contours,
        highlighted_nodes,
        crossed_nodes };
    return plot_mesh(
        image_size,
        line_thickness,
        point_size,
        c.triangles_2d,
        c.contours_2d,
        c.highlighted_nodes_2d,
        c.crossed_nodes_2d);
}
