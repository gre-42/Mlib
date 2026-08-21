#include "Remote_Statistics_Verbosity.hpp"

using namespace Mlib;

static bool g_print_transmission_stastics = false;

void Mlib::set_print_transmission_stastics(bool enabled) {
    g_print_transmission_stastics = enabled;
}

bool Mlib::get_print_transmission_stastics() {
    return g_print_transmission_stastics;
}
