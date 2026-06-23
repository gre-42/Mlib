#pragma once
#include <Mlib/Os/Io/Binary_Bitwise_Words_Reader.hpp>
#include <Mlib/Os/Io/Binary_Bitwise_Words_Writer.hpp>
#include <Mlib/Type_Traits/Duration.hpp>
#include <concepts>

namespace Mlib {

template <ChronoDuration T>
void save(
    BinaryBitwiseWordsWriter& writer,
    const T& value,
    std::string_view message)
{
    auto count = std::chrono::duration_cast<std::chrono::duration<int64_t, std::nano>>(value).count();
    save<int64_t, true>(writer, count, "duration"); // true = allow_i64
}

template <ChronoDuration T>
void load(
    BinaryBitwiseWordsReader& reader,
    T& result,
    std::string_view message)
{
    int64_t nano;
    load<int64_t, true>(reader, nano, "duration"); // true = allow_i64
    result = std::chrono::duration_cast<T>(std::chrono::nanoseconds{nano});
}

}
