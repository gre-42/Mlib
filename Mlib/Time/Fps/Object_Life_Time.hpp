#pragma once
#include <chrono>
#include <functional>

namespace Mlib {

class ObjectLifeTime {
public:
	explicit ObjectLifeTime(std::function<void(std::chrono::steady_clock::duration elapsed)> func);
	~ObjectLifeTime();
	std::chrono::steady_clock::duration elapsed() const;
private:
	std::function<void(std::chrono::steady_clock::duration elapsed)> func_;
	std::chrono::steady_clock::time_point start_time_;
};

}
