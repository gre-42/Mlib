#pragma once
#include <Mlib/Os/Os.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <array>
#include <cstdint>
#include <limits>
#include <string_view>
#include <vector>

namespace Mlib {

namespace TemplateRegex {

class SMatchGroup {
public:
    using Index = uint32_t;
    static const Index NO_MATCH = std::numeric_limits<Index>::max();
    static const Index END_MATCH = std::numeric_limits<Index>::max();
    SMatchGroup() = default;
    SMatchGroup(SMatchGroup&& other) = default;
    SMatchGroup(Index parallel_index, std::string_view str);
    ~SMatchGroup();
    Index parallel_index;
    bool matched() const {
        return parallel_index != NO_MATCH;
    }
    const std::string_view& str() const {
        return str_;
    }
    std::string_view str_;
};

template <SMatchGroup::Index ngroups>
class SMatch {
public:
    SMatch() = default;
    ~SMatch() = default;
    const SMatchGroup& operator [] (SMatchGroup::Index i) const {
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
    static const SMatchGroup::Index ngroups = 0;
    explicit String(std::string_view value);
    template <SMatchGroup::Index mngroups>
    SMatchGroup::Index match(const std::string_view& line, SMatch<mngroups>& match, SMatchGroup::Index match_index) const {
        if (line.starts_with(value_)) {
            match.suffix_ = line.substr(value_.length());
            return 0;
        } else {
            return SMatchGroup::NO_MATCH;
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
    static const SMatchGroup::Index ngroups = Inner::ngroups + 1;
    template <SMatchGroup::Index mngroups>
    SMatchGroup::Index match(const std::string_view& line, SMatch<mngroups>& match, SMatchGroup::Index match_index) const {
        SMatchGroup::Index iparallel_index = inner_.match(line, match, match_index + 1);
        auto& imatch = const_cast<SMatchGroup&>(match[match_index]);
        imatch.parallel_index = iparallel_index;
        if (iparallel_index != SMatchGroup::NO_MATCH) {
            imatch.str_ = line.substr(0, line.length() - match.suffix().length());
        } else {
            imatch.str_ = "";
        }
        return iparallel_index;
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
    static const SMatchGroup::Index ngroups = Inner::ngroups;
    Repeat(Inner inner, SMatchGroup::Index min_repetitions, SMatchGroup::Index max_repetitions)
        : inner_{std::move(inner)}
        , min_repetitions_{min_repetitions}
        , max_repetitions_{max_repetitions}
    {}
    template <SMatchGroup::Index mngroups>
    SMatchGroup::Index match(const std::string_view& line, SMatch<mngroups>& match, SMatchGroup::Index match_index) const {
        auto cline = line;
        for (SMatchGroup::Index i = 0; i < max_repetitions_; ++i) {
            SMatchGroup::Index res = inner_.match(cline, match, match_index);
            if (res != SMatchGroup::NO_MATCH) {
                cline = match.suffix();
            } else {
                if (i < min_repetitions_) {
                    return SMatchGroup::NO_MATCH;
                } else {
                    break;
                }
            }
        }
        match.suffix_ = cline;
        return 0;
    }
private:
    Inner inner_;
    SMatchGroup::Index min_repetitions_;
    SMatchGroup::Index max_repetitions_;
};

template <class Inner>
inline Repeat<Inner> plus(Inner inner) {
    return Repeat<Inner>{std::move(inner), 1, SMatchGroup::END_MATCH};
}

template <class Inner>
inline Repeat<Inner> star(Inner inner) {
    return Repeat<Inner>{std::move(inner), 0, SMatchGroup::END_MATCH};
}


template <class Inner>
inline Repeat<Inner> opt(Inner inner) {
    return Repeat<Inner>{std::move(inner), 0, 1};
}

template <class First, class Second>
class Sequence {
public:
    static const SMatchGroup::Index ngroups = First::ngroups + Second::ngroups;
    Sequence(First first, Second second)
        : first_{ first }
        , second_{ second }
    {}
    template <SMatchGroup::Index tngroups>
    SMatchGroup::Index match(const std::string_view& line, SMatch<tngroups>& match, SMatchGroup::Index match_index) const
    {
        if (first_.match(line, match, match_index) == SMatchGroup::NO_MATCH) {
            return SMatchGroup::NO_MATCH;
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
    static const SMatchGroup::Index ngroups = First::ngroups + Second::ngroups;
    Parallel(First first, Second second)
        : first_{ first }
        , second_{ second }
    {}
    template <SMatchGroup::Index mngroups>
    SMatchGroup::Index match(
        const std::string_view& line,
        SMatch<mngroups>& match,
        SMatchGroup::Index match_index) const
    {
        if (first_.match(line, match, match_index) != SMatchGroup::NO_MATCH) {
            return 0;
        }
        auto pi = second_.match(line, match, match_index + first_.ngroups);
        if (pi != SMatchGroup::NO_MATCH) {
            return pi + 1;
        }
        return SMatchGroup::NO_MATCH;
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
    static const SMatchGroup::Index ngroups = 0;
    template <SMatchGroup::Index mngroups>
    inline SMatchGroup::Index match(const std::string_view& line, SMatch<mngroups>& match, SMatchGroup::Index match_index) const {
        if (line.empty()) {
            match.suffix_ = "";
            return 0;
        } else {
            return SMatchGroup::NO_MATCH;
        }
    }
};

static const EndOfString eof;

template <class TPredicate>
class CharPredicate {
public:
    static const SMatchGroup::Index ngroups = 0;
    explicit CharPredicate(TPredicate predicate)
        : predicate_{ std::move(predicate) }
    {}
    template <SMatchGroup::Index ngroups>
    SMatchGroup::Index match(const std::string_view& line, SMatch<ngroups>& match, SMatchGroup::Index match_index) const {
        if (line.empty()) {
            return SMatchGroup::NO_MATCH;
        }
        if (predicate_(line[0])) {
            match.suffix_ = line.substr(1);
            return 0;
        } else {
            return SMatchGroup::NO_MATCH;
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
//     MatchResult match(const std::string_view& line, SMatch& match, SMatchGroup::Index match_index) const {
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
//     SMatchGroup::Index constexpr ngroups() const {
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
        m.parallel_index = SMatchGroup::NO_MATCH;
        m.str_ = "";
    }
    return re0.match(line, match, 0) != SMatchGroup::NO_MATCH;
}

}
}
