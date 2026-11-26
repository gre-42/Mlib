#include "Subdivided_Contour.hpp"
#include <Mlib/Initialization/Default_Uninitialized_Vector.hpp>
#include <Mlib/Iterator/Iterable.hpp>
#include <Mlib/Math/Interp.hpp>
#include <Mlib/Math/Math.hpp>
#include <Mlib/Memory/Float_To_Integral.hpp>
#include <Mlib/Stats/Linspace.hpp>

using namespace Mlib;

template <size_t tndim>
UUInterp<double, FixedArray<double, tndim>> Mlib::interpolated_contour(
    const std::list<FixedArray<CompressedScenePos, tndim>>& contour,
    ContourClosedness contour_closedness)
{
    if (contour.empty()) {
        return UUInterp<double, FixedArray<double, tndim>>({}, {});
    }
    switch (contour_closedness) {
        case ContourClosedness::CLOSED_WO_LAST_ELEMENT:
        {
            auto extended = contour;
            extended.push_back(contour.front());
            return interpolated_contour(extended, ContourClosedness::OPEN_OR_CLOSED_WITH_LAST_ELEMENT);
        }
        case ContourClosedness::OPEN_OR_CLOSED_WITH_LAST_ELEMENT:
        {
            std::vector<double> distance_to_origin;
            distance_to_origin.reserve(contour.size());
            distance_to_origin.push_back(0.);

            UUVector<FixedArray<double, tndim>> positions;
            positions.reserve(contour.size());
            positions.emplace_back(funpack(contour.front()));
            {
                for (const auto& current_position : Iterable{ ++contour.begin(), contour.end() }) {
                    distance_to_origin.push_back(
                        distance_to_origin.back() +
                        std::sqrt(sum(squared(funpack(current_position) - positions.back()))));
                    positions.emplace_back(funpack(current_position));
                }
            }
            return Interp<double, DefaultUnitialized<FixedArray<double, tndim>>>{ distance_to_origin, positions };
        }
    }
    throw std::runtime_error("Unknown contour closedness");
}

template <size_t tndim>
std::list<FixedArray<CompressedScenePos, tndim>> Mlib::subdivided_contour(
    const std::list<FixedArray<CompressedScenePos, tndim>>& contour,
    double scale,
    CompressedScenePos distance,
    ContourClosedness contour_closedness)
{
    std::list<FixedArray<CompressedScenePos, tndim>> result;
    auto interp = interpolated_contour(contour, contour_closedness);
    auto range = Linspace{
        0.,
        interp.xmax(),
        float_to_integral<size_t>(std::round(1. + interp.xmax() / (funpack(distance) * scale))),
        size_t(contour_closedness == ContourClosedness::CLOSED_WO_LAST_ELEMENT)};
    for (auto dist : range) {
        result.push_back(interp(dist).template casted<CompressedScenePos>());
    }
    return result;
}

template std::list<FixedArray<CompressedScenePos, 2>> Mlib::subdivided_contour(
    const std::list<FixedArray<CompressedScenePos, 2>>& contour,
    double scale,
    CompressedScenePos distance,
    ContourClosedness contour_closedness);

template std::list<FixedArray<CompressedScenePos, 3>> Mlib::subdivided_contour(
    const std::list<FixedArray<CompressedScenePos, 3>>& contour,
    double scale,
    CompressedScenePos distance,
    ContourClosedness contour_closedness);
