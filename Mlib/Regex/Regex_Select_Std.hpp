#pragma once
#include <regex>
#include <string>

#define DECLARE_REGEX(name, value) decltype(std::regex{ value }) name = std::regex{ value }

namespace Mlib {
    namespace re {
        inline bool regex_search(const std::string& s, const std::regex& regex) {
            return std::regex_search(s.data(), s.data() + s.size(), regex);
        }
        inline bool regex_search(const char* begin, const char* end, const std::regex& regex) {
            return std::regex_search(begin, end, regex);
        }
        inline bool regex_search(const std::string& s, std::cmatch& match, const std::regex& regex) {
            return std::regex_search(s.data(), s.data() + s.size(), match, regex);
        }
        inline bool regex_search(const char* begin, const char* end, std::cmatch& match, const std::regex& regex) {
            return std::regex_search(begin, end, match, regex);
        }

        inline bool regex_match(const std::string& s, const std::regex& regex) {
            return std::regex_match(s.data(), s.data() + s.size(), regex);
        }
        inline bool regex_match(const char* begin, const char* end, const std::regex& regex) {
            return std::regex_match(begin, end, regex);
        }
        inline bool regex_match(const std::string& s, std::cmatch& match, const std::regex& regex) {
            return std::regex_match(s.data(), s.data() + s.size(), match, regex);
        }
        inline bool regex_match(const char* begin, const char* end, std::cmatch& match, const std::regex& regex) {
            return std::regex_match(begin, end, match, regex);
        }
        inline auto cregex_token_iterator(const std::string& s, const std::regex& regex) {
            return std::cregex_token_iterator(s.data(), s.data() + s.size(), regex, -1, std::regex_constants::match_not_null);
        }
        inline auto cregex_token_iterator() {
            return std::cregex_token_iterator();
        }
        using cregex = std::regex;
        using cmatch = std::cmatch;
    }
    inline std::regex compile_regex(const std::string& s) {
        return std::regex{ s };
    }
}
