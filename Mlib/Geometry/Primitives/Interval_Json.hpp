#pragma once
#include "Interval.hpp"
#include <Mlib/Json/Json_View.hpp>
#include <Mlib/Misc/Argument_List.hpp>
#include <stdexcept>
#include <vector>

namespace Mlib {

namespace IntervalArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(min);
DECLARE_ARGUMENT(max);
}

template <class T>
void from_json(const nlohmann::json& j, Interval<T>& i) {
    if (j.is_array()) {
        auto vd = j.get<std::vector<T>>();
        if (vd.size() != 2) {
            throw std::runtime_error("Interval array does not have length 2");
        }
        i = { vd[0], vd[1] };
    } else {
        JsonView jv{ j };
        jv.validate(IntervalArgs::options);
        i.min = jv.at<T>(IntervalArgs::min);
        i.max = jv.at<T>(IntervalArgs::max);
    }
}

}
