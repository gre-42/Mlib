#include "Externals_Mode.hpp"
#include <stdexcept>

using namespace Mlib;

ExternalsMode Mlib::externals_mode_from_string(const std::string& externals_mode) {
    if (externals_mode == "pc") {
        return ExternalsMode::PC;
    } else if (externals_mode == "npc") {
        return ExternalsMode::NPC;
    } else {
        throw std::runtime_error("Unknown externals mode: " + externals_mode);
    }
}
