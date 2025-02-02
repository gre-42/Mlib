#pragma once
#include "Points_And_Adjacency.hpp"
#include <Mlib/Assert.hpp>
#include <Mlib/Geometry/Intersection/Bvh.hpp>
#include <Mlib/Geometry/Intersection/Fuzzy_Set_Of_Points_impl.hpp>
#include <Mlib/Images/Svg.hpp>
#include <Mlib/Iterator/Enumerate.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

namespace Mlib {

template <class TPoint>
PointsAndAdjacency<TPoint>::PointsAndAdjacency(size_t npoints)
    : points(npoints)
    , adjacency(npoints, npoints)
{}

template <class TPoint>
void PointsAndAdjacency<TPoint>::update_adjacency() {
    for (auto&& [c, col] : enumerate(adjacency.columns())) {
        for (auto& [r, value] : col) {
            value = (TData)std::sqrt(sum(squared(points.at(c) - points.at(r))));
        }
    }
}

template <class TPoint>
void PointsAndAdjacency<TPoint>::update_adjacency_diagonal() {
    for (auto&& [c, col] : enumerate(adjacency.columns())) {
        col[c] = (TData)0.f;
    }
}

template <class TPoint>
void PointsAndAdjacency<TPoint>::transform(const TransformationMatrix<float, TData, tlength>& m) {
    adjacency *= m.get_scale();
    for (auto& p : points) {
        using PP = FixedArray<TData, TPoint::length()>;
        PP& pp = p;
        pp = m.transform(funpack(pp)).template casted<TData>();
    }
}

template <class TPoint>
template <class TCalculateIntermediatePoints>
void PointsAndAdjacency<TPoint>::subdivide(
    const TCalculateIntermediatePoints& calculate_intermediate_points,
    SubdivisionType subdivision_type)
{
    std::map<std::tuple<size_t, size_t, size_t>, size_t> new_point_ids;
    std::list<TPoint> new_points;
    std::map<size_t, std::map<size_t, TData>> new_columns;
    {
        for (auto&& [c, col] : enumerate(adjacency.columns())) {
            for (typename std::map<size_t, TData>::iterator row = col.begin(); row != col.end();) {
                size_t r = row->first;
                if (r == c) {
                    ++row;
                    continue;
                }
                auto intermediate_points = (subdivision_type == SubdivisionType::MAKE_SYMMETRIC)
                    ? calculate_intermediate_points(std::min(r, c), std::max(r, c), row->second)
                    : calculate_intermediate_points(c, r, row->second);
                if ((subdivision_type == SubdivisionType::MAKE_SYMMETRIC) && (r < c)) {
                    std::reverse(intermediate_points.begin(), intermediate_points.end());
                }
                if (!intermediate_points.empty()) {
                    col.erase(row++);
                    size_t old_id = c;
                    auto old_point = points.at(c);
                    for (const auto& [i, pn] : enumerate(intermediate_points)) {
                        auto key = (r < c) || (subdivision_type == SubdivisionType::ASYMMETRIC)
                            ? std::tuple<size_t, size_t, size_t>{r, c, i}
                            : std::tuple<size_t, size_t, size_t>{c, r, intermediate_points.size() - i - 1};
                        auto it = new_point_ids.insert({key, points.size() + new_points.size()});
                        size_t new_id = it.first->second;
                        assert_true(new_columns[old_id].insert({new_id, (TData)std::sqrt(sum(squared(pn - old_point)))}).second);
                        if (it.second) {
                            new_points.push_back(pn);
                        }
                        old_id = new_id;
                        old_point = pn;
                    }
                    assert_true(new_columns[old_id].insert({r, (TData)std::sqrt(sum(squared(points.at(r) - old_point)))}).second);
                } else {
                    ++row;
                }
            }
        }
    }
    points.insert(points.end(), new_points.begin(), new_points.end());
    adjacency.resize(ArrayShape{points.size(), points.size()});
    for (const auto& c : new_columns) {
        for (const auto& r : c.second) {
            adjacency(r.first, c.first) = r.second;
        }
    }
}

template <class TPoint>
PointsAndAdjacency<TPoint> PointsAndAdjacency<TPoint>::concatenated(
    const PointsAndAdjacency& other) const
{
    PointsAndAdjacency<TPoint> result(points.size() + other.points.size());
    std::copy(
        points.begin(),
        points.end(),
        result.points.begin());
    std::copy(
        other.points.begin(),
        other.points.end(),
        result.points.data() + points.size());
    for (auto&& [c, col] : enumerate(adjacency.columns())) {
        auto& result_c = result.adjacency.column(c);
        for (auto&& row : col) {
            result_c[row.first] = row.second;
        }
    }
    for (auto&& [c, col] : enumerate(other.adjacency.columns())) {
        auto& result_c = result.adjacency.column(c + points.size());
        for (auto&& row : col) {
            result_c[row.first + points.size()] = row.second;
        }
    }
    return result;
}

template <class TPoint>
void PointsAndAdjacency<TPoint>::insert(const PointsAndAdjacency& other)
{
    *this = concatenated(other);
}

template <class TPoint>
template <class TCombinePoints>
PointsAndAdjacency<TPoint> PointsAndAdjacency<TPoint>::merged_neighbors(
    const TData& merge_radius,
    const TData& error_radius,
    const TCombinePoints& combine_points) const
{
    std::vector<size_t> new_ids(points.size());
    UVector<TPoint> result_points;
    result_points.reserve(points.size());
    {
        FuzzySetOfPoints<TData, tlength> point_bvh{ merge_radius, error_radius };
        for (const auto& [i, p] : enumerate(points)) {
            size_t neighbor_id;
            if (point_bvh.insert(p, neighbor_id)) {
                result_points.push_back(p);
            } else {
                combine_points(result_points[neighbor_id], p);
            }
            new_ids[i] = neighbor_id;
        }
    }
    // {
    //     auto info = linfo();
    //     point_bvh.optimize_search_time(info);
    // }
    PointsAndAdjacency<TPoint> result(result_points.size());
    result.points = result_points;
    result_points.clear();
    for (const auto& [c, col] : enumerate(adjacency.columns())) {
        auto& new_column = result.adjacency.column(new_ids[c]);
        for (const auto& [r, value] : col) {
            new_column.try_emplace(new_ids[r], value);
        }
    }
    result.update_adjacency_diagonal();
    return result;
}

template <class TPoint>
template <class TCombinePoints>
void PointsAndAdjacency<TPoint>::merge_neighbors(
    const TData& merge_radius,
    const TData& error_radius,
    const TCombinePoints& combine_points)
{
    *this = merged_neighbors(merge_radius, error_radius, combine_points);
}

template <class TPoint>
template <class TSize>
void PointsAndAdjacency<TPoint>::plot(Svg<TSize>& svg, float line_width) const {
    static_assert(tlength >= 2);
    std::vector<TData> x_start;
    std::vector<TData> y_start;
    std::vector<TData> x_stop;
    std::vector<TData> y_stop;
    for (size_t c = 0; c < adjacency.shape(1); ++c) {
        for (const auto& r : adjacency.column(c)) {
            if (r.first != c) {
                x_start.push_back(points.at(c)(0));
                y_start.push_back(points.at(c)(1));
                x_stop.push_back(points.at(r.first)(0));
                y_stop.push_back(points.at(r.first)(1));
            }
        }
    }
    svg.plot_edges(x_start, y_start, x_stop, y_stop, line_width);
}

template <class TPoint>
void PointsAndAdjacency<TPoint>::plot(const std::string& filename, float width, float height, float line_width) const {
    std::ofstream ofstr{ filename };
    Svg<float> svg{ ofstr, width, height };
    plot(svg, line_width);
    svg.finish();
    ofstr.flush();
    if (ofstr.fail()) {
        THROW_OR_ABORT("Could not save to file " + filename);
    }
}

}
