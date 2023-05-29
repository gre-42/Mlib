#include "Elapsed_Guard.hpp"
#include <Mlib/Os/Os.hpp>

using namespace Mlib;

ElapsedGuard::ElapsedGuard()
: start_time_{std::chrono::steady_clock::now()}
{}

ElapsedGuard::~ElapsedGuard() {
    linfo() << "Elapsed: " << (std::chrono::steady_clock::now() - start_time_).count() / (1000 * 1000) << " ms";
}
