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
    {
        std::unique_lock lock(mutex_);
        cv_.wait_for(lock, wait_time_, [this] { return (nproduced_ == 0) && (nwaiting_ == nparticipants_); });
        nparticipants_ = nwaiting_;
        nproduced_ = nparticipants_;
    }
    cv_.notify_all();
}

void Dispatcher::wait_for_data() {
    ++nwaiting_;
    cv_.notify_all();
    {
        std::unique_lock lock(mutex_);
        cv_.wait_for(lock, wait_time_, [this] { return nproduced_ > 0; });
    }
    --nwaiting_;
}

void Dispatcher::consume() {
    bool notify = false;
    {
        std::scoped_lock lock(mutex_);
        if (nproduced_ > 0) {
            --nproduced_;
            if (nproduced_ == 0) {
                notify = true;
            }
        }
    }
    if (notify) {
        cv_.notify_all();
    }
}
