#include "Plot.hpp"
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Coordinates/Homogeneous.hpp>
#include <Mlib/Geometry/Coordinates/Normalized_Points_Fixed.hpp>
#include <Mlib/Images/Coordinates.hpp>
#include <Mlib/Images/StbImage.hpp>
#include <Mlib/Images/Svg.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Orderable_Fixed_Array.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/PTri.hpp>

using namespace Mlib;

template <class TPos>
static void convert_to_2d(
    std::list<FixedArray<FixedArray<TPos, 2>, 3>>& triangles_2d,
    std::list<std::list<FixedArray<TPos, 2>>>& contours_2d,
    std::list<FixedArray<TPos, 2>>& highlighted_nodes_2d,
    std::list<FixedArray<TPos, 2>>& crossed_nodes_2d,
    const std::list<const FixedArray<ColoredVertex<TPos>, 3>*>& triangles_3d,
    const std::list<std::list<FixedArray<TPos, 3>>>& contours_3d,
    const std::list<FixedArray<TPos, 3>>& highlighted_nodes,
    const std::list<FixedArray<TPos, 3>>& crossed_nodes)
{
    for (const auto& t : triangles_3d) {
        triangles_2d.push_back({
            FixedArray<TPos, 2>{(*t)(0).position(0), (*t)(0).position(1)},
            FixedArray<TPos, 2>{(*t)(1).position(0), (*t)(1).position(1)},
            FixedArray<TPos, 2>{(*t)(2).position(0), (*t)(2).position(1)}});
    }
    for (const auto& c : contours_3d) {
        contours_2d.emplace_back();
        for (const auto& p : c) {
            contours_2d.back().push_back({p(0), p(1)});
        }
    }
    for (const auto& n : highlighted_nodes) {
        highlighted_nodes_2d.push_back(FixedArray<TPos, 2>{n(0), n(1)});
    }
    for (const auto& n : crossed_nodes) {
        crossed_nodes_2d.push_back(FixedArray<TPos, 2>{n(0), n(1)});
    }
}

template <class TPos>
struct ConvOrderable {
    ConvOrderable(
        const std::list<FixedArray<OrderableFixedArray<TPos, 2>, 3>>& triangles,
        const std::list<std::vector<OrderableFixedArray<TPos, 2>>>& contours,
        const std::list<OrderableFixedArray<TPos, 2>>& highlighted_nodes,
        const std::list<OrderableFixedArray<TPos, 2>>& crossed_nodes)
    : triangles_2d{reinterpret_cast<const std::list<FixedArray<FixedArray<TPos, 2>, 3>>&>(triangles)},
      highlighted_nodes_2d{reinterpret_cast<const std::list<FixedArray<TPos, 2>>&>(highlighted_nodes)},
      crossed_nodes_2d{reinterpret_cast<const std::list<FixedArray<TPos, 2>>&>(crossed_nodes)}
    {
        for (const auto& c : contours) {
            contours_2d.emplace_back();
            for (const auto& p : c) {
                contours_2d.back().push_back(p);
            }
        }
    }
    const std::list<FixedArray<FixedArray<TPos, 2>, 3>>& triangles_2d;
    std::list<std::list<FixedArray<TPos, 2>>> contours_2d;
    const std::list<FixedArray<TPos, 2>>& highlighted_nodes_2d;
    const std::list<FixedArray<TPos, 2>>& crossed_nodes_2d;
};

struct ConvPtri {
    ConvPtri(
        const std::list<PTri>& triangles,
        const std::list<std::vector<p2t::Point*>>& contours,
        const std::list<p2t::Point*>& highlighted_nodes,
        const std::list<p2t::Point*>& crossed_nodes)
    {
        typedef FixedArray<double, 2>* P;
        for (const auto& t : triangles) {
            triangles_2d.push_back(FixedArray<FixedArray<double, 2>, 3>{
                FixedArray<double, 2>{(double)t(0)->x, (double)t(0)->y},
                    FixedArray<double, 2>{(double)t(1)->x, (double)t(1)->y},
                    FixedArray<double, 2>{(double)t(2)->x, (double)t(2)->y}});
        }
        for (const auto& c : reinterpret_cast<const std::list<std::vector<P>>&>(contours)) {
            contours_2d.emplace_back();
            for (const auto& p : c) {
                contours_2d.back().push_back(FixedArray<double, 2>{p->casted<double>()});
            };
        }
        for (const auto& p : reinterpret_cast<const std::list<P>&>(highlighted_nodes)) {
            highlighted_nodes_2d.push_back(p->casted<double>());
        }
        for (const auto& p : reinterpret_cast<const std::list<P>&>(crossed_nodes)) {
            crossed_nodes_2d.push_back(p->casted<double>());
        }
    }
    std::list<FixedArray<FixedArray<double, 2>, 3>> triangles_2d;
    std::list<std::list<FixedArray<double, 2>>> contours_2d;
    std::list<FixedArray<double, 2>> highlighted_nodes_2d;
    std::list<FixedArray<double, 2>> crossed_nodes_2d;
};

