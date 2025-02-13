#include "Key_Configuration.hpp"
#include <Mlib/Render/Key_Bindings/Input_Type.hpp>
#include <Mlib/Strings/String.hpp>

using namespace Mlib;

std::string KeyConfiguration::to_string(InputType filter) const {
    std::list<std::string> result;
    auto append0 = [&](std::string s){
        if (!s.empty()) {
            result.emplace_back(std::move(s));
        }
    };
    auto append1 = [&](const std::string& prefix, const std::string& s){
        if (!s.empty()) {
            result.emplace_back('(' + prefix + s + ')');
        }
    };
    append0(base_combo.to_string(filter));
    append0(base_gamepad_analog_axes.to_string(filter));
    if (any(filter & InputType::MOUSE)) {
        append1("cursor: ", base_cursor_axis.to_string());
        append1("scroll wheel: ", base_scroll_wheel_axis.to_string());
    }
    if (result.size() > 1) {
        return '(' + join(" | ", result) + ')';
    }
    return result.empty() ? "" : result.front();
}
