#include "Subdivided_Contour.hpp"
#include <Mlib/Iterator/Iterable.hpp>
#include <Mlib/Math/Interp.hpp>

using namespace Mlib;

std::list<FixedArray<double, 3>> Mlib::subdivided_contour(
    const std::list<FixedArray<double, 3>>& contour,
    double scale,
    double distance)
{
    std::list<FixedArray<double, 3>> result;
    if (!contour.empty()) {
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
        Interp<double, FixedArray<double, 3>> interp{ distance_to_origin, positions };
        for (auto dist = 0.; dist <= distance_to_origin.back(); dist += distance * scale) {
            result.push_back(interp(dist));
        }
    }
    return result;
}
