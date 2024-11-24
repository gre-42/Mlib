#pragma once
#include <Mlib/Os/Os.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <array>
#include <string_view>
#include <vector>

namespace Mlib {

namespace TemplateRegex {

class SMatchGroup {
public:
    SMatchGroup() = default;
    SMatchGroup(SMatchGroup&& other) = default;
    SMatchGroup(bool matched, std::string_view str);
    ~SMatchGroup();
    bool matched;
    const std::string_view& str() const {
        return str_;
    }
    std::string_view str_;
};

template <size_t ngroups>
class SMatch {
public:
    SMatch() = default;
    ~SMatch() = default;
    const SMatchGroup& operator [] (size_t i) const {
        if (i >= matches_.size()) {
            verbose_abort("Match index out of bounds");
        }
        return matches_[i];
    }
    inline const std::string_view suffix() const {
        return suffix_;
    }

    std::string_view suffix_;
    std::array<SMatchGroup, ngroups> matches_;
};

class String {
public:
    static const size_t ngroups = 0;
    explicit String(std::string_view value);
    template <size_t mngroups>
    bool match(const std::string_view& line, SMatch<mngroups>& match, size_t match_index) const {
        if (line.starts_with(value_)) {
            match.suffix_ = line.substr(value_.length());
            return true;
        } else {
            return false;
        }
    }
private:
    std::string_view value_;
};

inline String str(std::string_view value) {
    return String(value);
}

template <class Inner>
class Group {
    template <class Inner2>
    friend Group<Inner2> group(Inner2 inner);
public:
    static const size_t ngroups = Inner::ngroups + 1;
    template <size_t mngroups>
    bool match(const std::string_view& line, SMatch<mngroups>& match, size_t match_index) const {
        bool imatched = inner_.match(line, match, match_index + 1);
        auto& imatch = const_cast<SMatchGroup&>(match[match_index]);
        imatch.matched = imatched;
        if (imatched) {
            imatch.str_ = line.substr(0, line.length() - match.suffix().length());
        } else {
            imatch.str_ = "";
        }
        return imatched;
    }
private:
    Group(Inner inner, int)
        : inner_{ std::move(inner) }
    {}
    Inner inner_;
};

template <class Inner>
inline Group<Inner> group(Inner inner) {
    return Group<Inner>{std::move(inner), 42};
}

template <class Inner>
class Repeat {
public:
    static const size_t ngroups = Inner::ngroups;
    Repeat(Inner inner, size_t min_repetitions, size_t max_repetitions)
    : inner_{std::move(inner)},
      min_repetitions_{min_repetitions},
      max_repetitions_{max_repetitions}
    {}
    template <size_t mngroups>
    bool match(const std::string_view& line, SMatch<mngroups>& match, size_t match_index) const {
        auto cline = line;
        for (size_t i = 0; i < max_repetitions_; ++i) {
            bool res = inner_.match(cline, match, match_index);
            if (res) {
                cline = match.suffix();
            } else {
                if (i < min_repetitions_) {
                    return false;
                } else {
                    break;
                }
            }
        }
        match.suffix_ = cline;
        return true;
    }
private:
    Inner inner_;
    size_t min_repetitions_;
    size_t max_repetitions_;
};

template <class Inner>
inline Repeat<Inner> plus(Inner inner) {
    return Repeat<Inner>{std::move(inner), 1, SIZE_MAX};
}

template <class Inner>
inline Repeat<Inner> star(Inner inner) {
    return Repeat<Inner>{std::move(inner), 0, SIZE_MAX};
}


template <class Inner>
inline Repeat<Inner> opt(Inner inner) {
    return Repeat<Inner>{std::move(inner), 0, 1};
}

template <class First, class Second>
class Sequence {
public:
    static const size_t ngroups = First::ngroups + Second::ngroups;
    Sequence(First first, Second second)
        : first_{ first }
        , second_{ second }
    {}
    template <size_t tngroups>
    bool match(const std::string_view& line, SMatch<tngroups>& match, size_t match_index) const
    {
        if (!first_.match(line, match, match_index)) {
            return false;
        }
        return second_.match(match.suffix(), match, match_index + first_.ngroups);
    }
private:
    First first_;
    Second second_;
};

template <class First, class Second>
class Parallel {
public:
    static const size_t ngroups = First::ngroups + Second::ngroups;
    Parallel(First first, Second second)
        : first_{ first }
        , second_{ second }
    {}
    template <size_t mngroups>
    bool match(const std::string_view& line, SMatch<mngroups>& match, size_t match_index) const
    {
        if (first_.match(line, match, match_index)) {
            return true;
        }
        return second_.match(line, match, match_index + first_.ngroups);
    }
private:
    First first_;
    Second second_;
};

template <class E0, class E1>
inline auto seq(E0&& e0, E1&& e1) {
    return Sequence{std::forward<E0>(e0), std::forward<E1>(e1)};
}

template<class E0, class... ERight>
inline auto seq(E0&& e0, ERight... eright) {
    return seq(std::forward<E0>(e0), seq(std::forward<ERight>(eright)...));
}

template <class E0, class E1>
inline auto par(E0&& e0, E1&& e1) {
    return Parallel{std::forward<E0>(e0), std::forward<E1>(e1)};
}

template<class E0, class... ERight>
inline auto par(E0&& e0, ERight... eright) {
    return par(std::forward<E0>(e0), par(std::forward<ERight>(eright)...));
}

class EndOfString {
public:
    static const size_t ngroups = 0;
    template <size_t mngroups>
    inline bool match(const std::string_view& line, SMatch<mngroups>& match, size_t match_index) const {
        if (line.empty()) {
            match.suffix_ = "";
            return true;
        } else {
            return false;
        }
    }
};

static const EndOfString eof;

template <class TPredicate>
class CharPredicate {
public:
    static const size_t ngroups = 0;
    explicit CharPredicate(TPredicate predicate)
        : predicate_{ std::move(predicate) }
    {}
    template <size_t ngroups>
    bool match(const std::string_view& line, SMatch<ngroups>& match, size_t match_index) const {
        if (line.empty()) {
            return false;
        }
        if (predicate_(line[0])) {
            match.suffix_ = line.substr(1);
            return true;
        } else {
            return false;
        }
    }
private:
    TPredicate predicate_;
};

// template <class TPredicate>
// class Char2Predicate {
// public:
//     explicit Char2Predicate(TPredicate predicate)
//     : predicate_{std::move(predicate)}
//     {}
//     MatchResult match(const std::string_view& line, SMatch& match, size_t match_index) const {
//         if (line.length() < 2) {
//             return MatchResult{.success = false};
//         }
//         if (predicate_(line[0], line[1])) {
//             return MatchResult{
//                 .success = true,
//                 .remainder = line};
//         } else {
//             return MatchResult{.success = false};
//         }
//     }
//     size_t constexpr ngroups() const {
//         return 0;
//     }
// private:
//     TPredicate predicate_;
// };

bool is_word(char c);

inline auto chr(char c) {
    return CharPredicate{[c](char c1){ return (c1 == c); }};
}
static const auto space = CharPredicate{[](char c){return std::isspace(c);}};
static const auto no_space = CharPredicate{[](char c){return !std::isspace(c);}};
static const auto digit = CharPredicate{[](char c){return (c >= '0') && (c <= '9');}};
static const auto adot = CharPredicate{[](char){return true;}};
static const auto word = CharPredicate(is_word);
// static const auto bdry = Char2Predicate{[](char a, char b){return is_word(a) != is_word(b);}};

template <class TRegex>
bool regex_match(const std::string_view& line, SMatch<TRegex::ngroups + 1>& match, const TRegex& regex) {
    auto re0 = group(regex);
    for (auto& m : match.matches_) {
        m.matched = false;
        m.str_ = "";
    }
    return re0.match(line, match, 0);
}

}
}
