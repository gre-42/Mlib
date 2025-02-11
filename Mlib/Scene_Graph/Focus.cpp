#include "Focus.hpp"
#include <Mlib/Regex/Regex_Select.hpp>
#include <Mlib/Scene_Graph/Focus_Filter.hpp>
#include <Mlib/Strings/String.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <algorithm>
#include <iostream>

using namespace Mlib;

Focuses::Focuses() = default;

Focuses::~Focuses() = default;

Focuses::Focuses(const std::initializer_list<Focus>& focuses)
    : focuses_{focuses}
{}

void Focuses::set_focuses(const std::initializer_list<Focus>& focuses) {
    mutex.assert_locked_by_caller();
    focuses_ = std::list(focuses.begin(), focuses.end());
}

void Focuses::set_focuses(const std::vector<Focus>& focuses)
{
    mutex.assert_locked_by_caller();
    focuses_ = std::list(focuses.begin(), focuses.end());
}

Focus Focuses::focus() const {
    return focuses_.empty()
        ? Focus::BASE
        : focuses_.back();
}

std::list<Focus>::const_iterator Focuses::find(Focus focus) const {
    return std::find(focuses_.begin(), focuses_.end(), focus);
}

std::list<Focus>::iterator Focuses::find(Focus focus) {
    return std::find(focuses_.begin(), focuses_.end(), focus);
}

std::list<Focus>::const_iterator Focuses::end() const {
    return focuses_.end();
}

std::list<Focus>::iterator Focuses::end() {
    return focuses_.end();
}

void Focuses::erase(const std::list<Focus>::iterator& it) {
    mutex.assert_locked_by_caller();
    focuses_.erase(it);
}

void Focuses::pop_back() {
    mutex.assert_locked_by_caller();
    if (focuses_.empty()) {
        THROW_OR_ABORT("pop_back called on empty focuses");
    }
    focuses_.pop_back();
}

void Focuses::push_back(Focus focus) {
    mutex.assert_locked_by_caller();
    if (this->focus() == focus) {
        THROW_OR_ABORT("Duplicate focus: " + focus_to_string(focus));
    }
    focuses_.push_back(focus);
}

bool Focuses::contains(Focus focus) const {
    return find(focus) != end();
}

bool Focuses::countdown_active() const {
    return contains(Focus::COUNTDOWN_PENDING) || contains(Focus::COUNTDOWN_COUNTING);
}

bool Focuses::game_over_countdown_active() const {
    return contains(Focus::GAME_OVER_COUNTDOWN_PENDING) || contains(Focus::GAME_OVER_COUNTDOWN_COUNTING);
}

size_t Focuses::size() const {
    return focuses_.size();
}

std::ostream& Mlib::operator << (std::ostream& ostr, const Focuses& focuses) {
    for (const auto& f : focuses.focuses_) {
        ostr << focus_to_string(f) << " ";
    }
    return ostr;
}

UiFocus::UiFocus() = default;

UiFocus::~UiFocus() = default;

void UiFocus::insert_submenu(
    const std::string& id,
    const SubmenuHeader& header,
    Focus focus_mask,
    size_t default_selection)
{
    if (!submenu_numbers.insert({id, submenu_headers.size()}).second) {
        THROW_OR_ABORT("Submenu with ID \"" + id + "\" already exists");
    }
    submenu_headers.push_back(header);
    focus_masks.push_back(focus_mask);
    // If the selection_ids array is not yet initialized, apply the default value.
    all_selection_ids.try_emplace(id, default_selection);
}

bool UiFocus::has_focus(const FocusFilter& focus_filter) const {
    if (!any(focuses.focus() & focus_filter.focus_mask)) {
        return false;
    }
    if (!focus_filter.submenu_ids.empty() && any(focuses.focus() & Focus::MENU_ANY)) {
        for (const std::string& submenu_id : focus_filter.submenu_ids) {
            auto it = submenu_numbers.find(submenu_id);
            if (it == submenu_numbers.end()) {
                THROW_OR_ABORT("Could not find submenu with ID " + submenu_id);
            }
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
    focus_masks.clear();
}

Focus Mlib::single_focus_from_string(const std::string& str) {
    static const std::map<std::string, Focus> m{
        {"main_menu", Focus::MAIN_MENU},
        {"new_game_menu", Focus::NEW_GAME_MENU},
        {"settings_menu", Focus::SETTINGS_MENU},
        {"controls_menu", Focus::CONTROLS_MENU},
        {"menu_any", Focus::MENU_ANY},
        {"loading", Focus::LOADING},
        {"countdown_pending", Focus::COUNTDOWN_PENDING},
        {"countdown_counting", Focus::COUNTDOWN_COUNTING},
        {"countdown_any", Focus::COUNTDOWN_ANY},
        {"game_over_countdown_pending", Focus::GAME_OVER_COUNTDOWN_PENDING},
        {"game_over_countdown_counting", Focus::GAME_OVER_COUNTDOWN_COUNTING},
        {"game_over_countdown_any", Focus::GAME_OVER_COUNTDOWN_ANY},
        {"scene", Focus::SCENE},
        {"game_over", Focus::GAME_OVER},
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
    if (any(focus & Focus::BASE)) result += "b";
    if (any(focus & Focus::MAIN_MENU)) result += "m";
    if (any(focus & Focus::NEW_GAME_MENU)) result += "n";
    if (any(focus & Focus::SETTINGS_MENU)) result += "s";
    if (any(focus & Focus::CONTROLS_MENU)) result += "o";
    if (any(focus & Focus::LOADING)) result += "l";
    if (any(focus & Focus::COUNTDOWN_PENDING)) result += "p";
    if (any(focus & Focus::COUNTDOWN_COUNTING)) result += "c";
    if (any(focus & Focus::GAME_OVER_COUNTDOWN_PENDING)) result += "P";
    if (any(focus & Focus::GAME_OVER_COUNTDOWN_COUNTING)) result += "C";
    if (any(focus & Focus::SCENE)) result += "S";
    if (any(focus & Focus::GAME_OVER)) result += "O";
    return '(' + result + ')';
}
