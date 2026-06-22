#pragma once
#include "Class_Fwd.hpp"
#include <Mlib/Os/Io/Binary_Bitwise_Words_Reader.hpp>
#include <Mlib/Os/Io/Binary_Bitwise_Words_Writer.hpp>

namespace Mlib {

template <HasSerializeNoSharedPtr T>
void save(
    BinaryBitwiseWordsWriter& writer,
    const T& value,
    std::string_view message)
{
    writer.flush_partial(message);
    WritingArchive archive{writer, message};
    const_cast<T&>(value).serialize(archive);
}

template <HasSerializeWithoutLoadAndConstructNoSharedPtr T>
void load(
    BinaryBitwiseWordsReader& reader,
    T& result,
    std::string_view message)
{
    reader.align_to_next_word();
    ReadingArchive archive{reader, message};
    result.serialize(archive);
}

template <HasLoadAndConstructNoSharedPtr T>
T load(
    BinaryBitwiseWordsReader& reader,
    std::string_view message)
{
    reader.align_to_next_word();
    ReadingArchive archive{reader, message};
    ObjectBlob<T> buffer;
    ConstructInplace<T> construct{buffer};
    T::load_and_construct(archive, construct);
    T result = std::move((T&)buffer);
    ((T&)buffer).~T();
    return result;
}

}
