#pragma once
#include <Mlib/Os/Io/Binary_Reader.hpp>
#include <Mlib/Os/Io/Bitwise_Word_Reader.hpp>
#include <Mlib/Type_Traits/Unsigned_Enum.hpp>
#include <concepts>
#include <cstdint>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <type_traits>

namespace Mlib {

// State-preserving Input Wrapper
template <std::unsigned_integral T>
class BitwiseWordsReader {
public:
    explicit BitwiseWordsReader(std::istream& istr, IoVerbosity verbosity) 
        : reader_{istr, verbosity}
        , active_word_(0, 0)
        , valid_bits_in_current_word_(0)
    {}

    // Extracts bits across packed word boundaries
    template <std::unsigned_integral TValue>
    TValue read_bits(size_t nbits, const char* message) {
        if (nbits == 0) {
            return 0;
        }
        TValue result = 0;
        size_t total_bits_read = 0;
        while (true) {
            if (active_word_.get_bit_index() >= valid_bits_in_current_word_) {
                fetch_next_word(message);
            }
            size_t remaining_bits_in_word = valid_bits_in_current_word_ - active_word_.get_bit_index();
            size_t bits_to_read = (nbits < remaining_bits_in_word) ? nbits : remaining_bits_in_word;

            T chunk = active_word_.extract(bits_to_read);
            
            // Reconstruct cleanly into the larger target type
            result |= (static_cast<TValue>(chunk) << total_bits_read);

            total_bits_read += bits_to_read;
            nbits -= bits_to_read;
            if (nbits == 0) {
                break;
            }
        }
        return result;
    }

    template <UnsignedEnum TValue>
    TValue read_bits(size_t nbits, const char* message) {
        return (TValue)read_bits<std::underlying_type_t<TValue>>(nbits, message);
    }

    // Resets the active buffer to match the writer's partial flush
    void align_to_next_word() {
        // Force the bit index to meet or exceed valid bits, draining the current word
        active_word_ = BitwiseWordReader<T>(0, 0);
        valid_bits_in_current_word_ = 0; 
    }
private:
    BinaryReader reader_;
    BitwiseWordReader<T> active_word_;
    const size_t bits_per_word = sizeof(T) * 8;
    size_t valid_bits_in_current_word_;

    void fetch_next_word(const char* message) {
        T data = reader_.read_binary<T>(message);
        valid_bits_in_current_word_ = bits_per_word;
        active_word_ = BitwiseWordReader<T>(data, 0);
    }
};

}
