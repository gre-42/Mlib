#include "Focus.hpp"
#include <Mlib/Iterator/Enumerate.hpp>
#include <Mlib/Json/Base.hpp>
#include <Mlib/Macro_Executor/Focus_Filter.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Regex/Regex_Select.hpp>
#include <Mlib/Strings/String.hpp>
#include <Mlib/Threads/Containers/Thread_Safe_String_Json.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <algorithm>
#include <iostream>

using namespace Mlib;

Focuses::Focuses()
    : focus_merge_{Focus::NONE}
{}

Focuses::~Focuses() = default;

Focuses::Focuses(const std::initializer_list<Focus>& focuses)
    : focus_stack_{focuses}
{
    compute_focus_merge();
}

void Focuses::compute_focus_merge() {
    mutex.is_owner();
    focus_merge_ = Focus::NONE;
    for (const auto f : focus_stack_) {
        focus_merge_ |= f;
    }
}

void Focuses::set_focuses(const std::initializer_list<Focus>& focuses) {
    mutex.is_owner();
    focus_stack_ = std::list(focuses.begin(), focuses.end());
    compute_focus_merge();
}

void Focuses::set_focuses(const std::vector<Focus>& focuses)
{
    mutex.is_owner();
    focus_stack_ = std::list(focuses.begin(), focuses.end());
    compute_focus_merge();
}

bool Focuses::operator == (const std::vector<Focus>& focuses) const {
    if (focus_stack_.size() != focuses.size()) {
        return false;
    }
    for (const auto& [i, e] : enumerate(focus_stack_)) {
        if (e != focuses[i]) {
            return false;
        }
    }
    return true;
}

bool Focuses::operator != (const std::vector<Focus>& focuses) const {
    return !(*this == focuses);
}

void Focuses::replace(Focus old, Focus new_) {
    mutex.is_owner();
    auto it = std::find(focus_stack_.begin(), focus_stack_.end(), old);
    if (it == focus_stack_.end()) {
        THROW_OR_ABORT("Could not find focus with value \"" + focus_to_string(old) + '"');
    }
    *it = new_;
    compute_focus_merge();
}

Focus Focuses::back_or_none() const {
    return focus_stack_.empty()
        ? Focus::NONE
        : focus_stack_.back();
}

void Focuses::remove(Focus focus) {
    mutex.is_owner();
    auto it = std::find(focus_stack_.begin(), focus_stack_.end(), focus);
    if (it == focus_stack_.end()) {
        THROW_OR_ABORT("Could not find focus with value \"" + focus_to_string(focus) + '"');
    }
    focus_stack_.erase(it);
    compute_focus_merge();
}

void Focuses::pop_back() {
    mutex.is_owner();
    if (focus_stack_.empty()) {
        THROW_OR_ABORT("pop_back called on empty focuses");
    }
    focus_stack_.pop_back();
    compute_focus_merge();
}

void Focuses::force_push_back(Focus focus) {
    mutex.is_owner();
    if (any(focus_merge_ & focus)) {
        THROW_OR_ABORT("Duplicate focus: " + focus_to_string(focus));
    }
    focus_stack_.push_back(focus);
    compute_focus_merge();
}

bool Focuses::has_focus(Focus focus) const {
    auto f = any(focus & Focus::QUERY_CONTAINS)
        ? focus_merge_
        : back_or_none();
    return any(focus & Focus::QUERY_ALL)
        ? all(f, focus & ~Focus::ANY_QUERY)
        : any(f & focus);
}

bool Focuses::game_over() const {
    return has_focus(Focus::QUERY_CONTAINS | Focus::GAME_OVER);
}

bool Focuses::empty() const {
    return focus_stack_.empty();
}

size_t Focuses::size() const {
    return focus_stack_.size();
}

std::ostream& Mlib::operator << (std::ostream& ostr, const Focuses& focuses) {
    for (const auto& f : focuses.focus_stack_) {
        ostr << focus_to_string(f) << " ";
    }
    return ostr;
}

UiFocus::UiFocus(std::string filename)
    : filename_{ std::move(filename) }
    , has_changes_{ false }
{}

UiFocus::~UiFocus() = default;

SubmenuHeader& UiFocus::insert_submenu(
    const std::string& id,
    const SubmenuHeader& header,
    FocusFilter focus_filter,
    size_t default_selection)
{
    if (!submenu_numbers.try_emplace(id, submenu_headers.size()).second) {
        THROW_OR_ABORT("Submenu with ID \"" + id + "\" already exists");
    }
    auto& res = submenu_headers.emplace_back(header);
    focus_filters.emplace_back(std::move(focus_filter));
    // If the selection_ids array is not yet initialized, apply the default value.
    all_selection_ids.try_emplace(id, default_selection);
    return res;
}

