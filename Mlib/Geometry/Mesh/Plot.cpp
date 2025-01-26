#include "Plot.hpp"
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Coordinates/Homogeneous.hpp>
#include <Mlib/Geometry/Coordinates/Normalized_Points_Fixed.hpp>
#include <Mlib/Images/Coordinates.hpp>
#include <Mlib/Images/StbImage3.hpp>
#include <Mlib/Images/Svg.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Orderable_Fixed_Array.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/PTri.hpp>
#include <Mlib/Scene_Precision.hpp>

using namespace Mlib;

template <class TPos>
static void convert_to_2d(
    std::list<FixedArray<TPos, 3, 2>>& triangles_2d,
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
        const std::list<FixedArray<TPos, 3, 2>>& triangles,
        const std::list<std::vector<FixedArray<TPos, 2>>>& contours,
        const std::list<FixedArray<TPos, 2>>& highlighted_nodes,
        const std::list<FixedArray<TPos, 2>>& crossed_nodes)
    : triangles_2d{reinterpret_cast<const std::list<FixedArray<TPos, 3, 2>>&>(triangles)},
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
    const std::list<FixedArray<TPos, 3, 2>>& triangles_2d;
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
        auto T = [](const p2t::Point* v) {
            return FixedArray<double, 2>{ v->x, v->y};
            };
        for (const auto& t : triangles) {
            triangles_2d.push_back(FixedArray<double, 3, 2>{
                T(t[0]), T(t[1]), T(t[2]) });
        }
        for (const auto& c : contours) {
            auto& c2d = contours_2d.emplace_back();
            for (const auto& p : c) {
                c2d.push_back(T(p));
            };
        }
        for (const auto& p : highlighted_nodes) {
            highlighted_nodes_2d.push_back(T(p));
        }
        for (const auto& p : crossed_nodes) {
            crossed_nodes_2d.push_back(T(p));
        }
    }
    std::list<FixedArray<double, 3, 2>> triangles_2d;
    std::list<std::list<FixedArray<double, 2>>> contours_2d;
    std::list<FixedArray<double, 2>> highlighted_nodes_2d;
    std::list<FixedArray<double, 2>> crossed_nodes_2d;
};

template <class TPos>
StbImage3 Mlib::plot_mesh(
    const FixedArray<size_t, 2>& image_size,
    size_t line_thickness,
    size_t point_size,
    const std::list<FixedArray<TPos, 3, 2>>& triangles,
    const std::list<std::list<FixedArray<TPos, 2>>>& contours,
    const std::list<FixedArray<TPos, 2>>& highlighted_nodes,
    const std::list<FixedArray<TPos, 2>>& crossed_nodes)
{
    if (any(image_size == (size_t)0)) {
        THROW_OR_ABORT("Image size cannot be zero");
    }
    using I = funpack_t<TPos>;
    NormalizedPointsFixed<I> np{ScaleMode::PRESERVE_ASPECT_RATIO, OffsetMode::MINIMUM};
    for (const auto& t : triangles) {
        np.add_point(funpack(t[0]));
        np.add_point(funpack(t[1]));
        np.add_point(funpack(t[2]));
    }
    for (const auto& c : contours) {
        for (const auto& p : c) {
            np.add_point(funpack(p));
        }
    }
    for (const auto& n : highlighted_nodes) {
        np.add_point(funpack(n));
    }
    StbImage3 im{ image_size, Rgb24::white() };
    auto normalization_matrix = np.normalization_matrix();
    auto trafo = [&](const FixedArray<TPos, 2>& p) {
        return ((I)0.5 + normalization_matrix.transform(funpack(p)) * (image_size - (size_t)1).casted<I>()).template casted<float>();
        };
    for (const auto& t : triangles) {
        auto a = trafo(t[0]);
        auto b = trafo(t[1]);
        auto c = trafo(t[2]);
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
        im.draw_line(FixedArray<float, 2>{a2fi(a(0)), 0.f}, FixedArray<float, 2>{a2fi(a(0)), (float)(im.shape(1) - 1)}, 1, Rgb24::red());
        im.draw_line(FixedArray<float, 2>{0.f, a2fi(a(1))}, FixedArray<float, 2>{(float)(im.shape(0) - 1), a2fi(a(1))}, 1, Rgb24::red());
    }
    return im;
}

