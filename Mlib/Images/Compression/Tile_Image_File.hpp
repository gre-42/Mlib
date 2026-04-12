#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Misc/FPath.hpp>
#include <Mlib/Os/Utf8_Path.hpp>
#include <cstddef>
#include <cstdint>
#include <nlohmann/json_fwd.hpp>
#include <optional>
#include <vector>

namespace Mlib {

struct OlsCoefficient {
    float offset;
    float slope;
};

struct FragmentAssembly {
    FPath color;
    FPath alpha;
    FPath alpha_fac;
    FixedArray<uint32_t, 2> size = uninitialized;
    float stepsize;
    float randsize;
    uint32_t channels;
    bool add;
    uint32_t upsampling;
    std::optional<std::vector<OlsCoefficient>> ols;
    void make_pathes_absolute(const Utf8Path& filename);
};

void to_json(nlohmann::json& j, const OlsCoefficient& c);
void from_json(const nlohmann::json& j, OlsCoefficient& c);

void to_json(nlohmann::json& j, const FragmentAssembly& fa);
void from_json(const nlohmann::json& j, FragmentAssembly& fa);

void save_fragment_assembly(const Utf8Path& filename, const FragmentAssembly& fa);
FragmentAssembly load_fragment_assembly(const Utf8Path& filename);

}
