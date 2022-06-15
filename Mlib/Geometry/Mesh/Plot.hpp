#pragma once
#include <cstddef>
#include <list>
#include <string>
#include <vector>

namespace p2t {

struct Point;

}

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
template <typename TData, size_t... tshape>
class OrderableFixedArray;
template <class TPos>
struct ColoredVertex;
class PgmImage;
template <class TSize>
class Svg;
class PTri;
class StbImage;
class ArrayShape;

template <class TPos>
StbImage plot_mesh(
    const ArrayShape& image_size,
    size_t line_thickness,
    size_t point_size,
    const std::list<FixedArray<FixedArray<TPos, 2>, 3>>& triangles,
    const std::list<std::list<FixedArray<TPos, 2>>>& contours,
    const std::list<FixedArray<TPos, 2>>& highlighted_nodes,
    const std::list<FixedArray<TPos, 2>>& crossed_nodes);

template <class TPos>
StbImage plot_mesh(
    const ArrayShape& image_size,
    size_t line_thickness,
    size_t point_size,
    const std::list<const FixedArray<ColoredVertex<TPos>, 3>*>& triangles,
    const std::list<std::list<FixedArray<TPos, 3>>>& contours,
    const std::list<FixedArray<TPos, 3>>& highlighted_nodes,
    const std::list<FixedArray<TPos, 3>>& crossed_nodes);

template <class TPos>
StbImage plot_mesh(
    const ArrayShape& image_size,
    size_t line_thickness,
    size_t point_size,
    const std::list<FixedArray<OrderableFixedArray<TPos, 2>, 3>>& triangles,
    const std::list<std::vector<OrderableFixedArray<TPos, 2>>>& contours,
    const std::list<OrderableFixedArray<TPos, 2>>& highlighted_nodes,
    const std::list<OrderableFixedArray<TPos, 2>>& crossed_nodes);

StbImage plot_mesh(
    const ArrayShape& image_size,
    size_t line_thickness,
    size_t point_size,
    const std::list<PTri>& triangles,
    const std::list<std::vector<p2t::Point*>>& contours,
    const std::list<p2t::Point*>& highlighted_nodes,
    const std::list<p2t::Point*>& crossed_nodes);

void plot_mesh(
    Svg<double>& svg,
    const std::list<FixedArray<FixedArray<double, 2>, 3>>& triangles,
    const std::list<FixedArray<FixedArray<double, 2>, 2>>& edges,
    const std::list<std::list<FixedArray<double, 2>>>& contours,
    const std::list<FixedArray<double, 2>>& highlighted_nodes,
    double line_width = 0.05f);

void plot_mesh(
    Svg<double>& svg,
    const std::list<const FixedArray<ColoredVertex<double>, 3>*>& triangles,
    const std::list<std::list<FixedArray<double, 3>>>& contours,
    const std::list<FixedArray<double, 3>>& highlighted_nodes,
    double line_width = 0.05f);

void plot_mesh_svg(
    const std::string& filename,
    double width,
    double height,
    const std::list<const FixedArray<ColoredVertex<double>, 3>*>& triangles,
    const std::list<std::list<FixedArray<double, 3>>>& contours,
    const std::list<FixedArray<double, 3>>& highlighted_nodes,
    double line_width = 0.05f);

void plot_mesh_svg(
    const std::string& filename,
    double width,
    double height,
    const std::list<FixedArray<FixedArray<double, 2>, 3>>& triangles,
    const std::list<FixedArray<FixedArray<double, 2>, 2>>& edges,
    const std::list<std::list<FixedArray<double, 2>>>& contours,
    const std::list<FixedArray<double, 2>>& highlighted_nodes,
    double line_width = 0.05f);

void plot_mesh_svg(
    const std::string& filename,
    double width,
    double height,
    const std::list<FixedArray<OrderableFixedArray<double, 2>, 3>>& triangles,
    const std::list<std::vector<OrderableFixedArray<double, 2>>>& contours,
    const std::list<OrderableFixedArray<double, 2>>& highlighted_nodes,
    double line_width = 0.05f);

void plot_mesh_svg(
    const std::string& filename,
    double width,
    double height,
    const std::list<PTri>& triangles,
    const std::list<std::vector<p2t::Point*>>& contours,
    const std::list<p2t::Point*>& highlighted_nodes,
    double line_width = 0.05f);

}
