#pragma once

namespace Mlib {

template <class TLock>
class UnlockGuard {
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

}
