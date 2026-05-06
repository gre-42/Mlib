#include "Dynamic_Light_Db.hpp"
#include <Mlib/Physics/Dynamic_Lights/Animated_Dynamic_Light.hpp>
#include <Mlib/Physics/Dynamic_Lights/Constant_Dynamic_Light.hpp>

using namespace Mlib;

DynamicLightDb::DynamicLightDb()
    : configurations_{ "Light configuration" }
{}

DynamicLightDb::~DynamicLightDb() = default;

void DynamicLightDb::add(const std::string& name, const TLightConfiguration& config) {
    configurations_.add(name, config);
}

const DynamicLightDb::TLightConfiguration& DynamicLightDb::get(const std::string& name) const {
    return configurations_.get(name);
}
