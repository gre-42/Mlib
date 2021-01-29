#pragma once
#include <Mlib/Regex_Select.hpp>
#include <functional>
#include <list>
#include <map>
#include <regex>
#include <string>

namespace Mlib {

static const std::string substitute_pattern = "(?:\\S+:\\S*)?(?:\\s+\\S+:\\S*)*";

class RegexSubstitutionCache {
public:
    const Mlib::regex& get0(const std::string& key) const;
    const Mlib::regex& get1(const std::string& key) const;
private:
    mutable std::map<std::string, Mlib::regex> c0_;
    mutable std::map<std::string, Mlib::regex> c1_;
};

std::string substitute(
    const std::string& str,
    const std::string& replacements,
    const RegexSubstitutionCache& rsc = RegexSubstitutionCache{});

std::string merge_replacements(const std::initializer_list<const std::string>& replacements);

void find_all(
    const std::string& str,
    const Mlib::regex& pattern,
    const std::function<void(const Mlib::re::smatch&)>& f);

std::list<std::pair<std::string, std::string>> find_all_name_values(
    const std::string& str,
    const std::string& name_pattern,
    const std::string& value_pattern);

class SubstitutionString {
public:
    SubstitutionString();
    explicit SubstitutionString(const std::string& s);
    std::string substitute(const std::string& t, const RegexSubstitutionCache& rsc) const;
    void merge(const SubstitutionString& other);
    void clear();
    explicit operator const std::string& () const;
private:
    std::string s_;
};

}
