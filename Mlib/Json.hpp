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
void from_json(const nlohmann::json& j, FixedArray<TData, tsize>& v) {
    if (j.type() != nlohmann::detail::value_t::array) {
        THROW_OR_ABORT("JSON -> FixedArray received non-array type");
    }
    if (j.size() != tsize) {
        THROW_OR_ABORT("JSON -> FixedArray received array of incorrect length");
    }
    for (size_t i = 0; i < tsize; ++i) {
        v(i) = j[i].get<TData>();
    }
}

template <class T>
T json_get(const nlohmann::json& j) {
    if constexpr (std::is_floating_point_v<T>) {
        if (j.type() == nlohmann::detail::value_t::string) {
            auto v = j.get<std::string>();
            if (v == "nan") {
                return NAN;
            } else if (v == "inf") {
                return INFINITY;
            } else if (v == "-inf") {
                return -INFINITY;
            } else {
                THROW_OR_ABORT("Unknown number string: \"" + v + '"');
            }
        }
    }
    return j.get<T>();
}

template <class TData, class TOperation>
auto get_vector(const nlohmann::json& j, const TOperation& op) {
    std::vector<decltype(op(j.get<TData>()))> result;
    result.reserve(j.size());
    for (auto& e : j) {
        result.push_back(op(json_get<TData>(e)));
    }
    return result;
}

std::string get_multiline_string(const nlohmann::json& j);

}
