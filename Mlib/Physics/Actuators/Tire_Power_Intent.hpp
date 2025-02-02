#pragma once

namespace Mlib {

enum class TirePowerIntentType {
    ACCELERATE,
    IDLE,
    BRAKE,
    BRAKE_OR_IDLE
};

struct TirePowerIntent {
    float power;
    float relaxation;
    TirePowerIntentType type;
};

}
