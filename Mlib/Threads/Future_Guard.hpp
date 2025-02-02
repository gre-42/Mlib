#pragma once
#include <Mlib/Threads/Termination_Manager.hpp>
#include <exception>
#include <future>

namespace Mlib {

class FutureGuard {
    FutureGuard(const FutureGuard&) = delete;
    FutureGuard& operator=(const FutureGuard&) = delete;
public:
    explicit FutureGuard(std::future<void>&& f)
    : f_{std::move(f)}
    {}
    FutureGuard& operator = (std::future<void>&& f) {
        if (f_.valid()) {
            THROW_OR_ABORT("FutureGuard already set");
        }
        f_ = std::move(f);
        return *this;
    }
    ~FutureGuard() {
        if (f_.valid()) {
            try {
                f_.get();
            } catch (...) {
                add_unhandled_exception(std::current_exception());
            }
        }
    }
private:
    std::future<void> f_;
};

}
