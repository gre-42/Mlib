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

}
