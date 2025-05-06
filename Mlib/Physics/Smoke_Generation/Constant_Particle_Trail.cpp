#include "Constant_Particle_Trail.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Json/Json_View.hpp>
#include <Mlib/Physics/Units.hpp>

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(particle);
DECLARE_ARGUMENT(generation_dt);
}

using namespace Mlib;

void Mlib::from_json(const nlohmann::json& j, ConstantParticleTrail& item) {
    JsonView jv{ j };
    jv.validate(KnownArgs::options);

    item.particle = jv.at<ParticleDescriptor>(KnownArgs::particle);
    item.generation_dt = jv.at<float>(KnownArgs::generation_dt) * seconds;
}
