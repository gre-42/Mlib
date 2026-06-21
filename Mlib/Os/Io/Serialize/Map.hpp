#pragma once
#include <Mlib/Memory/Integral_Cast.hpp>
#include <Mlib/Os/Io/Binary_Bitwise_Words_Reader.hpp>
#include <Mlib/Os/Io/Binary_Bitwise_Words_Writer.hpp>
#include <Mlib/Type_Traits/Any_Std_Map.hpp>
#include <concepts>

namespace Mlib {

class SerializationContextRead;
class SerializationContextWrite;

template <AnyStdMap T>
void save(
    BinaryBitwiseWordsWriter& writer,
    SerializationContextWrite& ctx,
    const T& value,
    std::string_view message)
{
    save(writer, ctx, integral_cast<uint32_t>(value.size()), "map size");
    for (const auto& [k, v] : value) {
        save(writer, ctx, k, "map key");
        save(writer, ctx, v, "map value");
    }
}

template <AnyStdMap T>
T load(
    BinaryBitwiseWordsReader& reader,
    SerializationContextRead& ctx,
    std::string_view message)
{
    auto size = reader.read_binary<uint32_t>("map size");
    if (size > 10'000) {
        throw std::runtime_error("Map size too large");
    }
    T result;
    for (uint32_t i = 0; i < size; ++i) {
        auto key = load<typename T::key_type>(reader, ctx, "map key");
        auto value = load<typename T::mapped_type>(reader, ctx, "map value");
        if (!result.try_emplace(key, std::move(value)).second) {
            throw std::runtime_error("Detected duplicate key");
        }
    }
    return result;
}

}
