#include "Template_Regex.hpp"

using namespace Mlib;
using namespace Mlib::TemplateRegex;

SMatchGroup::SMatchGroup(bool matched, std::string_view str)
: matched{matched},
    str_{std::move(str)}
{}

SMatchGroup::~SMatchGroup() = default;

SMatch::SMatch() = default;
SMatch::~SMatch() = default;

String::String(std::string_view value)
: value_{std::move(value)}
{}

MatchResult String::match(const std::string_view& line, SMatch& match, size_t match_index) const {
    bool success = line.starts_with(value_);
    return {
        .success = success,
        .remainder = success ? line.substr(value_.length()) : std::string_view()};
}

bool Mlib::TemplateRegex::is_word(char c) {
    if ((c >= '0') && (c <= '9')) {
        return true;
    }
    if ((c >= 'A') && (c <= 'Z')) {
        return true;
    }
    if ((c >= 'a') && (c <= 'z')) {
        return true;
    }
    return false;
}
