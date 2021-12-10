#pragma once
#include <Mlib/Render/Resources/Osm_Map_Resource/Resource_Cycle.hpp>
#include <vector>

namespace Mlib {

class SceneNodeResources;
struct ParsedResourceName;

class ResourceNameCycle: public ResourceCycle<ParsedResourceName> {
public:
    ResourceNameCycle(
        const SceneNodeResources& resources,
        const std::vector<ParsedResourceName>& names);
    ~ResourceNameCycle();
    const ParsedResourceName* try_once();
    const ParsedResourceName& operator () ();
    void seed(unsigned int seed);
private:
    bool predicate(const ParsedResourceName& prn);
    UniformRandomNumberGenerator<float> probability_;
};

}
