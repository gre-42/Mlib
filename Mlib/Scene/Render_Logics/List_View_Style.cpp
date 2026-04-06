#include "List_View_Style.hpp"
#include <stdexcept>

using namespace Mlib;

ListViewStyle Mlib::list_view_style_from_string(const std::string& s) {
    if (s == "text") {
        return ListViewStyle::TEXT;
    }
    if (s == "icon") {
        return ListViewStyle::ICON;
    }
    throw std::runtime_error("Unknown listview style: \"" + s + '"');
}
