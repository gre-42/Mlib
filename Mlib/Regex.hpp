#pragma once
#include <Mlib/Regex_Select.hpp>
#include <functional>
#include <iosfwd>
#include <list>
#include <map>
#include <shared_mutex>
#include <string>

namespace Mlib {

static const std::string substitute_pattern = "(?:\\S+:\\S*)?(?:\\s+\\S+:\\S*)*";

std::string substitute(
    const std::string& str,
    const std::map<std::string, std::string>& replacements);

std::map<std::string, std::string> replacements_to_map(const std::string& replacements);

void find_all(
    const std::string& str,
    const Mlib::regex& pattern,
    const std::function<void(const Mlib::re::smatch&)>& f);

std::list<std::pair<std::string, std::string>> find_all_name_values(
    const std::string& str,
    const std::string& name_pattern,
    const std::string& value_pattern);

class SubstitutionMap {
    friend std::ostream& operator << (std::ostream& ostr, const SubstitutionMap& s);
public:
    SubstitutionMap();
    SubstitutionMap(const SubstitutionMap& other);
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
    mutable std::shared_mutex mutex_;
};

class NotifyingSubstitutionMap {
public:
    NotifyingSubstitutionMap();
    void set_and_notify(const std::string& key, const std::string& value);
    void merge_and_notify(const SubstitutionMap& other);
    const std::string& get_value(const std::string& key) const;
    bool get_bool(const std::string& key) const;
    const SubstitutionMap& substitution_map() const;
    void add_observer(const std::function<void()>& func);
    void clear_observers();
private:
    SubstitutionMap substitution_map_;
    std::list<std::function<void()>> observers_;
    mutable std::shared_mutex mutex_;
};

class SubstitutionMapObserverGuard {
public:
    SubstitutionMapObserverGuard(NotifyingSubstitutionMap& nsm);
    ~SubstitutionMapObserverGuard();
private:
    NotifyingSubstitutionMap& nsm_;
};

std::ostream& operator << (std::ostream& ostr, const SubstitutionMap& s);

}
