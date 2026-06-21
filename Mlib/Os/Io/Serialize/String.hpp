#pragma once
#include <Mlib/Memory/Integral_Cast.hpp>
#include <Mlib/Os/Io/Binary_Bitwise_Words_Reader.hpp>
#include <Mlib/Os/Io/Binary_Bitwise_Words_Writer.hpp>
#include <Mlib/Type_Traits/String.hpp>
#include <concepts>

namespace Mlib {

class SerializationContextRead;
class SerializationContextWrite;

template <StdBasicString T>
void save(
    BinaryBitwiseWordsWriter& writer,
    SerializationContextWrite& ctx,
    const T& value,
    std::string_view message)
{
    writer.write_string<uint32_t>(value, message);
}

template <StdBasicString T>
T load(
    BinaryBitwiseWordsReader& reader,
    SerializationContextRead& ctx,
    std::string_view message)
{
    return reader.read_string<uint32_t>(message);
}

}
