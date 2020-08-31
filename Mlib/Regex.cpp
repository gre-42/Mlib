#include "Regex.hpp"
#include <map>
#include <regex>

using namespace Mlib;

static void iterate_replacements(
    const std::string& replacements,
    const std::function<void(const std::string& key, const std::string& value)>& op)
{
    std::regex re{"\\s+"};
    for(auto it = std::sregex_token_iterator(replacements.begin(), replacements.end(), re, -1, std::regex_constants::match_not_null);
        it != std::sregex_token_iterator();
        ++it)
    {
        std::string s = *it;
        if (s.empty()) {
            continue;
        }
        std::smatch match2;
        if (std::regex_match(s, match2, std::regex("(\\S+):(\\S*)"))) {
            op(match2[1].str(), match2[2].str());
        } else {
            throw std::runtime_error("Could not match replacement \"" + s + "\" from \"" + replacements + '"');
        }
    }
}

std::string Mlib::substitute(const std::string& str, const std::string& replacements) {
    std::string new_line = str;
    iterate_replacements(replacements, [&new_line](const std::string& key, const std::string& value){
        try {
            new_line = std::regex_replace(new_line, std::regex{"\\b" + key + "\\b(?!:)"}, value);
        } catch (const std::regex_error&) {
            throw std::runtime_error("Error in regex " + key);
        }
    });
    return new_line;
}

std::string Mlib::merge_replacements(const std::initializer_list<const std::string>& replacements) {
    std::map<std::string, std::string> repls;
    for(const auto& r : replacements) {
        iterate_replacements(r, [&repls](const std::string& key, const std::string& value){
            repls[key] = value;
        });
    }
    std::string res;
    for(const auto& p : repls) {
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
    const std::string& value_pattern)
{
    std::list<std::pair<std::string, std::string>> res;
    findall(str, std::regex{"(?:\\r?\\n|\\s)*name=([\\w+-. ]+) value=(" + value_pattern + ")|(.+)"}, [&](const std::smatch& m){
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

std::string SubstitutionString::substitute(const std::string& t) const {
    return Mlib::substitute(t, s_);
}

void SubstitutionString::merge(const SubstitutionString& other) {
    s_ = merge_replacements({s_, other.s_});
}

void SubstitutionString::clear() {
    s_.clear();
}