template <class TPos>
void Mlib::plot_mesh(
    Svg<double>& svg,
    const std::list<FixedArray<TPos, 3, 2>>& triangles,
    const std::list<FixedArray<TPos, 2, 2>>& edges,
    const std::list<std::list<FixedArray<TPos, 2>>>& contours,
    const std::list<FixedArray<TPos, 2>>& highlighted_nodes,
    TPos line_width)
{
    using I = funpack_t<TPos>;
    NormalizedPointsFixed<I> np{ScaleMode::PRESERVE_ASPECT_RATIO, OffsetMode::MINIMUM};
    for (const auto& t : triangles) {
        np.add_point(funpack(t[0]));
        np.add_point(funpack(t[1]));
        np.add_point(funpack(t[2]));
    }
    for (const auto& e : edges) {
        np.add_point(funpack(e[0]));
        np.add_point(funpack(e[1]));
    }
    for (const auto& c : contours) {
        for (const auto& p : c) {
            np.add_point(funpack(p));
        }
    }
    for (const auto& n : highlighted_nodes) {
        np.add_point(funpack(n));
    }
    auto normalization_matrix = np.normalization_matrix();
    auto trafo = [&](const FixedArray<TPos, 2>& p){
        FixedArray<double, 2> size{ svg.width(), svg.height() };
        return (normalization_matrix.transform(funpack(p)) * size.template casted<I>()).template casted<TPos>();
    };
    for (const auto& t : triangles) {
        auto a = trafo(t[0]);
        auto b = trafo(t[1]);
        auto c = trafo(t[2]);
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
        auto a = trafo(e[0]);
        auto b = trafo(e[1]);
        svg.draw_line(a(0), a(1), b(0), b(1), line_width, "blue");
    }
    for (const auto& c : contours) {
        std::vector<TPos> x;
        std::vector<TPos> y;
        x.reserve(c.size());
        y.reserve(c.size());
        for (const auto& p : c) {
            auto pt = trafo(p);
            x.push_back(pt(0));
            y.push_back(pt(1));
        }
        svg.draw_path(x, y, line_width, "blue");
    }
    for (const auto& n : highlighted_nodes) {
        auto a = trafo(n);
        svg.draw_rectangle(
            a(0) - TPos(0.2), a(1) - TPos(0.2),
            a(0) + TPos(0.2), a(1) + TPos(0.2),
            line_width,
            "red",
            TPos{1.});
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
StbImage3 Mlib::plot_mesh(
    const FixedArray<size_t, 2>& image_size,
    size_t line_thickness,
    size_t point_size,
    const std::list<const FixedArray<ColoredVertex<TPos>, 3>*>& triangles,
    const std::list<std::list<FixedArray<TPos, 3>>>& contours,
    const std::list<FixedArray<TPos, 3>>& highlighted_nodes,
    const std::list<FixedArray<TPos, 3>>& crossed_nodes)
{
    std::list<FixedArray<TPos, 3, 2>> triangles_2d;
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
    Svg<double>& svg,
    const std::list<const FixedArray<ColoredVertex<TPos>, 3>*>& triangles,
    const std::list<std::list<FixedArray<TPos, 3>>>& contours,
    const std::list<FixedArray<TPos, 3>>& highlighted_nodes,
    TPos line_width)
{
    std::list<FixedArray<TPos, 3, 2>> triangles_2d;
    std::list<FixedArray<TPos, 2, 2>> edges_2d;
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
    const std::list<const FixedArray<ColoredVertex<CompressedScenePos>, 3>*>& triangles,
    const std::list<std::list<FixedArray<CompressedScenePos, 3>>>& contour,
    const std::list<FixedArray<CompressedScenePos, 3>>& highlighted_nodes,
    CompressedScenePos line_width)
{
    std::ofstream ofstr{filename};
    Svg<double> svg{ ofstr, width, height };
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
    const std::list<FixedArray<CompressedScenePos, 3, 2>>& triangles,
    const std::list<FixedArray<CompressedScenePos, 2, 2>>& edges,
    const std::list<std::list<FixedArray<CompressedScenePos, 2>>>& contours,
    const std::list<FixedArray<CompressedScenePos, 2>>& highlighted_nodes,
    CompressedScenePos line_width)
{
    std::ofstream ofstr{filename};
    Svg<double> svg{ ofstr, width, height };
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
    const std::list<FixedArray<CompressedScenePos, 3, 2>>& triangles,
    const std::list<std::vector<FixedArray<CompressedScenePos, 2>>>& contours,
    const std::list<FixedArray<CompressedScenePos, 2>>& highlighted_nodes,
    CompressedScenePos line_width)
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
StbImage3 Mlib::plot_mesh(
    const FixedArray<size_t, 2>& image_size,
    size_t line_thickness,
    size_t point_size,
    const std::list<FixedArray<TPos, 3, 2>>& triangles,
    const std::list<std::vector<FixedArray<TPos, 2>>>& contours,
    const std::list<FixedArray<TPos, 2>>& highlighted_nodes,
    const std::list<FixedArray<TPos, 2>>& crossed_nodes)
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

StbImage3 Mlib::plot_mesh(
    const FixedArray<size_t, 2>& image_size,
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
StbImage3 Mlib::plot_mesh(
    const FixedArray<size_t, 2>& image_size,
    size_t line_thickness,
    size_t point_size,
    const std::list<FixedArray<float, 3, 2>>& triangles,
    const std::list<std::list<FixedArray<float, 2>>>& contours,
    const std::list<FixedArray<float, 2>>& highlighted_nodes,
    const std::list<FixedArray<float, 2>>& crossed_nodes);

template
StbImage3 Mlib::plot_mesh(
    const FixedArray<size_t, 2>& image_size,
    size_t line_thickness,
    size_t point_size,
    const std::list<const FixedArray<ColoredVertex<float>, 3>*>& triangles,
    const std::list<std::list<FixedArray<float, 3>>>& contours,
    const std::list<FixedArray<float, 3>>& highlighted_nodes,
    const std::list<FixedArray<float, 3>>& crossed_nodes);

template
StbImage3 Mlib::plot_mesh(
    const FixedArray<size_t, 2>& image_size,
    size_t line_thickness,
    size_t point_size,
    const std::list<FixedArray<float, 3, 2>>& triangles,
    const std::list<std::vector<FixedArray<float, 2>>>& contours,
    const std::list<FixedArray<float, 2>>& highlighted_nodes,
    const std::list<FixedArray<float, 2>>& crossed_nodes);

template
StbImage3 Mlib::plot_mesh(
    const FixedArray<size_t, 2>& image_size,
    size_t line_thickness,
    size_t point_size,
    const std::list<FixedArray<CompressedScenePos, 3, 2>>& triangles,
    const std::list<std::list<FixedArray<CompressedScenePos, 2>>>& contours,
    const std::list<FixedArray<CompressedScenePos, 2>>& highlighted_nodes,
    const std::list<FixedArray<CompressedScenePos, 2>>& crossed_nodes);

template
StbImage3 Mlib::plot_mesh(
    const FixedArray<size_t, 2>& image_size,
    size_t line_thickness,
    size_t point_size,
    const std::list<const FixedArray<ColoredVertex<CompressedScenePos>, 3>*>& triangles,
    const std::list<std::list<FixedArray<CompressedScenePos, 3>>>& contours,
    const std::list<FixedArray<CompressedScenePos, 3>>& highlighted_nodes,
    const std::list<FixedArray<CompressedScenePos, 3>>& crossed_nodes);

template
StbImage3 Mlib::plot_mesh(
    const FixedArray<size_t, 2>& image_size,
    size_t line_thickness,
    size_t point_size,
    const std::list<FixedArray<CompressedScenePos, 3, 2>>& triangles,
    const std::list<std::vector<FixedArray<CompressedScenePos, 2>>>& contours,
    const std::list<FixedArray<CompressedScenePos, 2>>& highlighted_nodes,
    const std::list<FixedArray<CompressedScenePos, 2>>& crossed_nodes);

template
void Mlib::plot_mesh(
    Svg<double>& svg,
    const std::list<FixedArray<float, 3, 2>>& triangles,
    const std::list<FixedArray<float, 2, 2>>& edges,
    const std::list<std::list<FixedArray<float, 2>>>& contours,
    const std::list<FixedArray<float, 2>>& highlighted_nodes,
    float line_width);

template
void Mlib::plot_mesh(
    Svg<double>& svg,
    const std::list<FixedArray<CompressedScenePos, 3, 2>>& triangles,
    const std::list<FixedArray<CompressedScenePos, 2, 2>>& edges,
    const std::list<std::list<FixedArray<CompressedScenePos, 2>>>& contours,
    const std::list<FixedArray<CompressedScenePos, 2>>& highlighted_nodes,
    CompressedScenePos line_width);

template
void Mlib::plot_mesh(
    Svg<double>& svg,
    const std::list<const FixedArray<ColoredVertex<float>, 3>*>& triangles,
    const std::list<std::list<FixedArray<float, 3>>>& contours,
    const std::list<FixedArray<float, 3>>& highlighted_nodes,
    float line_width);

template
void Mlib::plot_mesh(
    Svg<double>& svg,
    const std::list<const FixedArray<ColoredVertex<CompressedScenePos>, 3>*>& triangles,
    const std::list<std::list<FixedArray<CompressedScenePos, 3>>>& contours,
    const std::list<FixedArray<CompressedScenePos, 3>>& highlighted_nodes,
    CompressedScenePos line_width);
