#pragma once
#include <Mlib/Os/Io/Binary_Writer.hpp>
#include <Mlib/Os/Io/Bitwise_Words_Writer.hpp>
#include <cstdint>

namespace Mlib {

class BinaryBitwiseWordsWriter;

class WritingArchive {
public:
    WritingArchive(BinaryBitwiseWordsWriter& writer, const char* message)
        : writer_{writer}
        , message_{message}
    {}
    struct is_saving {
        static const bool value = true;
    };
    void operator () (const auto& element);

private:
    BinaryBitwiseWordsWriter& writer_;
    const char* message_;
};

class BinaryBitwiseWordsWriter {
public:
    inline explicit BinaryBitwiseWordsWriter(std::ostream& ostr)
        : words_writer_{ ostr }
        , binary_writer_{ ostr }
    {}
    template <std::integral LengthType>
    inline void write_string(const std::string& str, const char* message) {
        words_writer_.flush_partial(message);
        binary_writer_.write_string<LengthType>(str, message);
    }
    template <class T>
    inline void write_binary(const T& v, const char* message) {
        words_writer_.flush_partial(message);
        binary_writer_.write_binary(v, message);
    }
    template <class TIterable>
    void write_iterable(const TIterable& iterable, const char* message) {
        words_writer_.flush_partial(message);
        binary_writer_.write_iterable(iterable, message);
    }
    template <class T>
    void write_bits(const T& value, size_t nbits, const char* message) {
        words_writer_.write_bits(value, nbits, message);
    }
    void write_bool_bit(bool value, const char* message) {
        write_bits((uint8_t)value, 1, message);
    }
    template <class T>
    void serialize(const T& value, const char* message) {
        WritingArchive archive{*this, message};
        words_writer_.flush_partial(message);
        const_cast<T&>(value).serialize(archive);
    }
    inline void flush_partial(const char* message) {
        words_writer_.flush_partial(message);
    }
private:
    BitwiseWordsWriter<uint8_t> words_writer_;
    BinaryWriter binary_writer_;
};

void WritingArchive::operator () (const auto& element) {
    writer_.write_binary(element, message_);
}

}
