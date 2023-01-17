#pragma once
#include <string>

namespace Mlib {

enum class ListViewStyle {
    TEXT,
    ICON
};

ListViewStyle list_view_style_from_string(const std::string& s);

}
