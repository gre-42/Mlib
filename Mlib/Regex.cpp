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
    // std::cerr << str << std::endl;
    // for (const auto& e : replacements) {
    //     std::cerr << e.first << " -> " << e.second << '\n';
    // }
    std::string new_line = str;
    for (const auto& e : replacements) {
        const auto& key = e.first;
        const auto& value = e.second;
        try {
            // Substitute expressions with and without default value.
            new_line = std::move(Mlib::re::regex_replace(new_line, rsc.get0(key), ':' + value));
            // Substitute simple expressions.
            new_line = std::move(Mlib::re::regex_replace(new_line, rsc.get1(key), value));
        } catch (const std::regex_error&) {
            throw std::runtime_error("Error in regex " + key);
        }
    }
    // Assign default values to remainders.
    static const DECLARE_REGEX(re, "(\\S+:)\\S+=");
    new_line = Mlib::re::regex_replace(new_line, re, "$1");
    return new_line;
}

std::map<std::string, std::string> Mlib::replacements_to_map(const std::string& replacements) {
    std::map<std::string, std::string> repls;
    iterate_replacements(replacements, [&repls](const std::string& key, const std::string& value){
        repls[key] = value;
    });
    return repls;
}

std::map<std::string, std::string> Mlib::merge_replacements(const std::initializer_list<std::map<std::string, std::string>>& replacements) {
    std::map<std::string, std::string> res;
    for (const auto& p : replacements) {
        for (const auto& e : p) {
            res[e.first] = e.second;
        }
    }
    return res;
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

SubstitutionString::SubstitutionString()
{}

SubstitutionString::SubstitutionString(const std::map<std::string, std::string>& s)
: s_{s}
{}

std::string SubstitutionString::substitute(const std::string& t, const RegexSubstitutionCache& rsc) const {
    return Mlib::substitute(t, s_, rsc);
}

void SubstitutionString::merge(const SubstitutionString& other) {
    s_ = merge_replacements({s_, other.s_});
}

void SubstitutionString::clear() {
    s_.clear();
}

std::ostream& Mlib::operator << (std::ostream& ostr, const SubstitutionString& s) {
    for (const auto& e : s.s_) {
        ostr << e.first << " -> " << e.second << '\n';
    }
    return ostr;
}
