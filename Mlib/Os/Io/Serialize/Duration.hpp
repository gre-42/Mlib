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
    save(writer, value.count(), "duration");
}

template <ChronoDuration T>
void load(
    BinaryBitwiseWordsReader& reader,
    T& result,
    std::string_view message)
{
    typename T::rep count;
    load(reader, count, "duration");
    result = T{count};
}

}
