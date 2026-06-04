#pragma once
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <type_traits>

namespace Mlib {

// Core Serialization Class
template <typename T = uint64_t>
class BitwiseWordWriter {
    static_assert(std::is_integral<T>::value && std::is_unsigned<T>::value, "Template type must be an unsigned integral.");

private:
    T storage;
    size_t bit_index;

public:
    // Constructor initializes storage and the bit counter
    explicit BitwiseWordWriter(T initial_value = 0, size_t initial_counter = 0)
        : storage(initial_value), bit_index(initial_counter) {}

    // Appends the lowest 'nbits' of 'value' to the storage
    void append(T value, size_t nbits) {
        if (nbits == 0) return;
        
        // Prevent overflow of the underlying storage type
        if (bit_index + nbits > sizeof(T) * 8) {
            throw std::overflow_error("Not enough bit capacity remaining in storage.");
        }
        if (nbits > sizeof(T) * 8) {
            throw std::runtime_error("Requested bits exceed type capacity.");
        }

        // Create a mask for safety (handles nbits == total bits safely)
        T mask = (nbits == sizeof(T) * 8)
            ? static_cast<T>(~0u)
            : (static_cast<T>(1u) << nbits) - 1u;
        T clean_value = value & mask;

        // Shift incoming value to current index and merge
        storage |= (clean_value << bit_index);
        bit_index += nbits;
    }

    // Getters
    T get_storage() const { return storage; }
    size_t get_bit_index() const { return bit_index; }
};

}
