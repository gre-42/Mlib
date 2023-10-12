#include "Dispatcher.hpp"

using namespace Mlib;

Dispatcher::Dispatcher(std::chrono::milliseconds wait_time)
    : nparticipants_{ 0 }
    , nwaiting_{ 0 }
    , nproduced_{ 0 }
    , wait_time_{ wait_time }
{}

Dispatcher::~Dispatcher() = default;

void Dispatcher::produce() {
    std::unique_lock lock(mutex_);
    cv_.wait_for(lock, wait_time_, [this]{ return nproduced_ == 0; });
    nparticipants_ = nwaiting_;
    nproduced_ = nparticipants_;
    cv_.notify_all();
}

void Dispatcher::wait_for_data() {
    std::unique_lock lock(mutex_);
    ++nwaiting_;
    cv_.wait_for(lock, wait_time_, [this] { return nproduced_ > 0; });
    --nwaiting_;
}

void Dispatcher::consume() {
    std::scoped_lock lock(mutex_);
    if (nproduced_ > 0) {
        --nproduced_;
        if (nproduced_ == 0) {
            cv_.notify_all();
        }
    }
}

void Dispatcher::register_participant() {
    {
        std::scoped_lock lock{mutex_};
        ++nparticipants_;
    }
    cv_.notify_all();
}

void Dispatcher::deregister_participant() {
    {
        std::scoped_lock lock{mutex_};
        --nparticipants_;
    }
    cv_.notify_all();
}
