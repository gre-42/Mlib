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
struct ColoredVertex;
class PgmImage;
template <class TSize>
class Svg;
class PTri;
class StbImage;
class ArrayShape;

StbImage plot_mesh(
    const ArrayShape& image_size,
    size_t line_thickness,
    size_t point_size,
    const std::list<FixedArray<FixedArray<float, 2>, 3>>& triangles,
    const std::list<std::list<FixedArray<float, 2>>>& contours,
    const std::list<FixedArray<float, 2>>& highlighted_nodes,
    const std::list<FixedArray<float, 2>>& crossed_nodes);

StbImage plot_mesh(
    const ArrayShape& image_size,
    size_t line_thickness,
    size_t point_size,
    const std::list<const FixedArray<ColoredVertex, 3>*>& triangles,
    const std::list<std::list<FixedArray<float, 3>>>& contours,
    const std::list<FixedArray<float, 3>>& highlighted_nodes,
    const std::list<FixedArray<float, 3>>& crossed_nodes);

StbImage plot_mesh(
    const ArrayShape& image_size,
    size_t line_thickness,
    size_t point_size,
    const std::list<FixedArray<OrderableFixedArray<float, 2>, 3>>& triangles,
    const std::list<std::vector<OrderableFixedArray<float, 2>>>& contours,
    const std::list<OrderableFixedArray<float, 2>>& highlighted_nodes,
    const std::list<OrderableFixedArray<float, 2>>& crossed_nodes);

StbImage plot_mesh(
    const ArrayShape& image_size,
    size_t line_thickness,
    size_t point_size,
    const std::list<PTri>& triangles,
    const std::list<std::vector<p2t::Point*>>& contours,
    const std::list<p2t::Point*>& highlighted_nodes,
    const std::list<p2t::Point*>& crossed_nodes);

void plot_mesh(
    Svg<float>& svg,
    const std::list<FixedArray<FixedArray<float, 2>, 3>>& triangles,
    const std::list<std::list<FixedArray<float, 2>>>& contours,
    const std::list<FixedArray<float, 2>>& highlighted_nodes);

void plot_mesh(
    Svg<float>& svg,
    const std::list<const FixedArray<ColoredVertex, 3>*>& triangles,
    const std::list<std::list<FixedArray<float, 3>>>& contours,
    const std::list<FixedArray<float, 3>>& highlighted_nodes);

void plot_mesh_svg(
    const std::string& filename,
    float width,
    float height,
    const std::list<const FixedArray<ColoredVertex, 3>*>& triangles,
    const std::list<std::list<FixedArray<float, 3>>>& contours,
    const std::list<FixedArray<float, 3>>& highlighted_nodes);

void plot_mesh_svg(
    const std::string& filename,
    float width,
    float height,
    const std::list<FixedArray<OrderableFixedArray<float, 2>, 3>>& triangles,
    const std::list<std::vector<OrderableFixedArray<float, 2>>>& contours,
    const std::list<OrderableFixedArray<float, 2>>& highlighted_nodes);

void plot_mesh_svg(
    const std::string& filename,
    float width,
    float height,
    const std::list<PTri>& triangles,
    const std::list<std::vector<p2t::Point*>>& contours,
    const std::list<p2t::Point*>& highlighted_nodes);

}
