#include "Misc.hpp"
#include <Mlib/Throw_Or_Abort.hpp>
#include <iostream>
#include <map>
#include <mutex>

using namespace Mlib;
using namespace TemplateRegex;

static void iterate_replacements(
    const std::string& replacements,
    const std::function<void(const std::string& key, const std::string& value)>& op)
{
    static const DECLARE_REGEX(re, "\\s+");
    for (auto it = Mlib::re::cregex_token_iterator(replacements, re);
        it != Mlib::re::cregex_token_iterator();
        ++it)
    {
        std::string s = *it;
        if (s.empty()) {
            continue;
        }
        size_t pos = s.find(':');
        if (pos == std::string::npos) {
            THROW_OR_ABORT("Could not match replacement \"" + s + "\" from \"" + replacements + '"');
        }
        op(s.substr(0, pos), s.substr(pos + 1));
    }
}

// "(?:\\$(\\$[$\\w/{}]+|\\w+)-?|([^$]+))"
static const auto ddw = seq(chr('$'), plus(par(chr('$'), word, chr('/'), chr('{'), chr('}')))); // \\$[$\\w/{}]+
static const auto left = seq(group(par(ddw, plus(word))), opt(chr('-')));
static const auto nd = CharPredicate{[](char c){ return (c != '$'); }};
static const auto right = group(plus(nd));
static const auto s0 = par(seq(chr('$'), left), right);

std::string Mlib::substitute_dollar(const std::string_view& str, const std::function<std::string(std::string_view)>& replacements) {
    auto rem = str;
    std::string new_line;
    while (!rem.empty()) {
        if ((rem.length() >= 3) && (rem[0] == '$') && (rem[1] == '{') && (rem[rem.length() - 1] == '}')) {
            return replacements(substitute_dollar(rem.substr(2, rem.length() - 3), replacements));
        }
        bool found = false;
        rem = find_all_templated(rem, s0, [&](const TemplateRegex::SMatch<3>& v) {
            if (v[1].matched()) {
                new_line += replacements(v[1].str());
            } else if (v[2].matched()) {
                new_line += v[2].str();
            } else {
                verbose_abort("Internal error parsing string \"" + std::string(str) + "\". Remainder: \"" + std::string(rem) + '"');
            }
            found = true;
        });
        if (!found) {
            THROW_OR_ABORT("Could not parse \"" + std::string(str) + "\". Remainder: \"" + std::string(rem) + '"');
        }
    }
    return new_line;
}

std::string Mlib::substitute(const std::string& str, const std::map<std::string, std::string>& replacements) {
    // lerr() << "in: " << str;
    // for (const auto& e : replacements) {
    //     lerr() << e.first << " -> " << e.second;
    // }
    std::string new_line;
    // 1. Substitute expressions with and without default value, assigning default values.
    // 2. Substitute simple expressions.
    //                                  1      2      3        4         5           6      7       8          9
    static const DECLARE_REGEX(s0, "(?:(\\w+):(!)?(?:(\\w+)-)?(\\w*)(?:=(\\S*))?|(?:([-!]?)(\\w+))|([^-!\\w]+)|(.))");
    find_all(str, s0, [&new_line, &replacements](const Mlib::re::cmatch& v) {
        // if (v[1].str() == "-decimate") {
        //     lerr() << "x";
        // }
        if (v[1].matched) {
            auto it = replacements.find(v[4].str());
            if (it != replacements.end()) {
                if (v[2].matched) {
                    if (v[3].matched) {
                        THROW_OR_ABORT("Found concatenation despite negation");
                    }
                    if (it->second.empty()) {
                        new_line += v[1].str() + ":#";
                    } else if (it->second == "#") {
                        new_line += v[1].str() + ':';
                    } else {
                        THROW_OR_ABORT("Could not negate \"" + it->second + '"');
                    }
                } else {
                    new_line += v[1].str() + ':' + v[3].str() + it->second;
                }
            }
            else {
                if (v[2].matched) {
                    THROW_OR_ABORT("Could not find variable \"" + v[4].str() + "\" despite negation");
                }
                if (v[3].matched) {
                    THROW_OR_ABORT("Could not find variable \"" + v[4].str() + "\" despite concatenation");
                }
                if (v[5].matched) {
                    // If a default argument is given and the variable did not match,
                    // apply the default argument.
                    new_line += v[1].str() + ':' + v[5].str();
                }
                else {
                    // If no default argument is given and the variable did not match,
                    // do not modify anything.
                    new_line += v[1].str() + ':' + v[4].str();
                }
            }
        }
        else if (v[7].matched) {
            auto it = replacements.find(v[7].str());
            if (it != replacements.end()) {
                if (v[6] == "!") {
                    if (it->second.empty()) {
                        new_line += "#";
                    } else if (it->second != "#") {
                        THROW_OR_ABORT("Could not negate \"" + it->second + '"');
                    }
                } else {
                    new_line += it->second;
                }
            }
            else {
                new_line += v[6].str() + v[7].str();
            }
        }
        // This group is a performance optimization to avoid the single-character case 9.
        else if (v[8].matched) {
            new_line += v[8].str();
        }
        else {
            new_line += v[9].str();
        }
        });
    return new_line;
}