void UiFocus::try_push_back(Focus focus) {
    focuses.force_push_back(focus);
    if (focuses.has_focus(Focus::MENU_ANY)) {
        pop_invalid_focuses();
    }
}

bool UiFocus::has_focus(const FocusFilter& focus_filter) const {
    if (!focuses.has_focus(focus_filter.focus_mask)) {
        return false;
    }
    if (!focus_filter.submenu_ids.empty() && focuses.has_focus(Focus::MENU_ANY)) {
        for (const std::string& submenu_id : focus_filter.submenu_ids) {
            auto it = submenu_numbers.find(submenu_id);
            if (it == submenu_numbers.end()) {
                THROW_OR_ABORT("Could not find submenu with ID " + submenu_id);
            }
            // Has focus if it is selected in at least one tab menu
            for (const auto& [_, n] : menu_selection_ids) {
                if (it->second == n) {
                    return true;
                }
            }
        }
        return false;
    }
    return true;
}

void UiFocus::clear() {
    submenu_numbers.clear();
    submenu_headers.clear();
    focus_filters.clear();
}

bool UiFocus::editing() const {
    std::scoped_lock lock{ edit_mutex };
    return edit_focus.has_value();
}

void UiFocus::set_persisted_selection_id(
    const std::string_view& submenu,
    const nlohmann::json& s,
    PersistedValueType cb)
{
    set_persisted_selection_id_generic(submenu, s, cb);
}

void UiFocus::set_persisted_selection_id(
    const std::vector<std::string>& submenu,
    const nlohmann::json& s,
    PersistedValueType cb)
{
    set_persisted_selection_id_generic(submenu, s, cb);
}
    
template <JsonKey Key>
void UiFocus::set_persisted_selection_id_generic(
    const Key& submenu,
    const nlohmann::json& s,
    PersistedValueType cb)
{
    if (cb == PersistedValueType::CUSTOM) {
        current_persistent_selection_ids.set_and_notify(submenu, s);
        has_changes_ = get_has_changes();
    } else if (!current_persistent_selection_ids.contains(submenu)) {
        loaded_persistent_selection_ids.set_and_notify(submenu, s);
        current_persistent_selection_ids.set_and_notify(submenu, s);
    }
}

nlohmann::json UiFocus::get_persisted_selection_id(const std::string& submenu) const
{
    auto c = current_persistent_selection_ids.json_macro_arguments();
    auto it = c->try_at(submenu);
    if (!it.has_value()) {
        THROW_OR_ABORT("Could not find persisted submenu \"" + submenu + '"');
    }
    return *it;
}

void UiFocus::set_requires_reload(std::string submenu, std::string reason) {
    requires_reload_.emplace(std::move(submenu), std::move(reason));
}

void UiFocus::clear_requires_reload(const std::string& submenu) {
    requires_reload_.erase(submenu);
}

const std::map<std::string, std::string>& UiFocus::requires_reload() const {
    return requires_reload_;
}

bool UiFocus::has_changes() const {
    return has_changes_;
}

bool UiFocus::can_load() const {
    return !filename_.empty() && path_exists(filename_);
}

bool UiFocus::can_save() const {
    return !filename_.empty();
}

static bool has_changes_rec(const JsonView& c, const JsonView& l){
    for (const auto& [k, v] : c.json().items()) {
        auto it = l.try_at(k);
        if (!it.has_value()) {
            return true;
        }
        if (v.type() == nlohmann::detail::value_t::object) {
            if (has_changes_rec(JsonView{v}, JsonView{*it})) {
                return true;
            }
        } else if (*it != v) {
            return true;
        }
    }
    for (const auto& [k, v] : l.json().items()) {
        auto it = c.try_at(k);
        if (!it.has_value()) {
            return true;
        }
        if (v.type() == nlohmann::detail::value_t::object) {
            if (has_changes_rec(JsonView{*it}, JsonView{v})) {
                return true;
            }
        }
    }
    return false;
};

bool UiFocus::get_has_changes() const {
    auto c = current_persistent_selection_ids.json_macro_arguments();
    auto l = loaded_persistent_selection_ids.json_macro_arguments();
    return has_changes_rec(c, l);
}

void UiFocus::load() {
    auto f = create_ifstream(filename_);
    if (f->fail()) {
        THROW_OR_ABORT("Could not open file \"" + filename_ + "\" for read");
    }
    nlohmann::json j;
    *f >> j;
    JsonView jv{ j };
    loaded_persistent_selection_ids.assign_and_notify(jv);
    current_persistent_selection_ids.assign_and_notify(jv);
}

void UiFocus::save() {
    auto f = create_ofstream(filename_);
    if (f->fail()) {
        THROW_OR_ABORT("Could not open file \"" + filename_ + "\" for write");
    }
    auto c = current_persistent_selection_ids.json_macro_arguments();
    nlohmann::json j = c;
    *f << j;
    if (f->fail()) {
        THROW_OR_ABORT("Could not write to file \"" + filename_ + '"');
    }
    loaded_persistent_selection_ids.assign_and_notify(c);
}

