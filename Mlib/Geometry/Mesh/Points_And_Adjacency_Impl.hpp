#pragma once
#include "Points_And_Adjacency.hpp"
#include <Mlib/Images/Svg.hpp>
#include <Mlib/Math/Transformation_Matrix.hpp>

namespace Mlib {

template <class TData, size_t tndim>
void PointsAndAdjacency<TData, tndim>::update_adjacency() {
    size_t c = 0;
    for (std::map<size_t, TData>& col : adjacency.columns()) {
        for (auto& row : col) {
            row.second = std::sqrt(sum(squared(points.at(c) - points.at(row.first))));
        }
        ++c;
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
        size_t c = 0;
        for (std::map<size_t, TData>& col : adjacency.columns()) {
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
            ++c;
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
        throw std::runtime_error("Could not save to file " + filename);
    }
}

}
