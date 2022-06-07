#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <nlohmann/json.hpp>
#include <vector>

namespace Mlib {

template <class TData, size_t tsize>
FixedArray<TData, tsize> get_fixed_array(const nlohmann::json& j) {
    auto v = j.get<std::vector<TData>>();
    if (v.size() != tsize) {
        throw std::runtime_error("Unsupported dimensionality");
    }
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
    return FixedArray<TData, tsize>{v};
#pragma GCC diagnostic pop
}

template <class TData, size_t tsize>
std::vector<FixedArray<TData, tsize>> load_vector(const nlohmann::json& j) {
    std::list<FixedArray<TData, tsize>> vertex_list;
    for (const auto& vertex : j) {
        vertex_list.push_back(get_fixed_array<TData, tsize>(vertex));
    }
    return std::vector<FixedArray<TData, tsize>>{vertex_list.begin(), vertex_list.end()};
}

}
