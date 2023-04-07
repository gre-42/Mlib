#pragma once
#include <Mlib/Json.hpp>
#include <set>
#include <string>

#define BEGIN_ARGUMENT_LIST                                                   \
    static std::set<std::string> options;                                    \
    namespace {                                                              \
        struct Option: public std::string {                                  \
            Option(const char* c): std::string{c} {options.insert(*this);}   \
        };                                                                   \
    }

#define DECLARE_ARGUMENT_LIST(a) static const Option a(#a);
