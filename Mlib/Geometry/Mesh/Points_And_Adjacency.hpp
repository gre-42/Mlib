#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Array/Sparse_Array.hpp>
#include <Mlib/Images/Svg.hpp>

namespace Mlib {

template <class TData, size_t tndim>
struct PointsAndAdjacency {
    std::vector<FixedArray<TData, tndim>> points;
    SparseArrayCcs<TData> adjacency;

    void subdivide(const TData& max_length) {
        std::list<FixedArray<TData, tndim>> new_points;
        std::map<size_t, std::map<size_t, TData>> new_columns;
        {
            size_t c = 0;
            for (std::map<size_t, TData>& col : adjacency.columns()) {
                for (typename std::map<size_t, TData>::iterator row = col.begin(); row != col.end();) {
                    size_t npoints = 1 + (size_t)(row->second / max_length);
                    if (npoints > 2) {
                        size_t r = row->first;
                        col.erase(row++);
                        Linspace<float> ls{0, 1, npoints};
                        size_t old_id = c;
                        FixedArray<TData, tndim> old_point = points.at(c);
                        for (size_t i = 1; i < ls.length(); ++i) {
                            float alpha = ls[i];
                            FixedArray<TData, tndim> pn = points.at(c) * (1 - alpha) + points.at(r) * alpha;
                            size_t new_id = points.size() + new_points.size();
                            new_columns[old_id].insert({new_id, std::sqrt(sum(squared(pn - old_point)))});
                            new_points.push_back(pn);
                            old_id = new_id;
                        }
                        new_columns[old_id].insert({r, std::sqrt(sum(squared(points.at(r) - old_point)))});
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

    template <class TSize>
    void plot(Svg<TSize>& svg, float line_width = 1.5) const {
        static_assert(tndim == 2);
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

    void plot(const std::string& filename, float width, float height, float line_width = 1.5) const {
        std::ofstream ofstr{filename};
        Svg<float> svg{ofstr, width, height};
        plot(svg, line_width);
        svg.finish();
        ofstr.flush();
        if (ofstr.fail()) {
            throw std::runtime_error("Could not save to file " + filename);
        }
    }
};

}
