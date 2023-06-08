#pragma once
#include "Points_And_Adjacency.hpp"
#include <Mlib/Assert.hpp>
#include <Mlib/Images/Svg.hpp>
#include <Mlib/Iterator/Enumerate.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

namespace Mlib {

template <class TData, size_t tndim>
PointsAndAdjacency<TData, tndim>::PointsAndAdjacency(size_t npoints)
: points(npoints),
  adjacency(npoints, npoints)
{}

template <class TData, size_t tndim>
void PointsAndAdjacency<TData, tndim>::update_adjacency() {
    for (auto&& [c, col] : enumerate(adjacency.columns())) {
        for (auto& row : col) {
            row.second = std::sqrt(sum(squared(points.at(c) - points.at(row.first))));
        }
    }
}

template <class TData, size_t tndim>
void PointsAndAdjacency<TData, tndim>::transform(const TransformationMatrix<float, double, 3>& m) {
    adjacency = adjacency * (double)m.get_scale();
    for (FixedArray<double, 3>& p : points) {
        p = m.transform(p);
    }
}

template <class TData, size_t tndim>
template <class TCalculateIntermediatePoints>
void PointsAndAdjacency<TData, tndim>::subdivide(
    const TCalculateIntermediatePoints& calculate_intermediate_points,
    SubdivisionType subdivision_type)
{
    std::map<std::tuple<size_t, size_t, size_t>, size_t> new_point_ids;
    std::list<FixedArray<TData, tndim>> new_points;
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
                    ? calculate_intermediate_points(points.at(std::min(r, c)), points.at(std::max(r, c)), row->second)
                    : calculate_intermediate_points(points.at(c), points.at(r), row->second);
                if ((subdivision_type == SubdivisionType::MAKE_SYMMETRIC) && (r < c)) {
                    std::reverse(intermediate_points.begin(), intermediate_points.end());
                }
                if (!intermediate_points.empty()) {
                    col.erase(row++);
                    size_t old_id = c;
                    FixedArray<TData, tndim> old_point = points.at(c);
                    for (size_t i = 0; i < intermediate_points.size(); ++i) {
                        FixedArray<TData, tndim> pn = intermediate_points[i];
                        auto key = (r < c) || (subdivision_type == SubdivisionType::ASYMMETRIC)
                            ? std::tuple<size_t, size_t, size_t>{r, c, i}
                            : std::tuple<size_t, size_t, size_t>{c, r, intermediate_points.size() - i - 1};
                        auto it = new_point_ids.insert({key, points.size() + new_points.size()});
                        size_t new_id = it.first->second;
                        assert_true(new_columns[old_id].insert({new_id, std::sqrt(sum(squared(pn - old_point)))}).second);
                        if (it.second) {
                            new_points.push_back(pn);
                        }
                        old_id = new_id;
                        old_point = pn;
                    }
                    assert_true(new_columns[old_id].insert({r, std::sqrt(sum(squared(points.at(r) - old_point)))}).second);
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

template <class TData, size_t tndim>
PointsAndAdjacency<TData, tndim> PointsAndAdjacency<TData, tndim>::concatenated(
    const PointsAndAdjacency& other) const
{
    PointsAndAdjacency<TData, tndim> result{points.size() + other.points.size()};
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

template <class TData, size_t tndim>
void PointsAndAdjacency<TData, tndim>::insert(const PointsAndAdjacency& other)
{
    *this = concatenated(other);
}

template <class TData, size_t tndim>
void PointsAndAdjacency<TData, tndim>::merge_neighbors(TData radius) {
    THROW_OR_ABORT("PointsAndAdjacency<TData, tndim>::merge_neighbors not yet implemented");
}

template <class TData, size_t tndim>
template <class TSize>
void PointsAndAdjacency<TData, tndim>::plot(Svg<TSize>& svg, float line_width) const {
    static_assert(tndim == 3);
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

template <class TData, size_t tndim>
void PointsAndAdjacency<TData, tndim>::plot(const std::string& filename, float width, float height, float line_width) const {
    std::ofstream ofstr{filename};
    Svg<float> svg{ofstr, width, height};
    plot(svg, line_width);
    svg.finish();
    ofstr.flush();
    if (ofstr.fail()) {
        THROW_OR_ABORT("Could not save to file " + filename);
    }
}

}
