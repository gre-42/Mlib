#pragma once
#include <concepts>

namespace Mlib {

// From: https://stackoverflow.com/questions/60449592/how-do-you-define-a-c-concept-for-the-standard-library-containers
template <class T>
concept SizedIterable = requires(T a, const T b)
{
    { a.begin() } -> std::same_as<typename T::iterator>;
    { a.end() } -> std::same_as<typename T::iterator>;
    { b.begin() } -> std::same_as<typename T::const_iterator>;
    { b.end() } -> std::same_as<typename T::const_iterator>;
    { b.size() } -> std::same_as<typename T::size_type>;
    { b.empty() } -> std::same_as<bool>;
};

}
