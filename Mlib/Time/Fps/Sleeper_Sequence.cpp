#include "Sleeper_Sequence.hpp"

using namespace Mlib;

SleeperSequence::SleeperSequence(ISleeper& a, ISleeper& b)
: a_{a}, b_{b}
{}

void SleeperSequence::tick() {
    a_.tick();
    b_.tick();
}

void SleeperSequence::reset() {
    a_.reset();
    b_.reset();
}

bool SleeperSequence::is_up_to_date() const {
    return a_.is_up_to_date() && b_.is_up_to_date();
}
