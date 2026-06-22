#pragma once
#include <Mlib/Memory/Integral_Cast.hpp>
#include <Mlib/Os/Io/Binary_Bitwise_Words_Reader.hpp>
#include <Mlib/Os/Io/Binary_Bitwise_Words_Writer.hpp>
#include <Mlib/Type_Traits/Any_Std_Set.hpp>
#include <concepts>

namespace Mlib {

class SerializationContextRead;
class SerializationContextWrite;

template <AnyStdSet T>
void save(
    BinaryBitwiseWordsWriter& writer,
    const T& value,
    std::string_view message)
{
    save(writer, integral_cast<uint32_t>(value.size()), "list size");
    for (const auto& v : value) {
        save(writer, v, "list value");
    }
}

template <AnyStdSet T>
void load(
    BinaryBitwiseWordsReader& reader,
    T& result,
    std::string_view message)
{
    uint32_t size;
    load(reader, size, "list size");
    if (size > 10'000) {
        throw std::runtime_error("List size too large");
    }
    for (uint32_t i = 0; i < size; ++i) {
        typename T::value_type value;
        load(reader, value, "list value");
        if (!result.insert(std::move(value)).second) {
            throw std::runtime_error("Detected duplicate element");
        }
    }
}

}
