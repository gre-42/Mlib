#pragma once
#include <Mlib/Threads/Safe_Atomic_Shared_Mutex.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <mutex>
#include <shared_mutex>

namespace Mlib {

template <class TObject, class TLock>
class LockedObject {
public:
    template <class TMutex>
    LockedObject(TObject& o, TMutex& mutex)
        : o_{ o }
        , lock_{ mutex }
    {}
    template <class TMutex>
    LockedObject(TObject& o, TMutex& mutex, std::defer_lock_t)
        : o_{ o }
        , lock_{ mutex, std::defer_lock }
    {}
    TObject* operator -> () {
        return &o_;
    }
    TObject& operator * () {
        return o_;
    }
    template <class Rep, class Period>
    bool try_lock_for(const std::chrono::duration<Rep,Period>& duration) {
        return lock_.try_lock_for(duration);
    }
private:
    TObject& o_;
    TLock lock_;
};

template <class T>
class LockableObject {
public:
    using LockExclusive = LockedObject<T, std::unique_lock<SafeAtomicSharedMutex>>;
    using ConstLockExclusive = LockedObject<const T, std::unique_lock<SafeAtomicSharedMutex>>;
    using LockShared = LockedObject<T, std::shared_lock<SafeAtomicSharedMutex>>;
    using ConstLockShared = LockedObject<const T, std::shared_lock<SafeAtomicSharedMutex>>;
    template <class Rep, class Period>
    LockExclusive lock_exclusive_for(
        const std::chrono::duration<Rep, Period>& duration,
        const std::string& message)
    {
        LockExclusive result{ o_, mutex_, std::defer_lock };
        if (!result.try_lock_for(duration)) {
            THROW_OR_ABORT(message + " object is already locked");
        }
        return result;
    }
    template <class Rep, class Period>
    ConstLockExclusive lock_exclusive_for(
        const std::chrono::duration<Rep, Period>& duration,
        const std::string& message) const
    {
        ConstLockExclusive result{ o_, mutex_, std::defer_lock };
        if (!result.try_lock_for(duration)) {
            THROW_OR_ABORT(message + " object is already locked");
        }
        return result;
    }
    LockShared lock_shared() {
        return { o_, mutex_ };
    }
    ConstLockShared lock_shared() const {
        return { o_, mutex_ };
    }
private:
    T o_;
    mutable SafeAtomicSharedMutex mutex_;
};

}
