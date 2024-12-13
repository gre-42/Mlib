#include "Subdivided_Contour.hpp"
#include <Mlib/Default_Uninitialized_Vector.hpp>
#include <Mlib/Iterator/Iterable.hpp>
#include <Mlib/Math/Interp.hpp>
#include <Mlib/Math/Math.hpp>

using namespace Mlib;

UUInterp<double, FixedArray<double, 3>> Mlib::interpolated_contour(
    const std::list<FixedArray<CompressedScenePos, 3>>& contour)
{
    if (contour.empty()) {
        return UUInterp<double, FixedArray<double, 3>>({}, {});
    }
    std::vector<double> distance_to_origin;
    distance_to_origin.reserve(contour.size());
    distance_to_origin.push_back(0.);

    UUVector<FixedArray<double, 3>> positions;
    positions.reserve(contour.size());
    positions.push_back(funpack(contour.front()));
    {
        for (const auto& current_position : Iterable{ ++contour.begin(), contour.end() }) {
            distance_to_origin.push_back(
                distance_to_origin.back() +
                std::sqrt(sum(squared(funpack(current_position) - positions.back()))));
            positions.push_back(funpack(current_position));
        }
    }
    return Interp<double, DefaultUnitialized<FixedArray<double, 3>>>{ distance_to_origin, positions };
}

std::list<FixedArray<CompressedScenePos, 3>> Mlib::subdivided_contour(
    const std::list<FixedArray<CompressedScenePos, 3>>& contour,
    double scale,
    CompressedScenePos distance)
{
    std::list<FixedArray<CompressedScenePos, 3>> result;
    auto interp = interpolated_contour(contour);
    for (auto dist = 0.; dist <= interp.xmax(); dist += funpack(distance) * scale) {
        result.push_back(interp(dist).casted<CompressedScenePos>());
    }
    return result;
}
