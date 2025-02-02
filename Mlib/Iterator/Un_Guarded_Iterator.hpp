#pragma once
#include <utility>

namespace Mlib {

template<typename Iterable, typename TLock>
class UnGuardedIterator
{
private:
    static Iterable container();
    TLock& lock_;
    decltype(std::begin(container())) begin_;
    decltype(std::end(container())) end_;

    UnGuardedIterator(const UnGuardedIterator&) = delete;
    UnGuardedIterator& operator = (const UnGuardedIterator&) = delete;
public:
    struct end_type {};
    UnGuardedIterator(Iterable& iter, TLock& lock)
        : lock_{ lock }
        , begin_(std::begin(iter))
        , end_(std::end(iter))
    {
        lock_.unlock();
    }

    ~UnGuardedIterator() {
        lock_.lock();
    }

    bool operator != (const end_type&) const
    {
        return begin_ != end_;
    }

    void operator++()
    {
        lock_.lock();
        ++begin_;
        lock_.unlock();
    }

    decltype(*begin_) operator*() const {
        return *begin_;
    }
};

template<typename Iterable, typename TLock>
class UnGuardedIterable
{
private:
    Iterable& container_;
    TLock& lock_;

    UnGuardedIterable(const UnGuardedIterable&) = delete;
    UnGuardedIterable& operator = (const UnGuardedIterable&) = delete;
public:
    struct end_type {};
    UnGuardedIterable(Iterable& iter, TLock& lock)
        : container_{ iter }
        , lock_{ lock }
    {}

    UnGuardedIterator<Iterable, TLock> begin() const { return UnGuardedIterator<Iterable, TLock>{ container_, lock_ }; }
    UnGuardedIterator<Iterable, TLock>::end_type end() const { return {}; }
};

template<typename Iterable, typename TLock>
void un_guarded_iterator(Iterable&& iter, TLock& lock);

template<typename Iterable, typename TLock>
auto un_guarded_iterator(Iterable& iter, TLock& lock) -> UnGuardedIterable<Iterable&, TLock>
{
    return { iter, lock };
}

template<typename Iterable, typename TLock>
auto un_guarded_iterator(const Iterable& iter, TLock& lock) -> UnGuardedIterable<const Iterable&, TLock>
{
    return { iter, lock };
}

}
