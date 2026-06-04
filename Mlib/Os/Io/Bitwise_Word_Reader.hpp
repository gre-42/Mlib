#pragma once
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <type_traits>

namespace Mlib {

// Compatible Deserialization Class
template <typename T>
class BitwiseWordReader {
    static_assert(std::is_integral<T>::value && std::is_unsigned<T>::value, "Template type must be an unsigned integral.");

private:
    T storage;
    size_t bit_index;

public:
    // Constructor loads the serialized value and initial reading index
    explicit BitwiseWordReader(T serialized_value, size_t initial_counter = 0)
        : storage(serialized_value), bit_index(initial_counter) {}

    // Extracts the next 'nbits' from the storage
    T extract(size_t nbits) {
        if (nbits == 0) return 0;

        // Prevent reading past bounds
        if (bit_index + nbits > sizeof(T) * 8) {
            throw std::out_of_range("Attempted to read past serialized bit storage boundary.");
        }

        // Shift storage right to align target bits to the bottom
        T shifted = storage >> bit_index;

        // Mask out everything above the requested bit count
        T mask = (nbits == sizeof(T) * 8)
            ? static_cast<T>(~0u)
            : (static_cast<T>(1u) << nbits) - 1u;
        
        bit_index += nbits;
        return shifted & mask;
    }

    // Getters
    size_t get_bit_index() const { return bit_index; }
};

}
