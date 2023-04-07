#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Features.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <vector>

#ifdef WITHOUT_EXCEPTIONS
#include <Mlib/Os/Os.hpp>
#define JSON_TRY_USER if(true)
#define JSON_CATCH_USER(exception) if(false)
#define JSON_THROW_USER(exception)                           \
    {::Mlib::verbose_abort((exception).what());}
#endif

#include <nlohmann/json.hpp>

namespace Mlib {

template <class TData, size_t tsize>
FixedArray<TData, tsize> get_fixed_array(const nlohmann::json& j) {
    auto v = j.get<std::vector<TData>>();
    if (v.size() != tsize) {
        THROW_OR_ABORT("Unsupported dimensionality");
    }
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
    return FixedArray<TData, tsize>{v};
#pragma GCC diagnostic pop
}

template <class TData, size_t tsize>
std::vector<FixedArray<TData, tsize>> get_vector_of_arrays(const nlohmann::json& j) {
    std::list<FixedArray<TData, tsize>> vertex_list;
    for (const auto& vertex : j) {
        vertex_list.push_back(get_fixed_array<TData, tsize>(vertex));
    }
    return std::vector<FixedArray<TData, tsize>>{vertex_list.begin(), vertex_list.end()};
}

}
