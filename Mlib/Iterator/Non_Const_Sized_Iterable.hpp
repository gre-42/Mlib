#pragma once
#include <concepts>

namespace Mlib {

// From: https://stackoverflow.com/questions/60449592/how-do-you-define-a-c-concept-for-the-standard-library-containers
template <class T>
concept NonConstSizedIterable = requires(T a, const T b)
{
    { a.begin() } -> std::same_as<typename T::iterator>;
    { a.end() } -> std::same_as<typename T::iterator>;
    { b.size() } -> std::same_as<typename T::size_type>;
    { b.empty() } -> std::same_as<bool>;
};

}
