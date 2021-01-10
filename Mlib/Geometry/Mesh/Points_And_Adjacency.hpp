#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Array/Sparse_Array.hpp>
#include <Mlib/Images/Svg.hpp>

namespace Mlib {

template <class TData, size_t tndim>
struct PointsAndAdjacency {
    std::vector<FixedArray<TData, tndim>> points;
    SparseArrayCcs<TData> adjacency;

    template <class TSize>
    void plot(Svg<TSize>& svg, float line_width = 1.5) const {
        static_assert(tndim == 2);
        std::vector<TData> x_start;
        std::vector<TData> y_start;
        std::vector<TData> x_stop;
        std::vector<TData> y_stop;
        for (size_t c = 0; c < adjacency.shape(1); ++c) {
            for (const auto& r : adjacency.column(c)) {
                x_start.push_back(points.at(r.first)(0));
                y_start.push_back(points.at(r.first)(1));
                x_stop.push_back(points.at(c)(0));
                y_stop.push_back(points.at(c)(1));
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
