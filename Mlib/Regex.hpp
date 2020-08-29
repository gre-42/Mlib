#pragma once
#include <functional>
#include <regex>
#include <string>

namespace Mlib {

static const std::string substitute_pattern = "(?:(?:\\r?\\n|\\s)+\\S+:\\S*)*";

std::string substitute(
    const std::string& str,
    const std::string& replacements);

void findall(
    const std::string& str,
    const std::regex& pattern,
    const std::function<void(const std::smatch&)>& f);

}
