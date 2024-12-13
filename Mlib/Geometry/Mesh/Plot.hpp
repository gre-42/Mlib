#pragma once
#include <Mlib/Scene_Precision.hpp>
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
template <class TPos>
struct ColoredVertex;
class PgmImage;
template <class TSize>
class Svg;
class PTri;
class StbImage3;

template <class TPos>
StbImage3 plot_mesh(
    const FixedArray<size_t, 2>& image_size,
    size_t line_thickness,
    size_t point_size,
    const std::list<FixedArray<TPos, 3, 2>>& triangles,
    const std::list<std::list<FixedArray<TPos, 2>>>& contours,
    const std::list<FixedArray<TPos, 2>>& highlighted_nodes,
    const std::list<FixedArray<TPos, 2>>& crossed_nodes);

template <class TPos>
StbImage3 plot_mesh(
    const FixedArray<size_t, 2>& image_size,
    size_t line_thickness,
    size_t point_size,
    const std::list<const FixedArray<ColoredVertex<TPos>, 3>*>& triangles,
    const std::list<std::list<FixedArray<TPos, 3>>>& contours,
    const std::list<FixedArray<TPos, 3>>& highlighted_nodes,
    const std::list<FixedArray<TPos, 3>>& crossed_nodes);

template <class TPos>
StbImage3 plot_mesh(
    const FixedArray<size_t, 2>& image_size,
    size_t line_thickness,
    size_t point_size,
    const std::list<FixedArray<TPos, 3, 2>>& triangles,
    const std::list<std::vector<FixedArray<TPos, 2>>>& contours,
    const std::list<FixedArray<TPos, 2>>& highlighted_nodes,
    const std::list<FixedArray<TPos, 2>>& crossed_nodes);

StbImage3 plot_mesh(
    const FixedArray<size_t, 2>& image_size,
    size_t line_thickness,
    size_t point_size,
    const std::list<PTri>& triangles,
    const std::list<std::vector<p2t::Point*>>& contours,
    const std::list<p2t::Point*>& highlighted_nodes,
    const std::list<p2t::Point*>& crossed_nodes);

template <class TPos>
void plot_mesh(
    Svg<double>& svg,
    const std::list<FixedArray<TPos, 3, 2>>& triangles,
    const std::list<FixedArray<TPos, 2, 2>>& edges,
    const std::list<std::list<FixedArray<TPos, 2>>>& contours,
    const std::list<FixedArray<TPos, 2>>& highlighted_nodes,
    TPos line_width = 0.05f);

template <class TPos>
void plot_mesh(
    Svg<double>& svg,
    const std::list<const FixedArray<ColoredVertex<TPos>, 3>*>& triangles,
    const std::list<std::list<FixedArray<TPos, 3>>>& contours,
    const std::list<FixedArray<TPos, 3>>& highlighted_nodes,
    TPos line_width = 0.05f);

void plot_mesh_svg(
    const std::string& filename,
    double width,
    double height,
    const std::list<const FixedArray<ColoredVertex<CompressedScenePos>, 3>*>& triangles,
    const std::list<std::list<FixedArray<CompressedScenePos, 3>>>& contours,
    const std::list<FixedArray<CompressedScenePos, 3>>& highlighted_nodes,
    CompressedScenePos line_width = (CompressedScenePos)0.5f);

void plot_mesh_svg(
    const std::string& filename,
    double width,
    double height,
    const std::list<FixedArray<CompressedScenePos, 3, 2>>& triangles,
    const std::list<FixedArray<CompressedScenePos, 2, 2>>& edges,
    const std::list<std::list<FixedArray<CompressedScenePos, 2>>>& contours,
    const std::list<FixedArray<CompressedScenePos, 2>>& highlighted_nodes,
    CompressedScenePos line_width = (CompressedScenePos)0.5f);

void plot_mesh_svg(
    const std::string& filename,
    double width,
    double height,
    const std::list<FixedArray<CompressedScenePos, 3, 2>>& triangles,
    const std::list<std::vector<FixedArray<CompressedScenePos, 2>>>& contours,
    const std::list<FixedArray<CompressedScenePos, 2>>& highlighted_nodes,
    CompressedScenePos line_width = (CompressedScenePos)0.5f);

void plot_mesh_svg(
    const std::string& filename,
    double width,
    double height,
    const std::list<PTri>& triangles,
    const std::list<std::vector<p2t::Point*>>& contours,
    const std::list<p2t::Point*>& highlighted_nodes,
    double line_width = 0.05f);

}
