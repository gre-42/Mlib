#pragma once
#include <Mlib/Os/Io/Binary_Bitwise_Words_Reader.hpp>
#include <Mlib/Os/Io/Binary_Bitwise_Words_Writer.hpp>
#include <Mlib/Os/Io/Serialize/Serialization_Context_Read.hpp>
#include <Mlib/Os/Io/Serialize/Serialization_Context_Write.hpp>
#include <Mlib/Type_Traits/Unique_Ptr.hpp>
#include <memory>

namespace Mlib {

template <UniquePtr TUniquePtr>
void save(
    BinaryBitwiseWordsWriter& writer,
    const TUniquePtr& value,
    std::string_view message)
{
    if (value == nullptr) {
        throw std::runtime_error("unique_ptr is null during write");
    }
    save(writer, *value, message);
}

template <UniquePtr TUniquePtr>
void load(
    BinaryBitwiseWordsReader& reader,
    TUniquePtr& result,
    std::string_view message)
{
    if (result == nullptr) {
        throw std::runtime_error("unique_ptr is null");
    }
    load(reader, *result, message);
}

}
