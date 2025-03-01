#pragma once
#include <Mlib/Iterator/Guarded_Iterable.hpp>
#include <Mlib/Threads/Recursive_Shared_Mutex.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <list>
#include <mutex>
#include <utility>

namespace Mlib {

template <class TMutex, class TBaseList>
class ThreadSafeGenericList {
public:
	using value_type = TBaseList::value_type;

	ThreadSafeGenericList() = default;
	~ThreadSafeGenericList() = default;
	template <class... TArgs>
	ThreadSafeGenericList(TArgs&&... args)
		: container_(std::forward<TArgs>(args)...)
	{}
	template <class TArg>
	ThreadSafeGenericList(std::initializer_list<TArg> lst)
		: container_(lst)
	{}
	template <class... TArgs>
	void emplace_back(TArgs&&... args) {
		container_.emplace_back(std::forward<TArgs>(args)...);
	}
	template <class TArgs>
	void push_back(TArgs&& arg) {
		container_.push_back(std::forward<TArgs>(arg));
	}
	GuardedIterable<typename TBaseList::iterator, std::scoped_lock<TMutex>> scoped() {
		return { mutex_, container_.begin(), container_.end() };
	}
	GuardedIterable<typename TBaseList::iterator, std::shared_lock<TMutex>> shared() {
		return { mutex_, container_.begin(), container_.end() };
	}
	GuardedIterable<typename TBaseList::const_iterator, std::scoped_lock<TMutex>> scoped() const {
		return { mutex_, container_.begin(), container_.end() };
	}
	GuardedIterable<typename TBaseList::const_iterator, std::shared_lock<TMutex>> shared() const {
		return { mutex_, container_.begin(), container_.end() };
	}
private:
	mutable TMutex mutex_;
	TBaseList container_;
};

template <class TData>
using ThreadSafeList = ThreadSafeGenericList<SafeAtomicRecursiveSharedMutex, std::list<TData>>;

}
