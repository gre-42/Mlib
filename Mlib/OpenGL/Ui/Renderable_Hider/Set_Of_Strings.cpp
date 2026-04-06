#include "Set_Of_Strings.hpp"
#include <Mlib/Math/Sorted_Pointers.hpp>
#include <Mlib/Os/Os.hpp>

using namespace Mlib;

bool SetOfStrings::contains(const std::string& name) const {
    return strings_.contains(name);
}

void SetOfStrings::try_insert(const std::string& name, size_t max_entries) {
    auto capacity_ok = [&](){
        return strings_.size() < max_entries;
    };
    if (strings_.contains(name)) {
        return;
    } else if (!capacity_ok()) {
        lwarn(LogFlags::SUPPRESS_DUPLICATES) << "Too many renderables";
        return;
    }
    if (!capacity_ok()) {
        return;
    } else {
        strings_.insert(name);
    }
}
