
#include "Recursion_Guard.hpp"
#include <stdexcept>

using namespace Mlib;

RecursionCounter::RecursionCounter()
: recursion_counter_{0}
{}

void RecursionCounter::operator ++() {
    if (recursion_counter_ > 100) {
        throw std::runtime_error("Possibly infinite recursion detected");
    }
    ++recursion_counter_;
}

void RecursionCounter::operator --() {
    --recursion_counter_;
}

RecursionGuard::RecursionGuard(RecursionCounter& recursion_counter)
: recursion_counter_{recursion_counter}
{
    ++recursion_counter;
}

RecursionGuard::~RecursionGuard() {
    --recursion_counter_;
}
