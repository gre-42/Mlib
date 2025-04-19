#include "Fresnel.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Json/Json_View.hpp>
#include <Mlib/Math/Fixed_Math.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(min);
DECLARE_ARGUMENT(max);
DECLARE_ARGUMENT(exponent);
DECLARE_ARGUMENT(ambient);
}

void Mlib::from_json(const nlohmann::json& j, FresnelAndAmbient& f) {
    JsonView jv{ j };
    jv.validate(KnownArgs::options);
    f.reflectance.min = jv.at<float>(KnownArgs::min);
    f.reflectance.max = jv.at<float>(KnownArgs::max);
    f.reflectance.exponent = jv.at<float>(KnownArgs::exponent);
    f.ambient = jv.at<UFixedArray<float, 3>>(KnownArgs::ambient, fixed_zeros<float, 3>());
}
