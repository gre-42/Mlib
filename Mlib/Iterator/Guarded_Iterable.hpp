#pragma once

namespace Mlib {

template <class TIterator, class TLockGuard>
class GuardedIterable {
    GuardedIterable(const GuardedIterable&) = delete;
    GuardedIterable& operator = (const GuardedIterable&) = delete;
public:
    template <class TMutex>
    GuardedIterable(TMutex& mutex, const TIterator& begin, const TIterator& end)
        : lock_{ mutex }
        , begin_{ begin }
        , end_{ end }
    {}
    TIterator begin() {
        return begin_;
    }
    TIterator end() {
        return end_;
    }
private:
    TLockGuard lock_;
    TIterator begin_;
    TIterator end_;
};

}
