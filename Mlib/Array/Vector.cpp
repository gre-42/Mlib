#include "Vector.hpp"

using namespace Mlib;

static bool debug_vector_allocation_ = false;

void Mlib::debug_vector_allocation(bool value) {
    debug_vector_allocation_ = value;
}

bool Mlib::debug_vector_allocation() {
    return debug_vector_allocation_;
}
