#pragma once
#include <Mlib/Os/Os.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <string_view>
#include <vector>

namespace Mlib {

namespace TemplateRegex {

class SMatchGroup {
    template <class Inner>
    friend class Group;
public:
    SMatchGroup() = default;
    SMatchGroup(SMatchGroup&& other) = default;
    SMatchGroup(bool matched, std::string_view str);
    ~SMatchGroup();
    bool matched;
    const std::string_view& str() const {
        return str_;
    }
private:
    std::string_view str_;
};

class SMatch {
    template <class TRegex>
    friend bool regex_match(const std::string_view& line, SMatch& match, const TRegex& regex);
    template <class Inner>
    friend class Group;
public:
    SMatch();
    ~SMatch();
    const SMatchGroup& operator [] (size_t i) const {
        return const_cast<SMatch*>(this)->get(i);
    }
private:
    void resize(size_t nitems)
    {
        matches_.resize(nitems);
    }
    void clear() {
        matches_.clear();
    }
    SMatchGroup& get(size_t i) {
        if (i >= matches_.size()) {
            verbose_abort("Match index out of bounds");
        }
        return matches_[i];
    }
    std::vector<SMatchGroup> matches_;
};

struct MatchResult {
    bool success;
    std::string_view remainder;
};

class String {
public:
    explicit String(std::string_view value);
    MatchResult match(const std::string_view& line, SMatch& match, size_t match_index) const;
    size_t constexpr ngroups() const {
        return 0;
    }
private:
    std::string_view value_;
};

String str(std::string_view value) {
    return String(std::move(value));
}

template <class Inner>
class Group {
    template <class Inner2>
    friend Group<Inner2> group(Inner2 inner);
public:
    MatchResult match(const std::string_view& line, SMatch& match, size_t match_index) const {
        MatchResult result = inner_.match(line, match, match_index + 1);
        auto& imatch = match.get(match_index);
        imatch.matched = result.success;
        if (result.success) {
            imatch.str_ = line.substr(0, line.length() - result.remainder.length());
        }
        return result;
    }
    size_t constexpr ngroups() const {
        return inner_.ngroups() + 1;
    }
private:
    Group(Inner inner, int)
    : inner_{std::move(inner)}
    {}
    Inner inner_;
};

template <class Inner>
Group<Inner> group(Inner inner) {
    return Group<Inner>{std::move(inner), 42};
}

template <class Inner>
class Repeat {
public:
    Repeat(Inner inner, size_t min_repetitions, size_t max_repetitions)
    : inner_{std::move(inner)},
      min_repetitions_{min_repetitions},
      max_repetitions_{max_repetitions}
    {}
    MatchResult match(const std::string_view& line, SMatch& match, size_t match_index) const {
        auto cline = line;
        for (size_t i = 0; i < max_repetitions_; ++i) {
            MatchResult res = inner_.match(cline, match, match_index);
            if (res.success) {
                cline = res.remainder;
            } else {
                if (i < min_repetitions_) {
                    return MatchResult{.success = false};
                } else {
                    return MatchResult{
                        .success = true,
                        .remainder = cline
                    };
                }
            }
        }
        return MatchResult{
            .success = true,
            .remainder = cline
        };
    }
    size_t constexpr ngroups() const {
        return inner_.ngroups();
    }
private:
    Inner inner_;
    size_t min_repetitions_;
    size_t max_repetitions_;
};

template <class Inner>
Repeat<Inner> plus(Inner inner) {
    return Repeat<Inner>{std::move(inner), 1, SIZE_MAX};
}

template <class Inner>
Repeat<Inner> star(Inner inner) {
    return Repeat<Inner>{std::move(inner), 0, SIZE_MAX};
}


template <class Inner>
Repeat<Inner> opt(Inner inner) {
    return Repeat<Inner>{std::move(inner), 0, 1};
}

template <class First, class Second>
class Sequence {
public:
    Sequence(First first, Second second)
    : first_{first},
      second_{second}
    {}
    MatchResult match(const std::string_view& line, SMatch& match, size_t match_index) const {
        auto res_first = first_.match(line, match, match_index);
        if (!res_first.success) {
            return MatchResult{.success = false};
        }
        return second_.match(res_first.remainder, match, match_index + first_.ngroups());
    }
    size_t constexpr ngroups() const {
        return first_.ngroups() + second_.ngroups();
    }
private:
    First first_;
    Second second_;
};

template <class E0, class E1>
auto seq(E0 e0, E1 e1) {
    return Sequence{e0, e1};
}

template <class E0, class E1, class E2>
auto seq(E0 e0, E1 e1, E2 e2) {
    return seq(seq(e0, e1), e2);
}

template <class E0, class E1, class E2, class E3>
auto seq(E0 e0, E1 e1, E2 e2, E3 e3) {
    return seq(seq(e0, e1, e2), e3);
}

template <class E0, class E1, class E2, class E3, class E4>
auto seq(E0 e0, E1 e1, E2 e2, E3 e3, E4 e4) {
    return seq(seq(e0, e1, e2, e3), e4);
}

template <class E0, class E1, class E2, class E3, class E4, class E5>
auto seq(E0 e0, E1 e1, E2 e2, E3 e3, E4 e4, E5 e5) {
    return seq(seq(e0, e1, e2, e3, e4), e5);
}

template <class E0, class E1, class E2, class E3, class E4, class E5, class E6>
auto seq(E0 e0, E1 e1, E2 e2, E3 e3, E4 e4, E5 e5, E6 e6) {
    return seq(seq(e0, e1, e2, e3, e4, e5), e6);
}

template <class E0, class E1, class E2, class E3, class E4, class E5, class E6, class E7>
auto seq(E0 e0, E1 e1, E2 e2, E3 e3, E4 e4, E5 e5, E6 e6, E7 e7) {
    return seq(seq(e0, e1, e2, e3, e4, e5, e6), e7);
}

template <class E0, class E1, class E2, class E3, class E4, class E5, class E6, class E7, class E8>
auto seq(E0 e0, E1 e1, E2 e2, E3 e3, E4 e4, E5 e5, E6 e6, E7 e7, E8 e8) {
    return seq(seq(e0, e1, e2, e3, e4, e5, e6, e7), e8);
}

template <class E0, class E1, class E2, class E3, class E4, class E5, class E6, class E7, class E8, class E9>
auto seq(E0 e0, E1 e1, E2 e2, E3 e3, E4 e4, E5 e5, E6 e6, E7 e7, E8 e8, E9 e9) {
    return seq(seq(e0, e1, e2, e3, e4, e5, e6, e7, e8), e9);
}

template <class E0, class E1, class E2, class E3, class E4, class E5, class E6, class E7, class E8, class E9, class E10>
auto seq(E0 e0, E1 e1, E2 e2, E3 e3, E4 e4, E5 e5, E6 e6, E7 e7, E8 e8, E9 e9, E10 e10) {
    return seq(seq(e0, e1, e2, e3, e4, e5, e6, e7, e8, e9), e10);
}

template <class E0, class E1, class E2, class E3, class E4, class E5, class E6, class E7, class E8, class E9, class E10, class E11>
auto seq(E0 e0, E1 e1, E2 e2, E3 e3, E4 e4, E5 e5, E6 e6, E7 e7, E8 e8, E9 e9, E10 e10, E11 e11) {
    return seq(seq(e0, e1, e2, e3, e4, e5, e6, e7, e8, e9, e10), e11);
}

template <class E0, class E1, class E2, class E3, class E4, class E5, class E6, class E7, class E8, class E9, class E10, class E11, class E12>
auto seq(E0 e0, E1 e1, E2 e2, E3 e3, E4 e4, E5 e5, E6 e6, E7 e7, E8 e8, E9 e9, E10 e10, E11 e11, E12 e12) {
    return seq(seq(e0, e1, e2, e3, e4, e5, e6, e7, e8, e9, e10, e11), e12);
}

template <class E0, class E1, class E2, class E3, class E4, class E5, class E6, class E7, class E8, class E9, class E10, class E11, class E12, class E13>
auto seq(E0 e0, E1 e1, E2 e2, E3 e3, E4 e4, E5 e5, E6 e6, E7 e7, E8 e8, E9 e9, E10 e10, E11 e11, E12 e12, E13 e13) {
    return seq(seq(e0, e1, e2, e3, e4, e5, e6, e7, e8, e9, e10, e11, e12), e13);
}

template <class E0, class E1, class E2, class E3, class E4, class E5, class E6, class E7, class E8, class E9, class E10, class E11, class E12, class E13, class E14>
auto seq(E0 e0, E1 e1, E2 e2, E3 e3, E4 e4, E5 e5, E6 e6, E7 e7, E8 e8, E9 e9, E10 e10, E11 e11, E12 e12, E13 e13, E14 e14) {
    return seq(seq(e0, e1, e2, e3, e4, e5, e6, e7, e8, e9, e10, e11, e12, e13), e14);
}

class EndOfString {
public:
    inline MatchResult match(const std::string_view& line, SMatch& match, size_t match_index) const {
        return MatchResult{.success = line.empty()};
    }
    size_t constexpr ngroups() const {
        return 0;
    }
};

static const EndOfString eof;

template <class TPredicate>
class CharPredicate {
public:
    explicit CharPredicate(TPredicate predicate)
    : predicate_{std::move(predicate)}
    {}
    MatchResult match(const std::string_view& line, SMatch& match, size_t match_index) const {
        if (line.empty()) {
            return MatchResult{.success = false};
        }
        if (predicate_(line[0])) {
            return MatchResult{
                .success = true,
                .remainder = line.substr(1)};
        } else {
            return MatchResult{.success = false};
        }
    }
    size_t constexpr ngroups() const {
        return 0;
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

static const auto space = CharPredicate{[](char c){return std::isspace(c);}};
static const auto no_space = CharPredicate{[](char c){return !std::isspace(c);}};
static const auto digit = CharPredicate{[](char c){return (c >= '0') && (c <= '9');}};
static const auto adot = CharPredicate{[](char){return true;}};
// static const auto bdry = Char2Predicate{[](char a, char b){return is_word(a) != is_word(b);}};

template <class TRegex>
bool regex_match(const std::string_view& line, SMatch& match, const TRegex& regex) {
    auto re0 = group(regex);
    match.clear();
    match.resize(re0.ngroups());
    for (auto& m : match.matches_) {
        m.matched = false;
    }
    return re0.match(line, match, 0).success;
}

}
}
