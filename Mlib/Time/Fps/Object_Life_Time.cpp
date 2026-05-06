#include "Object_Life_Time.hpp"

using namespace Mlib;

ObjectLifeTime::ObjectLifeTime(std::function<void(std::chrono::steady_clock::duration)> func)
	: func_{ std::move(func) }
	, start_time_{ std::chrono::steady_clock::now() }
{}

ObjectLifeTime::~ObjectLifeTime() {
	func_(elapsed());
}

std::chrono::steady_clock::duration ObjectLifeTime::elapsed() const {
	return std::chrono::steady_clock::now() - start_time_;
}
