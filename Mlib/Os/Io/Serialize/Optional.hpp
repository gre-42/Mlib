#pragma once
#include <Mlib/Memory/Integral_Cast.hpp>
#include <Mlib/Os/Io/Binary_Bitwise_Words_Reader.hpp>
#include <Mlib/Os/Io/Binary_Bitwise_Words_Writer.hpp>
#include <Mlib/Os/Io/Serialize/0Number.hpp>
#include <Mlib/Type_Traits/Optional.hpp>
#include <concepts>

namespace Mlib {

class SerializationContextRead;
class SerializationContextWrite;

template <StdOptional T>
void save(
    BinaryBitwiseWordsWriter& writer,
    const T& value,
    std::string_view message)
{
    save(writer, integral_cast<uint8_t>(value.has_value()), "optional has_value");
    if (value.has_value()) {
        save(writer, *value, "optional value");
    }
}

template <StdOptional T>
void load(
    BinaryBitwiseWordsReader& reader,
    T& result,
    std::string_view message)
{
    uint8_t has_value;
    load(reader, has_value, "optional has_value");
    if (has_value) {
        result.emplace();
        load(reader, *result, "optional value");
    }
}

}
