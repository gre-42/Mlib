#include "Wayside_Distances.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Json/Misc.hpp>

using namespace Mlib;

namespace Fields {
    BEGIN_ARGUMENT_LIST;
    DECLARE_ARGUMENT(tangential_distance);
    DECLARE_ARGUMENT(normal_distance);
    DECLARE_ARGUMENT(gradient_dx);
    DECLARE_ARGUMENT(max_gradient);
    DECLARE_ARGUMENT(yangle);
}

void Mlib::from_json(const nlohmann::json& j, WaysideDistances& item) {
    Mlib::validate(j, Fields::options);
    j.at(Fields::tangential_distance).get_to(item.tangential_distance);
    j.at(Fields::normal_distance).get_to(item.normal_distance);
    if (j.contains(Fields::gradient_dx)) {
        j.at(Fields::gradient_dx).get_to(item.gradient_dx);
    } else {
        item.gradient_dx = NAN;
    }
    if (j.contains(Fields::max_gradient)) {
        j.at(Fields::max_gradient).get_to(item.max_gradient);
    } else {
        item.max_gradient = NAN;
    }
}
