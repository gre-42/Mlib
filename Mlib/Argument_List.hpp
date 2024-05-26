#pragma once
#include <set>
#include <string>

namespace Mlib {

struct Option: public std::string {
    inline Option(const char* c, std::set<std::string>& options)
        : std::string{c}
    {
        options.insert(*this);
    }
};

}

#define BEGIN_ARGUMENT_LIST static std::set<std::string> options
#define DECLARE_ARGUMENT(a) static const ::Mlib::Option a(#a, options)
