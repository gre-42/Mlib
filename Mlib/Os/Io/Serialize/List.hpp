#pragma once
#include <Mlib/Memory/Integral_Cast.hpp>
#include <Mlib/Os/Io/Binary_Bitwise_Words_Reader.hpp>
#include <Mlib/Os/Io/Binary_Bitwise_Words_Writer.hpp>
#include <Mlib/Os/Io/Serialize/0Number.hpp>
#include <Mlib/Type_Traits/List.hpp>
#include <concepts>
#include <sstream>

namespace Mlib {

class SerializationContextRead;
class SerializationContextWrite;

template <StdList T>
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

template <StdList T>
void load(
    BinaryBitwiseWordsReader& reader,
    T& result,
    std::string_view message)
{
    uint32_t size;
    load(reader, size, "list size");
    if (size > 100'000) {
        throw std::runtime_error((std::stringstream() <<
            "List size too large: " << size << ", " << message).str());
    }
    for (uint32_t i = 0; i < size; ++i) {
        load(reader, result.emplace_back(), "list value");
    }
}

}
