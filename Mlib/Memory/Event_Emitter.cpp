#include "Event_Emitter.hpp"
#include <Mlib/Os/Os.hpp>

using namespace Mlib;

EventReceiverDeletionToken::EventReceiverDeletionToken(EventEmitter& emitter)
    : emitter_{ &emitter }
{}

EventReceiverDeletionToken::~EventReceiverDeletionToken() {
    if (emitter_ == nullptr) {
        return;
    }
    if (emitter_->functions_.erase(this) != 1) {
        verbose_abort("Could not delete event function");
    }
}

EventEmitter::EventEmitter(std::function<bool()> fire_initial_event)
    : fire_initial_event_{ std::move(fire_initial_event) }
{
    if (!fire_initial_event_) {
        verbose_abort("fire_initial_event not valid");
    }
}

EventEmitter::~EventEmitter() {
    for (const auto& [e, _]: functions_) {
        e->emitter_ = nullptr;
    }
}

std::unique_ptr<EventReceiverDeletionToken> EventEmitter::insert(std::function<void()> f) {
    if (fire_initial_event_()) {
        f();
    }
    auto res = std::make_unique<EventReceiverDeletionToken>(*this);
    if (!functions_.try_emplace(res.get(), std::move(f)).second) {
        verbose_abort("Could not insert event function");
    }
    return res;
}

void EventEmitter::emit() {
    for (const auto& [_, f] : functions_) {
        f();
    }
}
