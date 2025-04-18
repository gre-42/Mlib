#pragma once

#ifdef _MSC_VER
#include <experimental/generator>
#elif defined(__clang__)
#include <generator.hpp>
#else
#include <generator>
#endif

namespace Mlib {

#ifdef _MSC_VER
template <class T>
using Generator = std::experimental::generator<T>;
#else
template <class T>
using Generator = std::generator<T>;
#endif

}
