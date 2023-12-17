#include "Subdivided_Contour.hpp"
#include <Mlib/Iterator/Iterable.hpp>
#include <Mlib/Math/Interp.hpp>
#include <Mlib/Math/Math.hpp>

using namespace Mlib;

Interp<double, FixedArray<double, 3>> Mlib::interpolated_contour(
    const std::list<FixedArray<double, 3>>& contour)
{
    if (contour.empty()) {
        return Interp<double, FixedArray<double, 3>>({}, {});
    }
    std::vector<double> distance_to_origin;
    distance_to_origin.reserve(contour.size());
    distance_to_origin.push_back(0.);

    std::vector<FixedArray<double, 3>> positions;
    positions.reserve(contour.size());
    positions.push_back(contour.front());
    {
        for (const auto& current_position : Iterable{ ++contour.begin(), contour.end() }) {
            distance_to_origin.push_back(
                distance_to_origin.back() +
                std::sqrt(sum(squared(current_position - positions.back()))));
            positions.push_back(current_position);
        }
    }
    return Interp<double, FixedArray<double, 3>>{ distance_to_origin, positions };
}

std::list<FixedArray<double, 3>> Mlib::subdivided_contour(
    const std::list<FixedArray<double, 3>>& contour,
    double scale,
    double distance)
{
    std::list<FixedArray<double, 3>> result;
    auto interp = interpolated_contour(contour);
    for (auto dist = 0.; dist <= interp.xmax(); dist += distance * scale) {
        result.push_back(interp(dist));
    }
    return result;
}
