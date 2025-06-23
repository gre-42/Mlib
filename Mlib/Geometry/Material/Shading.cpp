#include "Shading.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Json/Json_View.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(emissive);
DECLARE_ARGUMENT(ambient);
DECLARE_ARGUMENT(diffuse);
DECLARE_ARGUMENT(specular);
DECLARE_ARGUMENT(specular_exponent);
DECLARE_ARGUMENT(reflectance);
DECLARE_ARGUMENT(fresnel);
DECLARE_ARGUMENT(fog_distances);
DECLARE_ARGUMENT(fog_ambient);
}

void Mlib::from_json(const nlohmann::json& j, Shading& shading) {
    JsonView jv{ j };
    jv.validate(KnownArgs::options);
    shading.emissive = jv.at<EFixedArray<float, 3>>(KnownArgs::emissive, { 0.f, 0.f, 0.f });
    shading.ambient = jv.at<EFixedArray<float, 3>>(KnownArgs::ambient, { 1.f, 1.f, 1.f });
    shading.diffuse = jv.at<EFixedArray<float, 3>>(KnownArgs::diffuse, { 0.8f, 0.8f, 0.8f });
    shading.specular = jv.at<EFixedArray<float, 3>>(KnownArgs::specular, { 0.5f, 0.5f, 0.5f });
    shading.specular_exponent = jv.at<float>(KnownArgs::specular_exponent, 4.f);
    shading.reflectance = jv.at<EFixedArray<float, 3>>(KnownArgs::reflectance, { 0.f, 0.f, 0.f });
    shading.fresnel = jv.at<FresnelAndAmbient>(KnownArgs::fresnel, FresnelAndAmbient{});
    shading.fog_distances = jv.at<EFixedArray<float, 2>>(KnownArgs::fog_distances, default_step_distances);
    shading.fog_ambient = jv.at<EFixedArray<float, 3>>(KnownArgs::fog_ambient, { -1.f, -1.f, -1.f });
}
