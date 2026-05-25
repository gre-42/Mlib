#pragma once
#include <future>
#include <mutex>

namespace Mlib {

template <class T>
class ThreadSafePromise {
    ThreadSafePromise(const ThreadSafePromise&) = delete;
    ThreadSafePromise& operator=(const ThreadSafePromise&) = delete;
public:
    ThreadSafePromise()
        : shared_future_(promise_.get_future().share())
        , fulfilled_(false)
    {}
    decltype(auto) get() const {
        return shared_future_.get();
    }
    template <class... TArgs>
    void set(TArgs&&... v) {
        std::scoped_lock lock{mutex_};
        if (fulfilled_) {
            return; 
        }
        promise_.set_value(std::forward<TArgs>(v)...);
        fulfilled_ = true;
    }
    bool fulfilled() const {
        std::scoped_lock lock{mutex_};
        return fulfilled_;
    }
private:
    std::promise<T> promise_;
    std::shared_future<T> shared_future_;
    bool fulfilled_;
    mutable std::mutex mutex_;
};

}
