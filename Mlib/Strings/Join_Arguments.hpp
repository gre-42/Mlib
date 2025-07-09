#pragma once
#include <sstream>
#include <string>
#include <string_view>

namespace Mlib {

// From: https://stackoverflow.com/questions/73661571/create-string-according-to-the-parameter-pack-arguments-count
template<typename Arg>
std::string join_arguments(std::string_view delimiter, Arg&& arg)
{
    return (std::ostringstream() << arg).str();
}

// Handle multiple-argument string construction
template<typename First, typename ...Rest>
std::string join_arguments(std::string_view delimiter, First&& first, Rest&& ...rest)
{
    return (
        std::ostringstream() <<
        join_arguments(delimiter, std::forward<First>(first)) <<
        delimiter <<
        join_arguments(delimiter, std::forward<Rest>(rest)...)).str();
}

}
