#include "Sleeper_Sequence.hpp"

using namespace Mlib;

SleeperSequence::SleeperSequence(std::vector<ISleeper*> sleepers)
    : sleepers_{std::move(sleepers)}
{}

void SleeperSequence::tick() {
    for (const auto& s : sleepers_) {
        s->tick();
    }
}

void SleeperSequence::reset() {
    for (const auto &s : sleepers_) {
        s->reset();
    }
}

bool SleeperSequence::is_up_to_date() const {
    for (const auto &s : sleepers_) {
        if (!s->is_up_to_date()) {
            return false;
        }
    }
    return true;
}
