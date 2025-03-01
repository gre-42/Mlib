#pragma once
#include <set>
#include <string>

namespace Mlib {

enum class Focus;

struct FocusFilter {
    Focus focus_mask;
    std::set<std::string> submenu_ids;
};

}
