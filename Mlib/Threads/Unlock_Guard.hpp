#pragma once
#include <optional>

namespace Mlib {

template <class TLock>
class UnlockGuard {
    UnlockGuard(const UnlockGuard&) = delete;
    UnlockGuard& operator = (const UnlockGuard&) = delete;
public:
	UnlockGuard(TLock& lock)
		: lock_{ lock }
	{
		lock_.unlock();
	}
	~UnlockGuard() {
		lock_.lock();
	}
private:
	TLock& lock_;
};

template <class TLock>
class OptionalUnlockGuard {
public:
	OptionalUnlockGuard(TLock& lock, bool enable) {
		if (enable) {
			lock_.emplace(lock);
		}
	}
	~OptionalUnlockGuard() = default;
private:
	std::optional<UnlockGuard<TLock>> lock_;
};

}
