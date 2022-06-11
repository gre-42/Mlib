#include "Resource_Name_Cycle.hpp"
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene_Graph/Parsed_Resource_Name.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>
#include <Mlib/Strings/From_Number.hpp>

using namespace Mlib;

ResourceNameCycle::ResourceNameCycle(
    const SceneNodeResources& resources,
    const std::vector<ParsedResourceName>& names)
: ResourceCycle{ names },
  probability_{ 1234321 }
{}

ResourceNameCycle::~ResourceNameCycle()
{}

const ParsedResourceName* ResourceNameCycle::try_once(const LocationInformation& location_info) {
    ResourceCycle& rc = *this;
    return rc.try_once([this, &location_info](const ParsedResourceName& prn){return predicate(prn, location_info);});
}

const ParsedResourceName& ResourceNameCycle::operator () (const LocationInformation& location_info) {
    ResourceCycle& rc = *this;
    return rc([this, &location_info](const ParsedResourceName& prn){return predicate(prn, location_info);});
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
        if (probability_() > prn.probability) {
            return false;
        }
    }
    return true;
}

const ParsedResourceName* ResourceNameCycle::optional(const LocationInformation& location_info) {
    const ParsedResourceName& res = (*this)(location_info);
    if (res.probability1 != 1) {
        if (probability_() > res.probability1) {
            return nullptr;
        }
    }
    return &res;
}

void ResourceNameCycle::seed(unsigned int seed) {
    ResourceCycle::seed(seed);
    probability_.seed(seed);
}
