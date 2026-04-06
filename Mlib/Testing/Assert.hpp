#pragma once
#include <stdexcept>

#define assert_true(x) if (!(x)) throw std::runtime_error("Assertion failed: " #x)