void UiFocus::pop_invalid_focuses() {
    focuses.mutex.is_owner();
    while (true) {
        bool any_has_focus = false;
        bool retry = false;
        for (const auto& [i, focus_filter] : enumerate(focus_filters)) {
            if (has_focus(focus_filter)) {
                any_has_focus = true;
                if (!submenu_headers.at(i).required()) {
                    lwarn() <<
                        "Pop invalid focus because of \"" << submenu_headers.at(i).title <<
                        "\": " << focuses;
                    if (focuses.size() <= 1) {
                        THROW_OR_ABORT("Cannot remove focus layer after invalidation");
                    }
                    focuses.pop_back();
                    retry = true;
                }
            }
        }
        if (!any_has_focus && focuses.has_focus(Focus::MENU_ANY) && (focuses.size() > 1)) {
            lwarn() << "Pop invalid focus because no submenu has the focus: " << focuses;
            focuses.pop_back();
        } else if (!retry) {
            break;
        }
    }
}

UiFocuses::UiFocuses(std::string filename_prefix)
    : filename_prefix_{ std::move(filename_prefix) }
{
    // Ensure the first user exists, so the settings are loaded
    // from disk.
    (*this)[0];
}

UiFocuses::~UiFocuses() = default;

UiFocus& UiFocuses::operator [] (uint32_t user_id) {
    auto it = focuses_.find(user_id);
    if (it == focuses_.end()) {
        auto res = focuses_.try_emplace(
            user_id,
            // Only save if the "filename_prefix_" is not empty,
            // and only for the first user.
            filename_prefix_.empty() || (user_id != 0)
                ? ""
                : filename_prefix_ + std::to_string(user_id) + ".json");
        if (!res.second) {
            verbose_abort("Could not create UiFocus");
        }
        return res.first->second;
    }
    return it->second;
}

const UiFocus& UiFocuses::operator [] (uint32_t user_id) const {
    return const_cast<UiFocuses&>(*this)[user_id];
}

void UiFocuses::trim(uint32_t user_count) {
    std::erase_if(focuses_, [&](const auto& item) {
        return item.first >= user_count;
    });
}

void UiFocuses::try_load() {
    for (auto& [_, f] : focuses_) {
        if (f.can_load()) {
            f.load();
        }
    }
}

void UiFocuses::try_save() {
    for (auto& [_, f] : focuses_) {
        if (f.has_changes() && f.can_save()) {
            f.save();
        }
    }
}

void UiFocuses::clear() {
    for (auto& [_, f] : focuses_) {
        f.clear();
    }
}

void UiFocuses::clear_focuses() {
    for (auto& [_, f] : focuses_) {
        std::scoped_lock lock{f.focuses.mutex};
        f.focuses.set_focuses({});
    }
}

void UiFocuses::pop_invalid_focuses() {
    for (auto& [_, f] : focuses_) {
        std::scoped_lock lock{ f.focuses.mutex };
        f.pop_invalid_focuses();
    }
}

Focus Mlib::single_focus_from_string(const std::string& str) {
    static const std::map<std::string, Focus> m{
        {"main_menu", Focus::MAIN_MENU},
        {"new_game_menu", Focus::NEW_GAME_MENU},
        {"settings_menu", Focus::SETTINGS_MENU},
        {"controls_menu", Focus::CONTROLS_MENU},
        {"menu_any", Focus::MENU_ANY},
        {"loading", Focus::LOADING},
        {"scene", Focus::SCENE},
        {"game_over", Focus::GAME_OVER},
        {"query_contains", Focus::QUERY_CONTAINS},
        {"query_all", Focus::QUERY_ALL},
        {"always", Focus::ALWAYS}
    };
    auto it = m.find(str);
    if (it == m.end()) {
        THROW_OR_ABORT("Unknown focus name \"" + str + '"');
    }
    return it->second;
}

Focus Mlib::focus_from_string(const std::string& s) {
    static const DECLARE_REGEX(re, "\\|");
    Focus result = Focus::NONE;
    for (const auto& m : string_to_list(s, re)) {
        result |= single_focus_from_string(m);
    }
    return result;
}

std::string Mlib::focus_to_string(Focus focus) {
    std::string result;
    if (any(focus & Focus::MAIN_MENU)) result += "m";
    if (any(focus & Focus::NEW_GAME_MENU)) result += "n";
    if (any(focus & Focus::SETTINGS_MENU)) result += "s";
    if (any(focus & Focus::CONTROLS_MENU)) result += "o";
    if (any(focus & Focus::LOADING)) result += "l";
    if (any(focus & Focus::SCENE)) result += "S";
    if (any(focus & Focus::GAME_OVER)) result += "O";
    return '(' + result + ')';
}
