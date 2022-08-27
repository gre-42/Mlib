#pragma once
#include <Mlib/Threads/Termination_Manager.hpp>
#include <exception>
#include <future>

namespace Mlib {

class FutureGuard {
public:
    explicit FutureGuard(std::future<void>&& f)
    : f_{std::move(f)}
    {}
    ~FutureGuard() {
        if (f_.valid()) {
            try {
                f_.get();
            } catch (const std::exception&) {
                add_unhandled_exception(std::current_exception());
            }
        }
    }
private:
    std::future<void> f_;
};

}
