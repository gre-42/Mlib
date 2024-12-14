#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Cached_Hash.hpp>
#include <Mlib/Geometry/Material/Fresnel.hpp>
#include <Mlib/Map/Threadsafe_Default_Map.hpp>
#include <Mlib/Regex/Regex_Select.hpp>
#include <Mlib/Threads/Atomic_Mutex.hpp>
#include <Mlib/Variable_And_Hash.hpp>
#include <optional>
#include <unordered_map>

namespace Mlib {

/**
 * Don't forget to update the "insert" function when adding new fields.
 */
struct ColorStyle {
    std::optional<Mlib::regex> selector;
    OrderableFixedArray<float, 3> emissive{-1.f, -1.f, -1.f};
    OrderableFixedArray<float, 3> ambient{-1.f, -1.f, -1.f};
    OrderableFixedArray<float, 3> diffuse{-1.f, -1.f, -1.f};
    OrderableFixedArray<float, 3> specular{-1.f, -1.f, -1.f};
    float specular_exponent = -1.f;
    OrderableFixedArray<float, 3> fresnel_ambient{-1.f, -1.f, -1.f};
    FresnelReflectance fresnel{
        .min = -1.f,
        .max = -1.f,
        .exponent = -1.f
    };
    std::unordered_map<VariableAndHash<std::string>, VariableAndHash<std::string>> reflection_maps;
    float reflection_strength = 1.f;
    CachedHash hash_;
    mutable AtomicMutex hash_mutex_;
    size_t get_hash() const;
    void insert(const ColorStyle& other);
    bool matches(const VariableAndHash<std::string>& name) const;
    void update_hash();
    ColorStyle& compute_hash();
    mutable ThreadsafeDefaultMap<bool> matches_{ [this](const VariableAndHash<std::string>& name){
        return
            !selector.has_value() ||
            Mlib::re::regex_search(*name, *selector);
    } };
};

}
