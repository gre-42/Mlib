#include "Template_Regex.hpp"

using namespace Mlib;
using namespace Mlib::TemplateRegex;

SMatchGroup::SMatchGroup(bool matched, std::string_view str)
: matched{matched},
    str_{std::move(str)}
{}

SMatchGroup::~SMatchGroup() = default;

String::String(std::string_view value)
    : value_{ std::move(value) }
{}

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
    if (c == '_') {
        return true;
    }
    return false;
}
