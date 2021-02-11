#pragma once
#include <Mlib/Regex_Select.hpp>
#include <functional>
#include <iosfwd>
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
    const std::map<std::string, std::string>& replacements,
    const RegexSubstitutionCache& rsc = RegexSubstitutionCache{});

std::map<std::string, std::string> replacements_to_map(const std::string& replacements);

std::map<std::string, std::string> merge_replacements(const std::initializer_list<std::map<std::string, std::string>>& replacements);

void find_all(
    const std::string& str,
    const Mlib::regex& pattern,
    const std::function<void(const Mlib::re::smatch&)>& f);

std::list<std::pair<std::string, std::string>> find_all_name_values(
    const std::string& str,
    const std::string& name_pattern,
    const std::string& value_pattern);

class SubstitutionString {
    friend std::ostream& operator << (std::ostream& ostr, const SubstitutionString& s);
public:
    SubstitutionString();
    explicit SubstitutionString(const std::map<std::string, std::string>& s);
    explicit SubstitutionString(std::map<std::string, std::string>&& s);
    std::string substitute(const std::string& t, const RegexSubstitutionCache& rsc) const;
    void merge(const SubstitutionString& other);
    void clear();
private:
    std::map<std::string, std::string> s_;
};

std::ostream& operator << (std::ostream& ostr, const SubstitutionString& s);

}
