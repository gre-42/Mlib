#pragma once
#include <Mlib/Json/Base.hpp>
#include <Mlib/Math/Orderable_Fixed_Array.hpp>

namespace Mlib {

template <class TData, size_t tsize>
void to_json(nlohmann::json& j, const FixedArray<TData, tsize>& v) {
    j = std::vector(v.flat_begin(), v.flat_end());
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

template <class TData, size_t tsize>
void from_json(const nlohmann::json& j, FixedArray<TData, tsize>& v) {
    if (j.type() != nlohmann::detail::value_t::array) {
        THROW_OR_ABORT("JSON -> FixedArray received non-array type");
    }
    if (j.size() != tsize) {
        THROW_OR_ABORT("JSON -> FixedArray received array of incorrect length");
    }
    for (size_t i = 0; i < tsize; ++i) {
        v(i) = json_get<TData>(j[i]);
    }
}

template <class TData, size_t tsize>
void from_json(const nlohmann::json& j, OrderableFixedArray<TData, tsize>& v) {
    if (j.type() != nlohmann::detail::value_t::array) {
        THROW_OR_ABORT("JSON -> OrderableFixedArray received non-array type");
    }
    if (j.size() != tsize) {
        THROW_OR_ABORT("JSON -> OrderableFixedArray received array of incorrect length");
    }
    for (size_t i = 0; i < tsize; ++i) {
        v(i) = json_get<TData>(j[i]);
    }
}

template <class TData, class TOperation>
auto get_vector(const nlohmann::json& j, const TOperation& op) {
    if (j.type() != nlohmann::detail::value_t::array) {
        THROW_OR_ABORT("JSON -> vector received non-array type");
    }
    std::vector<decltype(op(j.get<TData>()))> result;
    result.reserve(j.size());
    for (auto& e : j) {
        result.push_back(op(json_get<TData>(e)));
    }
    return result;
}

template <class TData, class TOperation>
auto get_vector_non_null(const nlohmann::json& j, const TOperation& op) {
    if (j.type() == nlohmann::detail::value_t::null) {
        return decltype(get_vector<TData>(j, op))();
    }
    return get_vector<TData>(j, op);
}

std::string get_multiline_string(const nlohmann::json& j);
void validate(const nlohmann::json& j, const std::set<std::string>& known_keys, const std::string& prefix = "");

}
