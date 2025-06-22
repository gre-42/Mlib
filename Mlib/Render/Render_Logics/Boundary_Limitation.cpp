#include "Boundary_Limitation.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Json/Json_View.hpp>

using namespace Mlib;

namespace BoundaryLimitationArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(max);
DECLARE_ARGUMENT(falloff);
}

void Mlib::from_json(const nlohmann::json& j, BoundaryLimitation& l) {
    JsonView jv{ j };
    jv.validate(BoundaryLimitationArgs::options);
    l.max = jv.at<float>(BoundaryLimitationArgs::max);
    l.falloff = jv.at<float>(BoundaryLimitationArgs::falloff);
}
