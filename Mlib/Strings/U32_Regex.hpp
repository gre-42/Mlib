#pragma once

#ifdef WITHOUT_ICU

#include <regex>
#include <Mlib/Strings/Encoding.hpp>

namespace Mlib {

using u32regex = std::regex;

template <class TString>
inline std::regex make_u32regex(const TString& pattern) {
    return std::regex(pattern);
}

inline bool u32regex_search(const std::string& s, const std::regex& re) {
    return std::regex_search(s, re);
}

inline decltype(auto) make_u32regex_token_iterator(
    const std::string& str,
    const std::regex& re,
    int submatch = 0)
{
    return std::regex_token_iterator(str.begin(), str.end(), re, submatch);
}

}

#else

#include <boost/regex/icu.hpp>

namespace Mlib {

using u32regex = boost::u32regex;

template <class TString>
inline boost::u32regex make_u32regex(const TString& pattern) {
    return boost::make_u32regex(pattern);
}

inline bool u32regex_search(const std::string& s, const boost::u32regex& re) {
    return boost::u32regex_search(s, re);
}

inline decltype(auto) make_u32regex_token_iterator(
    const std::u8string& str,
    const boost::u32regex& re,
    int submatch = 0)
{
    return boost::make_u32regex_token_iterator(str, re, submatch);
}

}

#endif
