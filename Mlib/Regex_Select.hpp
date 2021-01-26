#pragma once

#ifdef _MSC_VER
#include <boost/xpressive/xpressive_dynamic.hpp>

#define DECLARE_REGEX(name, value) auto name = boost::xpressive::sregex::compile(value)
#define REGEX_MATCH boost::xpressive::regex_match

namespace Mlib {
    typedef boost::xpressive::smatch smatch;
}

#else

#include <regex>

#define DECLARE_REGEX(name, value) std::regex name{value}
#define REGEX_MATCH std::regex_match

namespace Mlib {
    typedef std::smatch smatch;
}

#endif
