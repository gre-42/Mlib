#pragma once
#include <Mlib/Render/Renderables/Resource_Cycle.hpp>

namespace Mlib {

struct ColorAndProbability;

struct BuildingInformation {};

class ColorCycle: public ResourceCycle<ColorAndProbability> {
public:
    explicit ColorCycle(std::vector<ColorAndProbability> names);
    ~ColorCycle();
    const ColorAndProbability* try_once(const BuildingInformation& info);
    const ColorAndProbability& try_multiple_times(size_t nattempts, const BuildingInformation& info);
    void seed(unsigned int seed);
private:
    bool predicate(const ColorAndProbability& prn, const BuildingInformation& info);
    FastUniformRandomNumberGenerator<float> probability_;
};

}
