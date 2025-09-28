#include "Particle_Descriptor.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Geometry/Material/Particle_Type.hpp>
#include <Mlib/Json/Json_View.hpp>
#include <Mlib/Physics/Units.hpp>
#include <map>

namespace ParticleDescriptorArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(resource);
DECLARE_ARGUMENT(air_resistance_halflife);
DECLARE_ARGUMENT(animation_duration);
DECLARE_ARGUMENT(rotation);
DECLARE_ARGUMENT(type);
}

using namespace Mlib;

ParticleRotation Mlib::particle_rotation_from_string(const std::string& s) {
    static const std::map<std::string, ParticleRotation> m{
        {"emitter", ParticleRotation::EMITTER},
        {"random_yangle", ParticleRotation::RANDOM_YANGLE},
    };
    auto it = m.find(s);
    if (it == m.end()) {
        THROW_OR_ABORT("Unknown particle rotation: \"" + s + '"');
    }
    return it->second;
}

void Mlib::from_json(const nlohmann::json& j, ParticleDescriptor& item) {
    JsonView jv{ j };
    jv.validate(ParticleDescriptorArgs::options);

    item.resource_name = jv.at<std::string>(ParticleDescriptorArgs::resource);
    item.air_resistance_halflife = jv.at<float>(ParticleDescriptorArgs::air_resistance_halflife) * seconds;
    item.animation_duration = jv.at<float>(ParticleDescriptorArgs::animation_duration, NAN) * seconds;
    item.rotation = particle_rotation_from_string(jv.at<std::string>(ParticleDescriptorArgs::rotation, "emitter"));
    item.type = particle_type_from_string(jv.at<std::string>(ParticleDescriptorArgs::type));
}
