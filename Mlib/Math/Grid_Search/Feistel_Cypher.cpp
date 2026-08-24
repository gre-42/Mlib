#include "Feistel_Cypher.hpp"
#include <Mlib/Memory/Integral_Cast.hpp>
#include <Mlib/Os/Os.hpp>
#include <algorithm>
#include <bit>
#include <cstdint>
#include <stdexcept>

using namespace Mlib;

// From: Google Gemini
// Reversible round mixing function
static uint32_t round_function(uint32_t val, uint32_t round, uint32_t key) {
    uint32_t hash = val ^ round ^ key;
    hash = ((hash >> 16) ^ hash) * 0x45d9f3b;
    hash = ((hash >> 16) ^ hash) * 0x45d9f3b;
    hash = (hash >> 16) ^ hash;
    return hash;
}

// From: Google Gemini
static uint32_t get_unique_random(uint32_t index, uint32_t total_bits, uint32_t key) {
    uint32_t left_bits = total_bits - (total_bits / 2);
    uint32_t right_bits = total_bits / 2;

    uint32_t left = index >> right_bits;
    uint32_t right = index & ((1U << right_bits) - 1);

    for (uint32_t round = 0; round < 4; ++round) {
        uint32_t left_mask = (1U << left_bits) - 1;
        
        // Next left takes the current right's value (inheriting right_bits size)
        uint32_t next_left = right;
        // Next right takes the current left combined with the hash, masked to left_bits size
        uint32_t next_right = (left ^ round_function(right, round, key)) & left_mask;
        
        left = next_left;
        right = next_right;
        
        // Essential: Swap capacities because the values themselves swapped roles
        std::swap(left_bits, right_bits);
    }

    // After an even number of rounds (4), left_bits and right_bits have swapped 
    // an even number of times, returning safely to their original initial layout sizing.
    return (left << right_bits) | right;
}

// From: Google Gemini
Generator<uint32_t> Mlib::feistel_generator(uint32_t n, uint32_t key) {
    if (n == 0) {
        co_return;
    }

    // Determine the exact number of bits required to contain (n - 1)
    uint32_t total_bits = static_cast<uint32_t>(std::bit_width(n - 1));
    uint32_t total_elements = (1u << total_bits);

    for (uint32_t i = 0; i < total_elements; ++i) {
        auto result = get_unique_random(i, total_bits, key);
        if (result >= n) {
            continue; // Safely cycle-walk past values out of bounds
        }
        co_yield result;
    }
}
