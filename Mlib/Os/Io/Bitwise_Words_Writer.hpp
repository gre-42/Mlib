#pragma once
#include <Mlib/Os/Io/Binary_Writer.hpp>
#include <Mlib/Os/Io/Bitwise_Word_Writer.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Type_Traits/Unsigned_Enum.hpp>
#include <concepts>
#include <cstdint>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <type_traits>

namespace Mlib {

// State-preserving Output Wrapper
template <std::unsigned_integral T>
class BitwiseWordsWriter {
public:
    explicit BitwiseWordsWriter(std::ostream& ostr)
        : writer_{ostr}
        , active_word_{0, 0}
    {}

    // Destructor guarantees that any leftover bits are flushed out
    ~BitwiseWordsWriter() {
        if (active_word_.get_bit_index() > 0) {
            lwarn() << "BitwiseWordsWriter::flush_partial not called, flushing in dtor";
            try {
                flush_partial("BitwiseWordsWriter dtor");
            } catch (...) {}
        }
    }

    // Appends bits. If it exceeds the current word, it splits across the boundary.
    template <std::unsigned_integral TValue>
    void write_bits(TValue value, size_t nbits, std::string_view message) {
        if ((value & ~TValue((1 << nbits) - 1)) != 0) {
            throw std::runtime_error((std::stringstream() <<
                "Value too large for " << nbits << " bits: 0x" << std::hex << (value + 0)).str());
        }
        while (nbits > 0) {
            size_t available_bits = bits_per_word - active_word_.get_bit_index();
            size_t bits_to_write = std::min(nbits, available_bits);

            // Safe explicit cast to 'T' after isolating the exact chunk matching bits_to_write
            TValue mask = (bits_to_write == sizeof(TValue) * 8)
                ? static_cast<TValue>(~(T)0)
                : (static_cast<TValue>(1) << bits_to_write) - 1;
            T chunk = static_cast<T>(value & mask);

            active_word_.append(chunk, bits_to_write);

            if (active_word_.get_bit_index() == bits_per_word) {
                flush_full_word(message);
            }

            nbits -= bits_to_write;
            value >>= bits_to_write;
        }
    }

    template <UnsignedEnum TValue>
    void write_bits(TValue value, size_t nbits, std::string_view message) {
        write_bits((std::underlying_type_t<TValue>)value, nbits, message);
    }

    // Flushes residual bits when closing or destroying the wrapper
    void flush_partial(std::string_view message) {
        if (active_word_.get_bit_index() > 0) {
            flush_full_word(message);
        }
    }
private:
    BinaryWriter writer_;
    BitwiseWordWriter<T> active_word_;
    const size_t bits_per_word = sizeof(T) * 8;

    // Flushes a completely packed word to the stream
    void flush_full_word(std::string_view message) {
        T data = active_word_.get_storage();
        writer_.write_binary(data, message);
        active_word_ = BitwiseWordWriter<T>(0, 0); // Reset
    }
};

}
