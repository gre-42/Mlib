#pragma once
#include <Mlib/Render/Renderables/Resource_Cycle.hpp>
#include <vector>

namespace Mlib {

struct ParsedResourceName;

struct LocationInformation {
    float distance_to_boundary = NAN;
};

class ResourceNameCycle: public ResourceCycle<ParsedResourceName> {
public:
    explicit ResourceNameCycle(std::vector<ParsedResourceName> names);
    ~ResourceNameCycle();
    const ParsedResourceName* try_once(const LocationInformation& location_info = LocationInformation{});
    const ParsedResourceName* try_multiple_times(size_t nattempts, const LocationInformation& location_info = LocationInformation{});
    void seed(unsigned int seed);
private:
    bool predicate(
        const ParsedResourceName& prn,
        const LocationInformation& location_info);
    bool predicate1(const ParsedResourceName& prn);
    FastUniformRandomNumberGenerator<float> probability_;
};

}
