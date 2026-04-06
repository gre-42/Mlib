#pragma once
#include <Mlib/Map/Verbose_Map.hpp>
#include <cstdint>
#include <string>

namespace Mlib {

static const VerboseMap<uint32_t, int> tap_analog_axes_map {
    "Tap analog axis",
    [](uint32_t key){ return std::to_string(key); },
    {
        {1, 0},
        {2, 1},
        {3, 2},
        {4, 3},
        {5, 4},
        {6, 5},
        {7, 6},
        {8, 7},
        {9, 8},
        {10, 9},
        {11, 10},
        {12, 11},
        {13, 12},
        {14, 13},
        {15, 14},
        {16, 15}
    }};

}
