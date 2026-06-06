#pragma once
#include <Mlib/Os/Io/Binary_Reader.hpp>
#include <Mlib/Os/Io/Bitwise_Words_Reader.hpp>
#include <cstdint>

namespace Mlib {

class BinaryBitwiseWordsReader;

class ReadingArchive {
public:
    inline ReadingArchive(BinaryBitwiseWordsReader& reader, std::string_view message)
        : reader_{reader}
        , message_{message}
    {}
    struct is_saving {
        static const bool value = false;
    };
    void operator () (auto& element);

private:
    BinaryBitwiseWordsReader& reader_;
    std::string_view message_;
};

class BinaryBitwiseWordsReader {
public:
    BinaryBitwiseWordsReader(std::istream& istr, IoVerbosity verbosity)
        : words_reader_{istr, verbosity}
        , binary_reader_{istr, verbosity}
    {}
    template <std::integral LengthType>
    inline std::string read_string(std::string_view message) {
        words_reader_.align_to_next_word();
        return binary_reader_.read_string<LengthType>(message);
    }
    template <class T>
    T read_binary(std::string_view message) {
        words_reader_.align_to_next_word();
        return binary_reader_.read_binary<T>(message);
    }
    template <class TData>
    void read_vector(const std::span<TData>& vec, std::string_view message) {
        words_reader_.align_to_next_word();
        binary_reader_.read_vector(vec, message);
    }
    template <class TVec>
    void read_vector(TVec& vec, std::string_view message) {
        words_reader_.align_to_next_word();
        binary_reader_.read_vector(vec, message);
    }
    inline std::vector<std::byte> read_all_vector(std::string_view message) {
        words_reader_.align_to_next_word();
        return binary_reader_.read_all_vector(message);
    }
    inline void seek_relative_positive(std::streamoff amount) {
        words_reader_.align_to_next_word();
        binary_reader_.seek_relative_positive(amount);
    }
    template <std::unsigned_integral TValue>
    TValue read_bits(size_t nbits, std::string_view message) {
        return words_reader_.read_bits<TValue>(nbits, message);
    }
    template <UnsignedEnum TValue>
    TValue read_bits(size_t nbits, std::string_view message) {
        return words_reader_.read_bits<TValue>(nbits, message);
    }
    bool read_bool_bit(std::string_view message) {
        return read_bits<uint8_t>(1, message) != 0;
    }
    template <class TValue>
    TValue deserialize(std::string_view message) {
        ReadingArchive archive{*this, message};
        words_reader_.align_to_next_word();
        TValue result;
        result.serialize(archive);
        return result;
    }
    void align_to_next_word() {
        words_reader_.align_to_next_word();
    }
private:
    BitwiseWordsReader<uint8_t> words_reader_;
    BinaryReader binary_reader_;
};

void ReadingArchive::operator () (auto& element) {
    element = reader_.read_binary<std::remove_reference_t<decltype(element)>>(message_);
}

}
