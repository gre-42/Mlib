#pragma once
#include <Mlib/Memory/Integral_Cast.hpp>
#include <Mlib/Os/Io/Binary_Bitwise_Words_Reader.hpp>
#include <Mlib/Os/Io/Binary_Bitwise_Words_Writer.hpp>
#include <Mlib/Os/Io/Serialize/Number.hpp>
#include <Mlib/Type_Traits/List.hpp>
#include <concepts>

namespace Mlib {

class SerializationContextRead;
class SerializationContextWrite;

template <StdList T>
void save(
    BinaryBitwiseWordsWriter& writer,
    SerializationContextWrite& ctx,
    const T& value,
    std::string_view message)
{
    save(writer, ctx, integral_cast<uint32_t>(value.size()), "list size");
    for (const auto& v : value) {
        save(writer, ctx, v, "list value");
    }
}

template <StdList T>
T load(
    BinaryBitwiseWordsReader& reader,
    SerializationContextRead& ctx,
    std::string_view message)
{
    auto size = load<uint32_t>(reader, ctx, "list size");
    if (size > 10'000) {
        throw std::runtime_error("List size too large");
    }
    T result;
    for (uint32_t i = 0; i < size; ++i) {
        auto value = load<T::value_type>(reader, ctx, "list value");
        result.push_back(std::move(value));
    }
}

}
