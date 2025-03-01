#pragma once
#include <Mlib/Regex/Regex_Select.hpp>
#include <Mlib/Regex/Template_Regex.hpp>
#include <Mlib/Threads/Recursive_Shared_Mutex.hpp>
#include <functional>
#include <iosfwd>
#include <list>
#include <map>
#include <string>
#include <string_view>

namespace Mlib {

static const std::string substitute_pattern = "(?:\\S+:\\S*)?(?:\\s+\\S+:\\S*)*";

std::string substitute(
    const std::string& str,
    const std::map<std::string, std::string>& replacements);

std::string substitute_dollar(
    const std::string_view& str,
    const std::function<std::string(std::string_view)>& replacements);

std::map<std::string, std::string> replacements_to_map(const std::string& replacements);

void find_all(
    const std::string& str,
    const Mlib::regex& pattern,
    const std::function<void(const Mlib::re::smatch&)>& f);

template <class TRegex>
void find_all_templated(
    const std::string_view& str,
    const TRegex& pattern,
    const std::function<void(const TemplateRegex::SMatch<TRegex::ngroups + 1>&)>& f)
{
    TemplateRegex::SMatch<TRegex::ngroups + 1> match;
    std::string_view search_start = str;
    while (TemplateRegex::regex_match(search_start, match, pattern)) {
        f(match);
        search_start = search_start.substr(search_start.length() - match.suffix().length());
    }
}

std::list<std::pair<std::string, std::string>> find_all_name_values(
    const std::string& str,
    const std::string& name_pattern,
    const std::string& value_pattern);

class SubstitutionMap {
    friend std::ostream& operator << (std::ostream& ostr, const SubstitutionMap& s);
public:
    SubstitutionMap();
    SubstitutionMap(const SubstitutionMap& other);
    SubstitutionMap& operator = (SubstitutionMap&& other);
    explicit SubstitutionMap(const std::map<std::string, std::string>& s);
    explicit SubstitutionMap(std::map<std::string, std::string>&& s);
    std::string substitute(const std::string& t) const;
    void merge(const SubstitutionMap& other, const std::string& prefix = "");
    bool insert(const std::string& key, const std::string& value);
    void set(const std::string& key, const std::string& value);
    void clear();
    const std::string& get_value(const std::string& key) const;
    bool get_bool(const std::string& key) const;
private:
    std::map<std::string, std::string> s_;
    mutable SafeAtomicRecursiveSharedMutex mutex_;
};

std::ostream& operator << (std::ostream& ostr, const SubstitutionMap& s);

}
