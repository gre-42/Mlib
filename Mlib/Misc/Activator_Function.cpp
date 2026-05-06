#include "Activator_Function.hpp"

using namespace Mlib;

Activator::Activator(std::weak_ptr<ActivationState> state)
    : state_{ state }
{}

Activator::~Activator() = default;

void Activator::operator () () {
    if (auto ls = state_.lock()) {
        (*ls)();
    }
}

ActivationState::ActivationState(std::function<void()> func)
    : func_{ std::move(func) }
    , is_activated_{ false }
{}

ActivationState::~ActivationState() = default;

void ActivationState::notify_deactivated() {
    is_activated_ = false;
}

Activator ActivationState::generate_activator() {
    return { shared_from_this() };
}

void ActivationState::operator () () {
    if (!is_activated_) {
        func_();
        is_activated_ = true;
    }
}
