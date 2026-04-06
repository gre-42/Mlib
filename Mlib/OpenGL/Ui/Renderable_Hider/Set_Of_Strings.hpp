#pragma once
#include <set>
#include <string>

namespace Mlib {

class SetOfStrings {
public:
    bool contains(const std::string& name) const;
    void try_insert(const std::string& name, size_t max_entries);
    inline bool empty() const {
        return strings_.empty();
    }
    inline decltype(auto) find(const auto& key) const {
        return strings_.find(key);
    }
    inline decltype(auto) begin() const {
        return strings_.begin();
    }
    inline decltype(auto) end() const {
        return strings_.end();
    }
    inline decltype(auto) rbegin() const {
        return strings_.rbegin();
    }
    inline decltype(auto) rend() const {
        return strings_.rend();
    }
private:
    std::set<std::string> strings_;
};

}
