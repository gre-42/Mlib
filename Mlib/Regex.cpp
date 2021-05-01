#include "Regex.hpp"
#include <iostream>
#include <map>
#include <regex>

using namespace Mlib;

static void iterate_replacements(
    const std::string& replacements,
    const std::function<void(const std::string& key, const std::string& value)>& op)
{
    static const DECLARE_REGEX(re, "\\s+");
    static const DECLARE_REGEX(re2, "(\\S+):(\\S*)");
    for (auto it = Mlib::re::sregex_token_iterator(replacements.begin(), replacements.end(), re, -1, Mlib::re::regex_constants::match_not_null);
        it != Mlib::re::sregex_token_iterator();
        ++it)
    {
        std::string s = *it;
        if (s.empty()) {
            continue;
        }
        if (true) {
            size_t pos = s.find(':');
            if (pos == std::string::npos) {
                throw std::runtime_error("Could not match replacement \"" + s + "\" from \"" + replacements + '"');
            }
            op(s.substr(0, pos), s.substr(pos + 1));
        } else {
            Mlib::re::smatch match2;
            if (Mlib::re::regex_match(s, match2, re2)) {
                op(match2[1].str(), match2[2].str());
            } else {
                throw std::runtime_error("Could not match replacement \"" + s + "\" from \"" + replacements + '"');
            }
        }
    }
}

const Mlib::regex& RegexSubstitutionCache::get0(const std::string& key) const {
    auto it = c0_.find(key);
    if (it != c0_.end()) {
        return it->second;
    } else {
        c0_.insert({key, CONSTRUCT_REGEX(":" + key + "(?:=\\S*|\\b(?!:))")});
        return c0_.at(key);
    }
}

const Mlib::regex& RegexSubstitutionCache::get1(const std::string& key) const {
    auto it = c1_.find(key);
    if (it != c1_.end()) {
        return it->second;
    } else {
        c1_.insert({key, CONSTRUCT_REGEX("\\b" + key + "\\b(?!:)")});
        return c1_.at(key);
    }
}

std::string Mlib::substitute(const std::string& str, const std::map<std::string, std::string>& replacements, const RegexSubstitutionCache& rsc) {
    // std::cerr << "in: " << str << std::endl;
    // for (const auto& e : replacements) {
    //     std::cerr << e.first << " -> " << e.second << '\n';
    // }
    std::string new_line = "";
    // 1. Substitute expressions with and without default value, assigning default values.
    // 2. Substitute simple expressions.
    //                                  1         2        3         4           5   6       7          8
    static const DECLARE_REGEX(s0, "(?:(\\w+):(?:(\\w+)-)?(\\w*)(?:=(\\S*))?|(?:(-?)(\\w+))|([^-\\w]+)|(.))");
    find_all(str, s0, [&new_line, &replacements](const Mlib::re::smatch& v) {
        // if (v[1].str() == "-DECIMATE") {
        //     std::cerr << "x" << std::endl;
        // }
        if (v[1].matched) {
            auto it = replacements.find(v[3].str());
            if (it != replacements.end()) {
                new_line += v[1].str() + ':' + v[2].str() + it->second;
            }
            else {
                if (v[2].matched) {
                    throw std::runtime_error("Could not find variable \"" + v[3].str() + "\" despite concatenation");
                }
                if (v[4].matched) {
                    // If a default argument is given and the variable did not match,
                    // apply the default argument.
                    new_line += v[1].str() + ':' + v[4].str();
                }
                else {
                    // If no default argument is given and the variable did not match,
                    // do not modify anything.
                    new_line += v[1].str() + ':' + v[3].str();
                }
            }
        }
        else if (v[6].matched) {
            auto it = replacements.find(v[6].str());
            if (it != replacements.end()) {
                new_line += it->second;
            }
            else {
                new_line += v[5].str() + v[6].str();
            }
        }
        else if (v[7].matched) {
            new_line += v[7].str();
        }
        else {
            new_line += v[8].str();
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
    const Mlib::regex& pattern,
    const std::function<void(const Mlib::re::smatch&)>& f)
{
    Mlib::re::smatch match;
    std::string::const_iterator search_start( str.cbegin() );
    while (Mlib::re::regex_search(search_start, str.cend(), match, pattern)) {
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
    find_all(str, regex, [&](const Mlib::re::smatch& m){
        if (!m[3].str().empty()) {
            throw std::runtime_error("Could not parse \"" + str + "\", unknown element: \"" + m[3].str() + '"');
        }
        res.push_back({m[1].str(), m[2].str()});
    });
    return res;
}

SubstitutionMap::SubstitutionMap()
{}

SubstitutionMap::SubstitutionMap(const std::map<std::string, std::string>& s)
: s_{s}
{}

SubstitutionMap::SubstitutionMap(std::map<std::string, std::string>&& s)
: s_{s}
{}

std::string SubstitutionMap::substitute(const std::string& t, const RegexSubstitutionCache& rsc) const {
    return Mlib::substitute(t, s_, rsc);
}

void SubstitutionMap::merge(const SubstitutionMap& other) {
    for (const auto& e : other.s_) {
        s_[e.first] = e.second;
    }
}

void SubstitutionMap::clear() {
    s_.clear();
}

std::ostream& Mlib::operator << (std::ostream& ostr, const SubstitutionMap& s) {
    for (const auto& e : s.s_) {
        ostr << e.first << " -> " << e.second << '\n';
    }
    return ostr;
}
