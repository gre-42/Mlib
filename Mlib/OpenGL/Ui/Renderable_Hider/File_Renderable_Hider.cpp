#include "File_Renderable_Hider.hpp"
#include <Mlib/OpenGL/Batch_Renderers/Special_Renderable_Names.hpp>
#include <Mlib/OpenGL/Key_Bindings/Key_Configuration.hpp>
#include <Mlib/Os/Os.hpp>
#include <mutex>
#include <shared_mutex>

using namespace Mlib;

using S = VariableAndHash<std::string>;

FileRenderableHider::FileRenderableHider(
    const ButtonStates& button_states,
    Utf8Path filename)
    : filename_{std::move(filename)}
    , load_{ button_states, key_configurations_, 0, "load", "" }
    , save_{ button_states, key_configurations_, 0, "save", "" }
{
    auto lock = key_configurations_.lock_exclusive_for(std::chrono::seconds(2), "Key configurations");
    lock->insert(0, "load", KeyConfiguration{ {.key_bindings = {{.key = S("LEFT_CONTROL")}, {.key = S("L")}}} });
    lock->insert(0, "save", KeyConfiguration{ {.key_bindings = {{.key = S("LEFT_CONTROL")}, {.key = S("S")}}} });
}

void FileRenderableHider::process_input() {
    if (load_.keys_pressed()) {
        std::scoped_lock lock{ mutex_ };
        visible_names_.load_from_file(filename_);
    }
    if (save_.keys_pressed()) {
        std::scoped_lock lock{ mutex_ };
        available_names_.save_to_file(filename_);
    }
}

bool FileRenderableHider::is_visible(const VariableAndHash<std::string>& name) {
    if (name->empty() || (name == AAR_NAME)) {
        return true;
    }
    std::shared_lock lock{ mutex_ };
    auto result = [&name, this](){
        if (visible_names_.empty()) {
            return true;
        }
        return visible_names_.contains(name);
    }();
    available_names_.try_insert(name, 10'000);
    return result;
}
