#pragma once
#include <Mlib/Hashing/Variable_And_Hash.hpp>
#include <Mlib/Os/Utf8_Path.hpp>
#include <string>
#include <unordered_set>

namespace Mlib {

class UnorderedSetOfStrings {
public:
    bool contains(const VariableAndHash<std::string>& name) const;
    void try_insert(const VariableAndHash<std::string>& name, size_t max_entries);
    void load_from_file(const Utf8Path& filename);
    void save_to_file(const Utf8Path& filename) const;
    inline bool empty() const {
        return strings_.empty();
    }
private:
    std::unordered_set<VariableAndHash<std::string>> strings_;
};

}
