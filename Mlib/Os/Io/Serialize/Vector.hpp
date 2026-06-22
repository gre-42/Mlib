#pragma once
#include <Mlib/Memory/Integral_Cast.hpp>
#include <Mlib/Os/Io/Binary_Bitwise_Words_Reader.hpp>
#include <Mlib/Os/Io/Binary_Bitwise_Words_Writer.hpp>
#include <Mlib/Type_Traits/Vector.hpp>
#include <concepts>
#include <sstream>

namespace Mlib {

class SerializationContextRead;
class SerializationContextWrite;

template <StdVector T>
void save(
    BinaryBitwiseWordsWriter& writer,
    const T& value,
    std::string_view message)
{
    save(writer, integral_cast<uint32_t>(value.size()), "vector size");
    for (const auto& v : value) {
        save(writer, v, "vector value");
    }
}

template <StdVector T>
void load(
    BinaryBitwiseWordsReader& reader,
    T& result,
    std::string_view message)
{
    uint32_t size;
    load(reader, size, "vector size");
    if (size > 100'000) {
        throw std::runtime_error((std::stringstream() <<
            "Vector size too large: " << size << ", " << message).str());
    }
    result.reserve(size);
    for (uint32_t i = 0; i < size; ++i) {
        typename T::value_type value;
        load(reader, value, "vector value");
        result.push_back(std::move(value));
    }
}

}
