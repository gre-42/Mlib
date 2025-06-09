#pragma once
#include <Mlib/Scene_Precision.hpp>
#include <cstddef>
#include <list>
#include <map>
#include <memory>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
template <typename TData, size_t... tshape>
class OrderableFixedArray;
template <class TPos>
class ColoredVertexArray;
template <class TPos>
class TriangleList;

std::list<std::list<FixedArray<CompressedScenePos, 2>>> height_contours_by_vertex(
    const std::list<std::shared_ptr<ColoredVertexArray<CompressedScenePos>>>& cvas,
    CompressedScenePos height);

std::list<std::list<FixedArray<CompressedScenePos, 2>>> height_contours_by_vertex(
    const std::list<std::shared_ptr<TriangleList<CompressedScenePos>>>& cvas,
    CompressedScenePos height);

std::list<std::list<FixedArray<CompressedScenePos, 2>>> height_contours_by_edge(
    const std::list<std::shared_ptr<ColoredVertexArray<CompressedScenePos>>>& cvas,
    CompressedScenePos height);


std::list<std::list<FixedArray<CompressedScenePos, 2>>> height_contours_by_edge(
    const std::list<std::shared_ptr<TriangleList<CompressedScenePos>>>& cvas,
    CompressedScenePos height);

class HeightContoursByVertex {
public:
    explicit HeightContoursByVertex(CompressedScenePos height);
    ~HeightContoursByVertex();
    void add_triangle(const FixedArray<CompressedScenePos, 3, 3>& triangle);
    std::list<std::list<FixedArray<CompressedScenePos, 2>>> get_contours_and_clear();
private:
    CompressedScenePos height_;
    std::map<OrderableFixedArray<CompressedScenePos, 2>, FixedArray<CompressedScenePos, 2>> neighbors_;
};

class HeightContoursByEdge {
    using Edge = std::pair<OrderableFixedArray<CompressedScenePos, 3>, OrderableFixedArray<CompressedScenePos, 3>>;
public:
    explicit HeightContoursByEdge(CompressedScenePos height);
    ~HeightContoursByEdge();
    void add_triangle(const FixedArray<CompressedScenePos, 3, 3>& triangle);
    std::list<std::list<FixedArray<CompressedScenePos, 2>>> get_contours_and_clear();
private:
    static Edge swap(const Edge& e);
    CompressedScenePos height_;
    std::map<Edge, Edge> edge_neighbors_;
};

}
