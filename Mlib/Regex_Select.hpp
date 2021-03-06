#pragma once

#ifdef _MSC_VER
#include <boost/xpressive/xpressive_dynamic.hpp>

#define DECLARE_REGEX(name, value) decltype(boost::xpressive::sregex::compile(value)) name = boost::xpressive::sregex::compile(value)

namespace Mlib {
    typedef boost::xpressive::sregex regex;
    namespace re = boost::xpressive;
    inline boost::xpressive::sregex compile_regex(const std::string& s) {
        return boost::xpressive::sregex::compile(s);
    }
}

#else

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"

#include <regex>

#pragma GCC diagnostic pop

#define DECLARE_REGEX(name, value) std::regex name{value}

namespace Mlib {
    typedef std::regex regex;
    namespace re = std;
    inline std::regex compile_regex(const std::string& s) {
        return std::regex{ s };
    }
}

#endif
