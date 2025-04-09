#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <nlohmann/json_fwd.hpp>

namespace Mlib {

struct ColorAndProbability {
    FixedArray<float, 3> color = uninitialized;
    float probability;
};

void from_json(const nlohmann::json& j, ColorAndProbability& cp);

}
