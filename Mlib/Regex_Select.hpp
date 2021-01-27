#pragma once

#ifdef _MSC_VER
#include <boost/xpressive/xpressive_dynamic.hpp>

#define CONSTRUCT_REGEX(value) boost::xpressive::sregex::compile(value)
#define DECLARE_REGEX(name, value) auto name = boost::xpressive::sregex::compile(value)
#define REGEX_MATCH boost::xpressive::regex_match
#define REGEX_SEARCH boost::xpressive::regex_match
#define REGEX_REPLACE boost::xpressive::regex_replace

namespace Mlib {
    typedef boost::xpressive::sregex regex;
    typedef boost::xpressive::smatch smatch;
}

#else

#include <regex>

#define CONSTRUCT_REGEX(value) std::regex{value}
#define DECLARE_REGEX(name, value) std::regex name{value}
#define REGEX_MATCH std::regex_match
#define REGEX_SEARCH std::regex_search
#define REGEX_REPLACE std::regex_replace

namespace Mlib {
    typedef std::regex regex;
    typedef std::smatch smatch;
}

#endif
