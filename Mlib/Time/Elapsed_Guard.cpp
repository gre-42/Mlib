#include "Elapsed_Guard.hpp"
#include <Mlib/Os/Os.hpp>

using namespace Mlib;

ElapsedGuard::ElapsedGuard()
    : start_time_{std::chrono::steady_clock::now()}
{}

ElapsedGuard::~ElapsedGuard() {
    linfo() << "Elapsed: " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start_time_).count() << " ms";
}
