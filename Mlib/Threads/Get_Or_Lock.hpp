#pragma once
#include <memory>
#include <stdexcept>
#include <utility>

namespace Mlib {

template <class TMap, class TMutex>
class GetOrLock {
public:
    using TMapped = TMap::mapped_type;
    GetOrLock(TMap& map, TMutex& mutex)
        : map_{ map }
        , mutex_{ mutex }
    {}
    template <class TKey>
    TMapped* try_get(const TKey& key) {
        {
            std::shared_lock lock{mutex_};
            auto* res = map_.try_get(key);
            if (res != nullptr) {
                return res;
            }
        }
        lock();
        auto res = map_.try_get(key);
        if (res != nullptr) {
            lock_.reset();
        }
        return res;
    }
    template <class TKey, class... TArgs>
    TMapped& add(TKey key, TArgs&&... args) {
        if (lock_ == nullptr) {
            throw std::runtime_error("Emplace without previous lock");
        }
        return map_.add(std::forward<TKey>(key), std::forward<TArgs>(args)...);
    }
private:
    void lock() {
        if (lock_ != nullptr) {
            throw std::runtime_error("Lock already acquired");
        }
        lock_ = std::make_unique<std::scoped_lock<TMutex>>(mutex_);
    }
    std::unique_ptr<std::scoped_lock<TMutex>> lock_;
    TMap& map_;
    TMutex& mutex_;
};

}