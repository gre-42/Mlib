#pragma once
#include "Event_Emitter.hpp"
#include <Mlib/Os/Os.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

namespace Mlib {

template <class... Args>
EventReceiverDeletionToken<Args...>::EventReceiverDeletionToken(
    Emitter& emitter,
    std::function<void(const Args&...)> f)
    : emitter_{ &emitter }
{
    emitter.insert(*this, std::move(f));
}

template <class... Args>
EventReceiverDeletionToken<Args...>::~EventReceiverDeletionToken() {
    reset();
}

template <class... Args>
void EventReceiverDeletionToken<Args...>::reset() {
    if (emitter_ == nullptr) {
        return;
    }
    if (emitter_->functions_.erase(this) != 1) {
        verbose_abort("Could not delete event function");
    }
    emitter_ = nullptr;
}

template <class... Args>
EventEmitter<Args...>::EventEmitter(OnInsert on_insert)
    : on_insert_{ std::move(on_insert) }
{}

template <class... Args>
EventEmitter<Args...>::~EventEmitter() {
    clear();
}

template <class... Args>
void EventEmitter<Args...>::clear() {
    for (const auto& [e, _]: functions_) {
        e->emitter_ = nullptr;
    }
}

template <class... Args>
void EventEmitter<Args...>::insert(
    DeletionToken& deletion_token,
    EventCallback f)
{
    if (on_insert_) {
        on_insert_(f);
    }
    if (!functions_.try_emplace(&deletion_token, std::move(f)).second) {
        THROW_OR_ABORT("Deletion token already exists");
    }
}

template <class... Args>
void EventEmitter<Args...>::emit(const Args&... args) {
    for (const auto& [e, f] : functions_) {
        if (e->emitter_ != nullptr) {
            f(args...);
        }
    }
}

}
