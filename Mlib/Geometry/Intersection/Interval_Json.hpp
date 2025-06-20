#pragma once
#include "Interval.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Json/Json_View.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
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
            THROW_OR_ABORT("Interval array does not have length 2");
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
