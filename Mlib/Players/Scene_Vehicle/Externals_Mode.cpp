#include "Externals_Mode.hpp"
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

ExternalsMode Mlib::externals_mode_from_string(const std::string& externals_mode) {
    if (externals_mode == "pc") {
        return ExternalsMode::PC;
    } else if (externals_mode == "npc") {
        return ExternalsMode::NPC;
    } else {
        THROW_OR_ABORT("Unknown externals mode: " + externals_mode);
    }
}
