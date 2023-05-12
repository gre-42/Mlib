#include "Event_Emitter.hpp"
#include <Mlib/Os/Os.hpp>

using namespace Mlib;

EventReceiverDeletionToken::EventReceiverDeletionToken(EventEmitter* emitter)
: emitter_{emitter}
{}

EventReceiverDeletionToken::~EventReceiverDeletionToken() {
    if (emitter_->functions_.erase(this) != 1) {
        verbose_abort("Could not delete event function");
    }
}

std::unique_ptr<EventReceiverDeletionToken> EventEmitter::insert(std::function<void()> f) {
    auto res = std::make_unique<EventReceiverDeletionToken>(this);
    if (!functions_.insert({res.get(), f}).second) {
        verbose_abort("Could not insert event function");
    }
    return res;
}

void EventEmitter::emit() {
    for (const auto& [_, f] : functions_) {
        f();
    }
}
