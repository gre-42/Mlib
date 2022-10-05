#pragma once
#include <Mlib/Osm_Loader/Osm_Map_Resource/Resource_Cycle.hpp>
#include <vector>

namespace Mlib {

struct ParsedResourceName;

struct LocationInformation {
    float distance_to_boundary = NAN;
};

class ResourceNameCycle: public ResourceCycle<ParsedResourceName> {
public:
    explicit ResourceNameCycle(const std::vector<ParsedResourceName>& names);
    ~ResourceNameCycle();
    const ParsedResourceName* try_once(const LocationInformation& location_info = LocationInformation{});
    const ParsedResourceName* try_multiple_times(size_t nattempts, const LocationInformation& location_info = LocationInformation{});
    const ParsedResourceName& operator () (const LocationInformation& location_info = LocationInformation{});
    void seed(unsigned int seed);
private:
    bool predicate(
        const ParsedResourceName& prn,
        const LocationInformation& location_info);
    UniformRandomNumberGenerator<float> probability_;
};

}
