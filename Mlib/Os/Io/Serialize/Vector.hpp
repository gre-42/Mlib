#pragma once
#include <Mlib/Memory/Integral_Cast.hpp>
#include <Mlib/Os/Io/Binary_Bitwise_Words_Reader.hpp>
#include <Mlib/Os/Io/Binary_Bitwise_Words_Writer.hpp>
#include <Mlib/Type_Traits/Vector.hpp>
#include <concepts>

namespace Mlib {

class SerializationContextRead;
class SerializationContextWrite;

template <StdVector T>
void save(
    BinaryBitwiseWordsWriter& writer,
    SerializationContextWrite& ctx,
    const T& value,
    std::string_view message)
{
    save(writer, ctx, integral_cast<uint32_t>(value.size()), "vector size");
    for (const auto& v : value) {
        save(writer, ctx, v, "vector value");
    }
}

template <StdVector T>
T load(
    BinaryBitwiseWordsReader& reader,
    SerializationContextRead& ctx,
    std::string_view message)
{
    auto size = load<uint32_t>(reader, ctx, "vector size");
    if (size > 10'000) {
        throw std::runtime_error("vector size too large");
    }
    T result;
    result.reserve(size);
    for (uint32_t i = 0; i < size; ++i) {
        auto value = load<typename T::value_type>(reader, ctx, "vector value");
        result.push_back(std::move(value));
    }
    return result;
}

}
