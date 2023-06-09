#include "Resource_Name_Cycle.hpp"
#include <Mlib/Regex/Regex_Select.hpp>
#include <Mlib/Scene_Graph/Resources/Parsed_Resource_Name.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>
#include <Mlib/Strings/To_Number.hpp>

using namespace Mlib;

ResourceNameCycle::ResourceNameCycle(const std::vector<ParsedResourceName>& names)
: ResourceCycle{ names },
  probability_{ 1234321 }
{}

ResourceNameCycle::~ResourceNameCycle()
{}

const ParsedResourceName* ResourceNameCycle::try_once(const LocationInformation& location_info) {
    ResourceCycle& rc = *this;
    auto result = rc.try_once([this, &location_info](const ParsedResourceName& prn){return predicate(prn, location_info);});
    if ((result != nullptr) && !predicate1(*result)) {
        return nullptr;
    }
    return result;
}

const ParsedResourceName* ResourceNameCycle::try_multiple_times(size_t nattempts, const LocationInformation& location_info) {
    ResourceCycle& rc = *this;
    auto result = rc.try_multiple_times(
        [this, &location_info](const ParsedResourceName& prn){return predicate(prn, location_info);},
        nattempts);
    if ((result != nullptr) && !predicate1(*result)) {
        return nullptr;
    }
    return result;
}

bool ResourceNameCycle::predicate(
    const ParsedResourceName& prn,
    const LocationInformation& location_info)
{
    if (!std::isnan(location_info.distance_to_boundary)) {
        if (location_info.distance_to_boundary < prn.min_distance_to_bdry) {
            return false;
        }
        if (location_info.distance_to_boundary > prn.max_distance_to_bdry) {
            return false;
        }
    }
    if (prn.probability != 1) {
        if (probability_() >= prn.probability) {
            return false;
        }
    }
    return true;
}

bool ResourceNameCycle::predicate1(const ParsedResourceName& prn) {
    return (prn.probability1 == 1) || (probability_() < prn.probability1);
}

void ResourceNameCycle::seed(unsigned int seed) {
    ResourceCycle::seed(seed);
    probability_.seed(seed + 493845);
}
