#include "Particle_Descriptor.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Json/Json_View.hpp>
#include <Mlib/Physics/Units.hpp>

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(resource);
DECLARE_ARGUMENT(air_resistance);
DECLARE_ARGUMENT(animation_duration);
}

using namespace Mlib;

void Mlib::from_json(const nlohmann::json& j, ParticleDescriptor& item) {
    JsonView jv{ j };
    jv.validate(KnownArgs::options);

    item.resource_name = jv.at<std::string>(KnownArgs::resource, "");
    item.air_resistance = jv.at<float>(KnownArgs::air_resistance);
    item.animation_duration = jv.at<float>(KnownArgs::animation_duration, NAN) * seconds;
}
