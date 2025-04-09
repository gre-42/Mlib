#include "Color_Cycle.hpp"
#include <Mlib/Render/Renderables/Color_And_Probability.hpp>

using namespace Mlib;

ColorCycle::ColorCycle(std::vector<ColorAndProbability> names)
: ResourceCycle{ std::move(names) },
  probability_{ 1234321 }
{}

ColorCycle::~ColorCycle()
{}

const ColorAndProbability* ColorCycle::try_once(const BuildingInformation& info) {
    ResourceCycle& rc = *this;
    return rc.try_once([&](const ColorAndProbability& prn){return predicate(prn, info);});
}

const ColorAndProbability& ColorCycle::try_multiple_times(
    size_t nattempts,
    const BuildingInformation& info)
{
    ResourceCycle& rc = *this;
    auto res = rc.try_multiple_times(
        [&](const ColorAndProbability& prn){return predicate(prn, info);},
        nattempts);
    if (res == nullptr) {
        THROW_OR_ABORT("Could not generate color after " + std::to_string(nattempts) + " attempts");
    }
    return *res;
}

bool ColorCycle::predicate(
    const ColorAndProbability& prn,
    const BuildingInformation& info)
{
    if (prn.probability != 1) {
        if (probability_() >= prn.probability) {
            return false;
        }
    }
    return true;
}

void ColorCycle::seed(unsigned int seed) {
    ResourceCycle::seed(seed);
    probability_.seed(seed + 493845);
}
