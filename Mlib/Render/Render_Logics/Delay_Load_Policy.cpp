#include "Delay_Load_Policy.hpp"
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

DelayLoadPolicy Mlib::delay_load_policy_from_string(const std::string& str) {
    if (str == "delay") {
        return DelayLoadPolicy::DELAY;
    } else if (str == "no_delay") {
        return DelayLoadPolicy::NO_DELAY;
    } else {
        THROW_OR_ABORT("Unknown delay-load policy: \"" + str + '"');
    }
}
