#include "Key_Configuration.hpp"
#include <Mlib/Strings/String.hpp>

using namespace Mlib;

std::string KeyConfiguration::to_string() const {
    std::list<std::string> result;
    auto append = [&](std::string s){
        if (!s.empty()) {
            result.emplace_back(std::move(s));
        }
    };
    append(base_combo.to_string());
    append(base_gamepad_analog_axes.to_string());
    append(base_cursor_axis.to_string());
    append(base_scroll_wheel_axis.to_string());
    if (result.size() > 1) {
        return '(' + join(" | ", result) + ')';
    }
    return result.empty() ? "" : result.front();
}
