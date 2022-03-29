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

const ParsedResourceName* ResourceNameCycle::try_once() {
    ResourceCycle& rc = *this;
    return rc.try_once([this](const ParsedResourceName& prn){return predicate(prn);});
}

const ParsedResourceName& ResourceNameCycle::operator () () {
    ResourceCycle& rc = *this;
    return rc([this](const ParsedResourceName& prn){return predicate(prn);});
}

bool ResourceNameCycle::predicate(const ParsedResourceName& prn) {
    return (prn.probability == 1) || (probability_() < prn.probability);
}

void ResourceNameCycle::seed(unsigned int seed) {
    ResourceCycle::seed(seed);
    probability_.seed(seed);
}
