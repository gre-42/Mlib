#pragma once
#include <string>

namespace Mlib {

enum class DelayLoadPolicy {
    DELAY,
    NO_DELAY
};

DelayLoadPolicy delay_load_policy_from_string(const std::string& str);

}
