#pragma once
#include <boost/xpressive/xpressive_dynamic.hpp>
#include <string>

#define DECLARE_REGEX(name, value) decltype(boost::xpressive::cregex::compile(value)) name = boost::xpressive::cregex::compile(value)

namespace Mlib {
    namespace re {
        inline bool regex_search(const std::string& s, const boost::xpressive::cregex& regex) {
            return boost::xpressive::regex_search(s.data(), s.data() + s.size(), regex);
        }
        inline bool regex_search(const char* begin, const char* end, const boost::xpressive::cregex& regex) {
            return boost::xpressive::regex_search(begin, end, regex);
        }
        inline bool regex_search(const std::string& s, boost::xpressive::cmatch& match, const boost::xpressive::cregex& regex) {
            return boost::xpressive::regex_search(s.data(), s.data() + s.size(), match, regex);
        }
        inline bool regex_search(const char* begin, const char* end, boost::xpressive::cmatch& match, const boost::xpressive::cregex& regex) {
            return boost::xpressive::regex_search(begin, end, match, regex);
        }

        inline bool regex_match(const std::string& s, const boost::xpressive::cregex& regex) {
            return boost::xpressive::regex_match(s.data(), s.data() + s.size(), regex);
        }
        inline bool regex_match(const char* begin, const char* end, const boost::xpressive::cregex& regex) {
            return boost::xpressive::regex_match(begin, end, regex);
        }
        inline bool regex_match(const std::string& s, boost::xpressive::cmatch& match, const boost::xpressive::cregex& regex) {
            return boost::xpressive::regex_match(s.data(), s.data() + s.size(), match, regex);
        }
        inline bool regex_match(const char* begin, const char* end, boost::xpressive::cmatch& match, const boost::xpressive::cregex& regex) {
            return boost::xpressive::regex_match(begin, end, match, regex);
        }
        inline auto cregex_token_iterator(const std::string& s, const boost::xpressive::cregex& regex) {
            return boost::xpressive::cregex_token_iterator(s.data(), s.data() + s.size(), regex, -1, boost::xpressive::regex_constants::match_not_null);
        }
        inline auto cregex_token_iterator() {
            return boost::xpressive::cregex_token_iterator();
        }
        using cregex = boost::xpressive::cregex;
        using cmatch = boost::xpressive::cmatch;
    }
    inline boost::xpressive::cregex compile_regex(const std::string& s) {
        return boost::xpressive::cregex::compile(s);
    }
}