template <class TPos>
StbImage Mlib::plot_mesh(
    const ArrayShape& image_size,
    size_t line_thickness,
    size_t point_size,
    const std::list<FixedArray<FixedArray<TPos, 2>, 3>>& triangles,
    const std::list<std::list<FixedArray<TPos, 2>>>& contours,
    const std::list<FixedArray<TPos, 2>>& highlighted_nodes,
    const std::list<FixedArray<TPos, 2>>& crossed_nodes)
{
    NormalizedPointsFixed<TPos> np{ScaleMode::PRESERVE_ASPECT_RATIO, OffsetMode::MINIMUM};
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
    StbImage im{image_size, Rgb24::white()};
    auto normalization_matrix = np.normalization_matrix();
    auto trafo = [&](const FixedArray<TPos, 2>& p){
        return 0.5f + (normalization_matrix.transform(p) TEMPLATEV casted<float>()).to_array() * (Array<float>::from_shape(im.shape()) - 1.f);
    };
    for (const auto& t : triangles) {
        auto a = trafo(t(0));
        auto b = trafo(t(1));
        auto c = trafo(t(2));
        im.draw_line(a, b, line_thickness, Rgb24::black(), rvalue_address(Rgb24::nan()));
        im.draw_line(b, c, line_thickness, Rgb24::black(), rvalue_address(Rgb24::nan()));
        im.draw_line(c, a, line_thickness, Rgb24::black(), rvalue_address(Rgb24::nan()));
        im.draw_fill_rect(FixedArray<size_t, 2>{a2i(a(0)), a2i(a(1))}, point_size, Rgb24::blue());
        im.draw_fill_rect(FixedArray<size_t, 2>{a2i(b(0)), a2i(b(1))}, point_size, Rgb24::blue());
        im.draw_fill_rect(FixedArray<size_t, 2>{a2i(c(0)), a2i(c(1))}, point_size, Rgb24::blue());
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
            im.draw_fill_rect(FixedArray<size_t, 2>{a2i(a(0)), a2i(a(1))}, point_size, Rgb24::green());
        }
        ++i;
    }
    for (const auto& n : highlighted_nodes) {
        auto a = trafo(n);
        im.draw_fill_rect(FixedArray<size_t, 2>{a2i(a(0)), a2i(a(1))}, point_size, Rgb24::red());
    }
    for (const auto& n : crossed_nodes) {
        auto a = trafo(n);
        im.draw_line(Array<float>{a2fi(a(0)), 0.f}, Array<float>{a2fi(a(0)), (float)(im.shape(1) - 1)}, 1, Rgb24::red());
        im.draw_line(Array<float>{0.f, a2fi(a(1))}, Array<float>{(float)(im.shape(0) - 1), a2fi(a(1))}, 1, Rgb24::red());
    }
    return im;
}

