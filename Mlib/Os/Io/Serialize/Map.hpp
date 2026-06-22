#pragma once
#include <Mlib/Memory/Integral_Cast.hpp>
#include <Mlib/Os/Io/Binary_Bitwise_Words_Reader.hpp>
#include <Mlib/Os/Io/Binary_Bitwise_Words_Writer.hpp>
#include <Mlib/Type_Traits/Any_Std_Map.hpp>
#include <concepts>
#include <sstream>

namespace Mlib {

class SerializationContextRead;
class SerializationContextWrite;

template <AnyStdMap T>
void save(
    BinaryBitwiseWordsWriter& writer,
    const T& value,
    std::string_view message)
{
    save(writer, integral_cast<uint32_t>(value.size()), "map size");
    for (const auto& [k, v] : value) {
        save(writer, k, "map key");
        save(writer, v, "map value");
    }
}

template <AnyStdMap T>
void load(
    BinaryBitwiseWordsReader& reader,
    T& result,
    std::string_view message)
{
    auto size = reader.read_binary<uint32_t>("map size");
    if (size > 100'000) {
        throw std::runtime_error((std::stringstream() <<
            "Map size too large: " << size << ", " << message).str());
    }
    for (uint32_t i = 0; i < size; ++i) {
        typename T::key_type key;
        typename T::mapped_type value;
        load(reader, key, "map key");
        load(reader, value, "map value");
        if (!result.try_emplace(key, std::move(value)).second) {
            throw std::runtime_error("Detected duplicate key");
        }
    }
}

}
