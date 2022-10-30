#pragma once
#include <cmath>
#include <cstdint>

namespace Mlib {

struct PlayerStats {
    float best_lap_time = INFINITY;
    float race_time = INFINITY;
    uint32_t rank = UINT32_MAX;
    uint32_t nlaps = 0;
    uint32_t nkills = 0;
    uint32_t ndeaths = 0;
    uint32_t nwins = 0;
    uint32_t nlosses = 0;
};

}