template <class TPos>
void Mlib::plot_mesh(
    Svg<TPos>& svg,
    const std::list<FixedArray<FixedArray<TPos, 2>, 3>>& triangles,
    const std::list<FixedArray<FixedArray<TPos, 2>, 2>>& edges,
    const std::list<std::list<FixedArray<TPos, 2>>>& contours,
    const std::list<FixedArray<TPos, 2>>& highlighted_nodes,
    TPos line_width)
{
    NormalizedPointsFixed<TPos> np{ScaleMode::PRESERVE_ASPECT_RATIO, OffsetMode::MINIMUM};
    for (const auto& t : triangles) {
        np.add_point(t(0));
        np.add_point(t(1));
        np.add_point(t(2));
    }
    for (const auto& e : edges) {
        np.add_point(e(0));
        np.add_point(e(1));
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
    auto trafo = [&](const FixedArray<TPos, 2>& p){
        return TPos{0.5} + (normalization_matrix.transform(p)) * FixedArray<TPos, 2>{svg.width(), svg.height()};
    };
    for (const auto& t : triangles) {
        auto a = trafo(t(0));
        auto b = trafo(t(1));
        auto c = trafo(t(2));
        svg.draw_path(
            {a(0), b(0), c(0)},
            {a(1), b(1), c(1)},
            line_width,
            "#000",         // stroke
            "red",          // fill
            true,           // close
            1);             // down_sampling
        // svg.draw_line(a(0), a(1), b(0), b(1), line_width, "black");
        // svg.draw_line(b(0), b(1), c(0), c(1), line_width, "black");
        // svg.draw_line(c(0), c(1), a(0), a(1), line_width, "black");
        // im.draw_fill_rect(FixedArray<size_t, 2>{a2i(a(0)), a2i(a(1))}, 4, Rgb24::blue());
        // im.draw_fill_rect(FixedArray<size_t, 2>{a2i(b(0)), a2i(b(1))}, 4, Rgb24::blue());
        // im.draw_fill_rect(FixedArray<size_t, 2>{a2i(c(0)), a2i(c(1))}, 4, Rgb24::blue());
    }
    for (const auto& e : edges) {
        auto a = trafo(e(0));
        auto b = trafo(e(1));
        svg.draw_line(a(0), a(1), b(0), b(1), line_width, "blue");
    }
    for (const auto& c : contours) {
        for (auto it = c.begin(); ; ) {
            auto it0 = it++;
            if (it == c.end()) {
                break;
            }
            auto a = trafo(*it0);
            auto b = trafo(*it);
            svg.draw_line(a(0), a(1), b(0), b(1), line_width, "blue");
        }
    }
    for (const auto& n : highlighted_nodes) {
        auto a = trafo(n);
        svg.draw_rectangle(
            a(0) - TPos{0.2}, a(1) - TPos{0.2},
            a(0) + TPos{0.2}, a(1) + TPos{0.2},
            line_width,
            "red",
            TPos{1});
    }
    // if (!contour.empty()) {
    //     auto a = trafo(contour.front());
    //     im.draw_fill_rect(FixedArray<size_t, 2>{a2i(a(0)), a2i(a(1))}, 4, Rgb24::green());
    // }
    // for (const auto& n : highlighted_nodes) {
    //     auto a = trafo(n);
    //     im.draw_fill_rect(FixedArray<size_t, 2>{a2i(a(0)), a2i(a(1))}, 4, Rgb24::red());
    // }
}

template <class TPos>
StbImage Mlib::plot_mesh(
    const ArrayShape& image_size,
    size_t line_thickness,
    size_t point_size,
    const std::list<const FixedArray<ColoredVertex<TPos>, 3>*>& triangles,
    const std::list<std::list<FixedArray<TPos, 3>>>& contours,
    const std::list<FixedArray<TPos, 3>>& highlighted_nodes,
    const std::list<FixedArray<TPos, 3>>& crossed_nodes)
{
    std::list<FixedArray<FixedArray<TPos, 2>, 3>> triangles_2d;
    std::list<std::list<FixedArray<TPos, 2>>> contours_2d;
    std::list<FixedArray<TPos, 2>> highlighted_nodes_2d;
    std::list<FixedArray<TPos, 2>> crossed_nodes_2d;
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

template <class TPos>
void Mlib::plot_mesh(
    Svg<TPos>& svg,
    const std::list<const FixedArray<ColoredVertex<TPos>, 3>*>& triangles,
    const std::list<std::list<FixedArray<TPos, 3>>>& contours,
    const std::list<FixedArray<TPos, 3>>& highlighted_nodes,
    TPos line_width)
{
    std::list<FixedArray<FixedArray<TPos, 2>, 3>> triangles_2d;
    std::list<FixedArray<FixedArray<TPos, 2>, 2>> edges_2d;
    std::list<std::list<FixedArray<TPos, 2>>> contours_2d;
    std::list<FixedArray<TPos, 2>> highlighted_nodes_2d;
    std::list<FixedArray<TPos, 2>> crossed_nodes_2d;
    std::list<FixedArray<TPos, 3>> crossed_nodes;
    convert_to_2d(
        triangles_2d,
        contours_2d,
        highlighted_nodes_2d,
        crossed_nodes_2d,
        triangles,
        contours,
        highlighted_nodes,
        crossed_nodes);
    plot_mesh(svg, triangles_2d, edges_2d, contours_2d, highlighted_nodes_2d, line_width);
}

void Mlib::plot_mesh_svg(
    const std::string& filename,
    double width,
    double height,
    const std::list<const FixedArray<ColoredVertex<double>, 3>*>& triangles,
    const std::list<std::list<FixedArray<double, 3>>>& contour,
    const std::list<FixedArray<double, 3>>& highlighted_nodes,
    double line_width)
{
    std::ofstream ofstr{filename};
    Svg<double> svg{ofstr, width, height};
    if (ofstr.fail()) {
        THROW_OR_ABORT("Could not open file \"" + filename + '"');
    }
    ofstr.precision(15);
    plot_mesh(svg, triangles, contour, highlighted_nodes, line_width);
    svg.finish();
    ofstr.flush();
    if (ofstr.fail()) {
        THROW_OR_ABORT("Could not write to file \"" + filename + '"');
    }
}

void Mlib::plot_mesh_svg(
    const std::string& filename,
    double width,
    double height,
    const std::list<FixedArray<FixedArray<double, 2>, 3>>& triangles,
    const std::list<FixedArray<FixedArray<double, 2>, 2>>& edges,
    const std::list<std::list<FixedArray<double, 2>>>& contours,
    const std::list<FixedArray<double, 2>>& highlighted_nodes,
    double line_width)
{
    std::ofstream ofstr{filename};
    Svg<double> svg{ofstr, width, height};
    if (ofstr.fail()) {
        THROW_OR_ABORT("Could not open file \"" + filename + '"');
    }
    ofstr.precision(15);
    plot_mesh(svg, triangles, edges, contours, highlighted_nodes, line_width);
    svg.finish();
    ofstr.flush();
    if (ofstr.fail()) {
        THROW_OR_ABORT("Could not write to file \"" + filename + '"');
    }
}

void Mlib::plot_mesh_svg(
    const std::string& filename,
    double width,
    double height,
    const std::list<FixedArray<OrderableFixedArray<double, 2>, 3>>& triangles,
    const std::list<std::vector<OrderableFixedArray<double, 2>>>& contours,
    const std::list<OrderableFixedArray<double, 2>>& highlighted_nodes,
    double line_width)
{
    ConvOrderable c{
        triangles,
        contours,
        highlighted_nodes,
        {}};
    std::ofstream ofstr{ filename };
    Svg<double> svg{ ofstr, width, height };
    if (ofstr.fail()) {
        THROW_OR_ABORT("Could not open file \"" + filename + '"');
    }
    ofstr.precision(15);
    plot_mesh(
        svg,
        c.triangles_2d,
        {},
        c.contours_2d,
        c.highlighted_nodes_2d,
        line_width);
    svg.finish();
    ofstr.flush();
    if (ofstr.fail()) {
        THROW_OR_ABORT("Could not write to file \"" + filename + '"');
    }
}

void Mlib::plot_mesh_svg(
    const std::string& filename,
    double width,
    double height,
    const std::list<PTri>& triangles,
    const std::list<std::vector<p2t::Point*>>& contours,
    const std::list<p2t::Point*>& highlighted_nodes,
    double line_width)
{
    ConvPtri c{
        triangles,
        contours,
        highlighted_nodes,
        {} };
    std::ofstream ofstr{ filename };
    Svg<double> svg{ ofstr, width, height };
    if (ofstr.fail()) {
        THROW_OR_ABORT("Could not open file \"" + filename + '"');
    }
    ofstr.precision(15);
    plot_mesh(
        svg,
        c.triangles_2d,
        {},
        c.contours_2d,
        c.highlighted_nodes_2d,
        line_width);
    svg.finish();
    ofstr.flush();
    if (ofstr.fail()) {
        THROW_OR_ABORT("Could not write to file \"" + filename + '"');
    }
}

template <class TPos>
StbImage Mlib::plot_mesh(
    const ArrayShape& image_size,
    size_t line_thickness,
    size_t point_size,
    const std::list<FixedArray<OrderableFixedArray<TPos, 2>, 3>>& triangles,
    const std::list<std::vector<OrderableFixedArray<TPos, 2>>>& contours,
    const std::list<OrderableFixedArray<TPos, 2>>& highlighted_nodes,
    const std::list<OrderableFixedArray<TPos, 2>>& crossed_nodes)
{
    ConvOrderable c{
        triangles,
        contours,
        highlighted_nodes,
        crossed_nodes};
    return plot_mesh(
        image_size,
        line_thickness,
        point_size,
        c.triangles_2d,
        c.contours_2d,
        c.highlighted_nodes_2d,
        c.crossed_nodes_2d);
}

StbImage Mlib::plot_mesh(
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

template
StbImage Mlib::plot_mesh(
    const ArrayShape& image_size,
    size_t line_thickness,
    size_t point_size,
    const std::list<FixedArray<FixedArray<float, 2>, 3>>& triangles,
    const std::list<std::list<FixedArray<float, 2>>>& contours,
    const std::list<FixedArray<float, 2>>& highlighted_nodes,
    const std::list<FixedArray<float, 2>>& crossed_nodes);

template
StbImage Mlib::plot_mesh(
    const ArrayShape& image_size,
    size_t line_thickness,
    size_t point_size,
    const std::list<const FixedArray<ColoredVertex<float>, 3>*>& triangles,
    const std::list<std::list<FixedArray<float, 3>>>& contours,
    const std::list<FixedArray<float, 3>>& highlighted_nodes,
    const std::list<FixedArray<float, 3>>& crossed_nodes);

template
StbImage Mlib::plot_mesh(
    const ArrayShape& image_size,
    size_t line_thickness,
    size_t point_size,
    const std::list<FixedArray<OrderableFixedArray<float, 2>, 3>>& triangles,
    const std::list<std::vector<OrderableFixedArray<float, 2>>>& contours,
    const std::list<OrderableFixedArray<float, 2>>& highlighted_nodes,
    const std::list<OrderableFixedArray<float, 2>>& crossed_nodes);

template
StbImage Mlib::plot_mesh(
    const ArrayShape& image_size,
    size_t line_thickness,
    size_t point_size,
    const std::list<FixedArray<FixedArray<double, 2>, 3>>& triangles,
    const std::list<std::list<FixedArray<double, 2>>>& contours,
    const std::list<FixedArray<double, 2>>& highlighted_nodes,
    const std::list<FixedArray<double, 2>>& crossed_nodes);

template
StbImage Mlib::plot_mesh(
    const ArrayShape& image_size,
    size_t line_thickness,
    size_t point_size,
    const std::list<const FixedArray<ColoredVertex<double>, 3>*>& triangles,
    const std::list<std::list<FixedArray<double, 3>>>& contours,
    const std::list<FixedArray<double, 3>>& highlighted_nodes,
    const std::list<FixedArray<double, 3>>& crossed_nodes);

template
StbImage Mlib::plot_mesh(
    const ArrayShape& image_size,
    size_t line_thickness,
    size_t point_size,
    const std::list<FixedArray<OrderableFixedArray<double, 2>, 3>>& triangles,
    const std::list<std::vector<OrderableFixedArray<double, 2>>>& contours,
    const std::list<OrderableFixedArray<double, 2>>& highlighted_nodes,
    const std::list<OrderableFixedArray<double, 2>>& crossed_nodes);

template
void Mlib::plot_mesh(
    Svg<float>& svg,
    const std::list<FixedArray<FixedArray<float, 2>, 3>>& triangles,
    const std::list<FixedArray<FixedArray<float, 2>, 2>>& edges,
    const std::list<std::list<FixedArray<float, 2>>>& contours,
    const std::list<FixedArray<float, 2>>& highlighted_nodes,
    float line_width);

template
void Mlib::plot_mesh(
    Svg<double>& svg,
    const std::list<FixedArray<FixedArray<double, 2>, 3>>& triangles,
    const std::list<FixedArray<FixedArray<double, 2>, 2>>& edges,
    const std::list<std::list<FixedArray<double, 2>>>& contours,
    const std::list<FixedArray<double, 2>>& highlighted_nodes,
    double line_width);

template
void Mlib::plot_mesh(
    Svg<float>& svg,
    const std::list<const FixedArray<ColoredVertex<float>, 3>*>& triangles,
    const std::list<std::list<FixedArray<float, 3>>>& contours,
    const std::list<FixedArray<float, 3>>& highlighted_nodes,
    float line_width);

template
void Mlib::plot_mesh(
    Svg<double>& svg,
    const std::list<const FixedArray<ColoredVertex<double>, 3>*>& triangles,
    const std::list<std::list<FixedArray<double, 3>>>& contours,
    const std::list<FixedArray<double, 3>>& highlighted_nodes,
    double line_width);
