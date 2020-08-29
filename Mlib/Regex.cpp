#include "Regex.hpp"
#include <regex>

using namespace Mlib;

std::string Mlib::substitute(const std::string& str, const std::string& replacements) {
    std::string new_line = str;
    std::regex re{"\\s+"};
    std::string substitutions = replacements;
    for(auto it = std::sregex_token_iterator(substitutions.begin(), substitutions.end(), re, -1, std::regex_constants::match_not_null);
        it != std::sregex_token_iterator();
        ++it)
    {
        std::string s = *it;
        if (s.empty()) {
            continue;
        }
        std::smatch match2;
        if (std::regex_match(s, match2, std::regex("(\\S+):(\\S*)"))) {
            try {
                new_line = std::regex_replace(new_line, std::regex{match2[1].str()}, match2[2].str());
            } catch (const std::regex_error&) {
                throw std::runtime_error("Error in regex " + match2[1].str());
            }
        } else {
            throw std::runtime_error("Could not match replacement \"" + s + "\" from \"" + substitutions + '"');
        }
    }
    return new_line;
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
