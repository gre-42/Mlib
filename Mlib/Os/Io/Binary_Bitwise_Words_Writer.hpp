#pragma once
#include <Mlib/Os/Io/Binary_Writer.hpp>
#include <Mlib/Os/Io/Bitwise_Words_Writer.hpp>
#include <cstdint>

namespace Mlib {

class BinaryBitwiseWordsWriter;
class SerializationContextWrite;

class WritingArchive {
public:
    WritingArchive(
        BinaryBitwiseWordsWriter& writer,
        SerializationContextWrite& ctx,
        std::string_view message)
        : writer_{writer}
        , ctx_{ctx}
        , message_{message}
    {}
    struct is_saving {
        static const bool value = true;
    };
    void operator () (const auto& element);

private:
    BinaryBitwiseWordsWriter& writer_;
    SerializationContextWrite& ctx_;
    std::string_view message_;
};

class BinaryBitwiseWordsWriter {
public:
    inline explicit BinaryBitwiseWordsWriter(std::ostream& ostr)
        : words_writer_{ ostr }
        , binary_writer_{ ostr }
    {}
    template <std::integral LengthType>
    inline void write_string(const std::string& str, std::string_view message) {
        words_writer_.flush_partial(message);
        binary_writer_.write_string<LengthType>(str, message);
    }
    template <class T>
    inline void write_binary(const T& v, std::string_view message) {
        words_writer_.flush_partial(message);
        binary_writer_.write_binary(v, message);
    }
    template <class TIterable>
    void write_iterable(const TIterable& iterable, std::string_view message) {
        words_writer_.flush_partial(message);
        binary_writer_.write_iterable(iterable, message);
    }
    template <class T>
    void write_bits(const T& value, size_t nbits, std::string_view message) {
        words_writer_.write_bits(value, nbits, message);
    }
    void write_bool_bit(bool value, std::string_view message) {
        write_bits((uint8_t)value, 1, message);
    }
    template <class T>
    void serialize(const T& value, SerializationContextWrite& ctx, std::string_view message) {
        WritingArchive archive{*this, ctx, message};
        words_writer_.flush_partial(message);
        const_cast<T&>(value).serialize(archive);
    }
    inline void flush_partial(std::string_view message) {
        words_writer_.flush_partial(message);
    }
private:
    BitwiseWordsWriter<uint8_t> words_writer_;
    BinaryWriter binary_writer_;
};

}

#include "Binary_Bitwise_Words_Writer.impl.hpp"
