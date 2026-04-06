#pragma once
#include <set>
#include <string_view>

namespace Mlib {

struct Option: public std::string_view {
    inline Option(const char* c, std::set<std::string_view>& options)
        : std::string_view{ c }
    {
        options.insert(*this);
    }
};

}

#define BEGIN_ARGUMENT_LIST static std::set<std::string_view> options
#define DECLARE_ARGUMENT(a) static const ::Mlib::Option a(#a, options)
