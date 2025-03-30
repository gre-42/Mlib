#include "Subdivided_Contour.hpp"
#include <Mlib/Default_Uninitialized_Vector.hpp>
#include <Mlib/Iterator/Iterable.hpp>
#include <Mlib/Math/Interp.hpp>
#include <Mlib/Math/Math.hpp>

using namespace Mlib;

template <size_t tndim>
UUInterp<double, FixedArray<double, tndim>> Mlib::interpolated_contour(
    const std::list<FixedArray<CompressedScenePos, tndim>>& contour)
{
    if (contour.empty()) {
        return UUInterp<double, FixedArray<double, tndim>>({}, {});
    }
    std::vector<double> distance_to_origin;
    distance_to_origin.reserve(contour.size());
    distance_to_origin.push_back(0.);

    UUVector<FixedArray<double, tndim>> positions;
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
    return Interp<double, DefaultUnitialized<FixedArray<double, tndim>>>{ distance_to_origin, positions };
}

template <size_t tndim>
std::list<FixedArray<CompressedScenePos, tndim>> Mlib::subdivided_contour(
    const std::list<FixedArray<CompressedScenePos, tndim>>& contour,
    double scale,
    CompressedScenePos distance)
{
    std::list<FixedArray<CompressedScenePos, tndim>> result;
    auto interp = interpolated_contour(contour);
    for (auto dist = 0.; dist <= interp.xmax(); dist += funpack(distance) * scale) {
        result.push_back(interp(dist).template casted<CompressedScenePos>());
    }
    return result;
}

template std::list<FixedArray<CompressedScenePos, 2>> Mlib::subdivided_contour(
    const std::list<FixedArray<CompressedScenePos, 2>>& contour,
    double scale,
    CompressedScenePos distance);

template std::list<FixedArray<CompressedScenePos, 3>> Mlib::subdivided_contour(
    const std::list<FixedArray<CompressedScenePos, 3>>& contour,
    double scale,
    CompressedScenePos distance);
