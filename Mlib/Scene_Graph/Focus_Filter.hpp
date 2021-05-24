#pragma once
#include <string>

namespace Mlib {

enum class Focus;

struct FocusFilter {
    Focus focus_mask;
    std::string submenu_id;
};

}
