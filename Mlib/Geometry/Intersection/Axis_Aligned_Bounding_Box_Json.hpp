#pragma once
#include <Mlib/Argument_List.hpp>
#include <Mlib/Geometry/Intersection/Axis_Aligned_Bounding_Box.hpp>
#include <Mlib/Json/Json_View.hpp>

namespace Mlib {

namespace AabbArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(min);
DECLARE_ARGUMENT(max);
}

template <class TData, size_t tndim>
void from_json(const nlohmann::json& j, DefaultUnitialized<AxisAlignedBoundingBox<TData, tndim>>& aabb) {
    JsonView jv{ j };
    jv.validate(AabbArgs::options);
    aabb.base().min = jv.at<EFixedArray<TData, tndim>>(AabbArgs::min);
    aabb.base().max = jv.at<EFixedArray<TData, tndim>>(AabbArgs::max);
}

}
