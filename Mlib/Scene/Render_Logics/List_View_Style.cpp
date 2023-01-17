#include "List_View_Style.hpp"
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

ListViewStyle Mlib::list_view_style_from_string(const std::string& s) {
    if (s == "text") {
        return ListViewStyle::TEXT;
    }
    if (s == "icon") {
        return ListViewStyle::ICON;
    }
    THROW_OR_ABORT("Unknown listview style: \"" + s + '"');
}
