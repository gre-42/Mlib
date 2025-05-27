#include "Usage_Counter.hpp"
#include <mutex>

using namespace Mlib;

CounterUser::CounterUser(const DanglingBaseClassRef<UsageCounter>& counter)
    : usage_counter_{ counter }
    , value_{ false }
{}

CounterUser::~CounterUser() {
    set(false);
}

void CounterUser::set(bool value) {
    std::scoped_lock lock{ mutex_ };
    if (value && !value_) {
        usage_counter_->increase();
    }
    if (!value && value_) {
        usage_counter_->decrease();
    }
    value_ = value;
}

UsageCounter::UsageCounter()
    : count_{ 0 }
{}

UsageCounter::~UsageCounter() = default;

void UsageCounter::increase() {
    ++count_;
}

void UsageCounter::decrease() {
    --count_;
}

size_t UsageCounter::count() const {
    return count_;
}
