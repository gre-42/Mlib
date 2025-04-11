#include "Tty_Renderable_Hider.hpp"
#include <Mlib/Os/Os.hpp>
#include <Mlib/Render/Batch_Renderers/Special_Renderable_Names.hpp>
#include <Mlib/Render/Key_Bindings/Key_Configuration.hpp>
#include <mutex>
#include <shared_mutex>

using namespace Mlib;


TtyRenderableHider::TtyRenderableHider(const ButtonStates& button_states)
    : decrease_{ button_states, key_configurations_, "decrease", "" }
    , increase_{ button_states, key_configurations_, "increase", "" }
    , decrease_much_{ button_states, key_configurations_, "decrease_much", "" }
    , increase_much_{ button_states, key_configurations_, "increase_much", "" }
    , show_only_1_{ button_states, key_configurations_, "show_only_1", "" }
{
    auto lock = key_configurations_.lock_exclusive_for(std::chrono::seconds(2), "Key configurations");
    auto& cfg = lock->emplace();
    cfg.insert("decrease", KeyConfiguration{ {.key_bindings = {{.key = "LEFT_CONTROL"}, {.key = "UP"}}} });
    cfg.insert("increase", KeyConfiguration{ {.key_bindings = {{.key = "LEFT_CONTROL"}, {.key = "DOWN"}}} });
    cfg.insert("decrease_much", KeyConfiguration{ {.key_bindings = {{.key = "LEFT_CONTROL"}, {.key = "PAGE_UP"}}} });
    cfg.insert("increase_much", KeyConfiguration{ {.key_bindings = {{.key = "LEFT_CONTROL"}, {.key = "PAGE_DOWN"}}} });
    cfg.insert("show_only_1", KeyConfiguration{ {.key_bindings = {{.key = "LEFT_CONTROL"}, {.key = "LEFT_SHIFT"}}} });
}

void TtyRenderableHider::process_input() {
    auto old_first_visible_name = [this]() {
        std::shared_lock lock{ mutex_ };
        return first_visible_name_;
    }();
    auto dec = [this](size_t n){
        std::scoped_lock lock{ mutex_ };
        auto it = available_names_.find(first_visible_name_);
        if (it == available_names_.end()) {
            first_visible_name_.clear();
            if (!available_names_.empty()) {
                first_visible_name_ = *available_names_.begin();
            }
        } else {
            for (size_t i = 0; (i < n) && (it != available_names_.end()); ++i, --it);
            if (it != available_names_.end()) {
                first_visible_name_ = *it;
            } else {
                first_visible_name_.clear();
                if (!available_names_.empty()) {
                    first_visible_name_ = *available_names_.begin();
                }
            }
        }
    };
    auto inc = [this](size_t n){
        std::scoped_lock lock{ mutex_ };
        auto it = available_names_.find(first_visible_name_);
        if (it == available_names_.end()) {
            first_visible_name_.clear();
            if (!available_names_.empty()) {
                first_visible_name_ = *available_names_.begin();
            }
        } else {
            for (size_t i = 0; (i < n) && (it != available_names_.end()); ++i, ++it);
            if (it != available_names_.end()) {
                first_visible_name_ = *it;
            } else {
                first_visible_name_.clear();
                if (!available_names_.empty()) {
                    first_visible_name_ = *available_names_.rbegin();
                }
            }
        }
    };
    if (decrease_.keys_pressed()) {
        dec(1);
    }
    if (decrease_much_.keys_pressed()) {
        dec(50);
    }
    if (increase_.keys_pressed()) {
        inc(1);
    }
    if (increase_much_.keys_pressed()) {
        inc(50);
    }
    {
        std::shared_lock lock{ mutex_ };
        if (old_first_visible_name != first_visible_name_) {
            linfo() << "First visible renderable: " << first_visible_name_;
        }
    }
}

bool TtyRenderableHider::is_visible(const std::string& name) {
    if (name.empty() || (name == *AAR_NAME)) {
        return true;
    }
    auto result = [&name, this](){
        std::shared_lock lock{ mutex_ };
        if (first_visible_name_.empty()) {
            return true;
        }
        if (show_only_1_.keys_down()) {
            return name == first_visible_name_;
        } else {
            return name >= first_visible_name_;
        }
    }();
    auto capacity_ok = [this](){
        return available_names_.size() < 10'000;
    };
    {
        std::shared_lock lock{ mutex_ };
        if (available_names_.contains(name)) {
            return result;
        } else if (!capacity_ok()) {
            lwarn() << "Too many renderables";
            return result;
        }
    }
    std::scoped_lock lock{ mutex_ };
    if (!capacity_ok()) {
        return result;
    } else {
        available_names_.insert(name);
    }
    return result;
}