std::map<std::string, std::string> Mlib::replacements_to_map(const std::string& replacements) {
    std::map<std::string, std::string> repls;
    iterate_replacements(replacements, [&repls](const std::string& key, const std::string& value){
        repls[key] = value;
    });
    return repls;
}

void Mlib::find_all(
    const std::string& str,
    const Mlib::re::cregex& pattern,
    const std::function<void(const Mlib::re::cmatch&)>& f)
{
    Mlib::re::cmatch match;
    const char* search_start = str.data();
    const char* search_end = str.data() + str.size();
    while (Mlib::re::regex_search(search_start, search_end, match, pattern)) {
        f(match);
        search_start = match.suffix().first;
    }
}

std::list<std::pair<std::string, std::string>> Mlib::find_all_name_values(
    const std::string& str,
    const std::string& name_pattern,
    const std::string& value_pattern)
{
    std::list<std::pair<std::string, std::string>> res;
    DECLARE_REGEX(regex, "\\s*name=(" + name_pattern + ") value=(" + value_pattern + ")|(.+)");
    find_all(str, regex, [&](const Mlib::re::cmatch& m){
        if (!m[3].str().empty()) {
            THROW_OR_ABORT("Could not parse \"" + str + "\", unknown element: \"" + m[3].str() + '"');
        }
        res.emplace_back(m[1].str(), m[2].str());
    });
    return res;
}

SubstitutionMap::SubstitutionMap() = default;

SubstitutionMap::SubstitutionMap(const std::map<std::string, std::string>& s)
: s_{s}
{}

SubstitutionMap::SubstitutionMap(std::map<std::string, std::string>&& s)
: s_{std::move(s)}
{}

SubstitutionMap::SubstitutionMap(const SubstitutionMap& other) {
    std::shared_lock other_lock{other.mutex_};
    s_ = other.s_;
}

SubstitutionMap& SubstitutionMap::operator = (SubstitutionMap&& other) {
    std::unique_lock other_lock{other.mutex_};
    s_ = std::move(other.s_);
    return *this;
}

std::string SubstitutionMap::substitute(const std::string& t) const {
    std::shared_lock lock{mutex_};
    return Mlib::substitute(t, s_);
}

void SubstitutionMap::merge(const SubstitutionMap& other, const std::string& prefix) {
    std::scoped_lock lock{mutex_, other.mutex_};
    for (const auto& [k, v] : other.s_) {
        s_[prefix + k] = v;
    }
}

bool SubstitutionMap::insert(const std::string& key, const std::string& value) {
    std::scoped_lock lock{mutex_};
    return s_.insert({ key, value }).second;
}

void SubstitutionMap::set(const std::string& key, const std::string& value) {
    std::scoped_lock lock{mutex_};
    s_[key] = value;
}

void SubstitutionMap::clear() {
    std::scoped_lock lock{mutex_};
    s_.clear();
}

const std::string& SubstitutionMap::get_value(const std::string& key) const {
    std::shared_lock lock{mutex_};
    auto it = s_.find(key);
    if (it == s_.end()) {
        THROW_OR_ABORT("Could not find key \"" + key + '"');
    }
    return it->second;
}

bool SubstitutionMap::get_bool(const std::string& key) const {
    auto& v = get_value(key);
    if (v.empty()) {
        return true;
    } else if (v == "#") {
        return false;
    } else {
        THROW_OR_ABORT("Could not interpret key \"" + key + "\" as bool. Value: \"" + v + '"');
    }
}

std::ostream& Mlib::operator << (std::ostream& ostr, const SubstitutionMap& s) {
    std::shared_lock lock{s.mutex_};
    for (const auto& [k, v] : s.s_) {
        ostr << k << " -> " << v << '\n';
    }
    return ostr;
}
