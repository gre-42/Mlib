#pragma once
#include <Mlib/Type_Traits/Enum.hpp>
#include <Mlib/Type_Traits/Scalar.hpp>
#include <concepts>
#include <string_view>

namespace Mlib {

class SerializationContextRead;
class SerializationContextWrite;
class BinaryBitwiseWordsReader;
class BinaryBitwiseWordsWriter;

template <class T>
concept ScalarOrEnum = Scalar<T> || Enum<T>;

template <ScalarOrEnum T>
void save(
    BinaryBitwiseWordsWriter& writer,
    const T& value,
    std::string_view message);

template <ScalarOrEnum T>
void load(
    BinaryBitwiseWordsReader& reader,
    T& result,
    std::string_view message);

}

#include <Mlib/Os/Io/Binary_Bitwise_Words_Reader.hpp>
#include <Mlib/Os/Io/Binary_Bitwise_Words_Writer.hpp>

namespace Mlib {

template <ScalarOrEnum T>
void save(
    BinaryBitwiseWordsWriter& writer,
    const T& value,
    std::string_view message)
{
    writer.write_binary(value, message);
}

template <ScalarOrEnum T>
void load(
    BinaryBitwiseWordsReader& reader,
    T& result,
    std::string_view message)
{
    result = reader.read_binary<T>(message);
}

}
