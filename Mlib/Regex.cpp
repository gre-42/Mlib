#include "Regex.hpp"
#include <iostream>
#include <map>
#include <regex>

using namespace Mlib;

static void iterate_replacements(
    const std::string& replacements,
    const std::function<void(const std::string& key, const std::string& value)>& op)
{
    static const std::regex re{"\\s+"};
    static const std::regex re2{"(\\S+):(\\S*)"};
    for (auto it = std::sregex_token_iterator(replacements.begin(), replacements.end(), re, -1, std::regex_constants::match_not_null);
        it != std::sregex_token_iterator();
        ++it)
    {
        std::string s = *it;
        if (s.empty()) {
            continue;
        }
        std::smatch match2;
        if (std::regex_match(s, match2, re2)) {
            op(match2[1].str(), match2[2].str());
        } else {
            throw std::runtime_error("Could not match replacement \"" + s + "\" from \"" + replacements + '"');
        }
    }
}

const std::regex& RegexSubstitutionCache::get0(const std::string& key) const {
    auto it = c0_.find(key);
    if (it != c0_.end()) {
        return it->second;
    } else {
        c0_.insert({key, std::regex{":" + key + "(?:=\\S*|\\b(?!:))"}});
        return c0_.at(key);
    }
}

const std::regex& RegexSubstitutionCache::get1(const std::string& key) const {
    auto it = c1_.find(key);
    if (it != c1_.end()) {
        return it->second;
    } else {
        c1_.insert({key, std::regex{"\\b" + key + "\\b(?!:)"}});
        return c1_.at(key);
    }
}

std::string Mlib::substitute(const std::string& str, const std::string& replacements, const RegexSubstitutionCache& rsc) {
    std::string new_line = str;
    iterate_replacements(replacements, [&new_line, &rsc](const std::string& key, const std::string& value){
        try {
            // Substitute expressions with and without default value.
            new_line = std::move(std::regex_replace(new_line, rsc.get0(key), ':' + value));
            // Substitute simple expressions.
            new_line = std::move(std::regex_replace(new_line, rsc.get1(key), value));
        } catch (const std::regex_error&) {
            throw std::runtime_error("Error in regex " + key);
        }
    });
    // Assign default values to remainders.
    static const std::regex re{"(\\S+:)\\S+="};
    new_line = std::regex_replace(new_line, re, "$1");
    return new_line;
}

std::string Mlib::merge_replacements(const std::initializer_list<const std::string>& replacements) {
    std::map<std::string, std::string> repls;
    for (const auto& r : replacements) {
        iterate_replacements(r, [&repls](const std::string& key, const std::string& value){
            repls[key] = value;
        });
    }
    std::string res;
    for (const auto& p : repls) {
        res += ' ' + p.first + ':' + p.second;
    }
    return res;

}

void Mlib::findall(
    const std::string& str,
    const std::regex& pattern,
    const std::function<void(const std::smatch&)>& f)
{
    std::smatch match;
    std::string::const_iterator search_start( str.cbegin() );
    while (std::regex_search(search_start, str.cend(), match, pattern)) {
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
    findall(str, std::regex{"\\s*name=(" + name_pattern + ") value=(" + value_pattern + ")|(.+)"}, [&](const std::smatch& m){
        if (!m[3].str().empty()) {
            throw std::runtime_error("Could not parse \"" + str + "\", unknown element: \"" + m[3].str() + '"');
        }
        res.push_back({m[1].str(), m[2].str()});
    });
    return res;
}

SubstitutionString::SubstitutionString()
{}

SubstitutionString::SubstitutionString(const std::string& s)
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

SubstitutionString::operator const std::string& () const {
    return s_;
}
