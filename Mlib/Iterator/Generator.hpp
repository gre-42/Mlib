#pragma once
#include <version> // Essential for feature-test macros

#if defined(__cpp_lib_generator)
    #include <generator>
    #define MLIB_GENERATOR_NS std
#elif __has_include(<experimental/generator>)
    #include <experimental/generator>
    #define MLIB_GENERATOR_NS std::experimental
#else
    #include <generator.hpp>
    #define MLIB_GENERATOR_NS std 
#endif

namespace Mlib {
    template <class T>
    using Generator = MLIB_GENERATOR_NS::generator<T>;
}
