#include "Focus.hpp"
#include <Mlib/Scene_Graph/Focus_Filter.hpp>
#include <algorithm>
#include <iostream>
#include <stdexcept>

using namespace Mlib;

Focuses::Focuses()
{}

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
    focuses_.pop_back();
}

void Focuses::push_back(Focus focus) {
    mutex.assert_locked_by_caller();
    if (this->focus() == focus) {
        throw std::runtime_error("Duplicate focus: " + focus_to_string(focus));
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

void UiFocus::insert_submenu(
    const std::string& id,
    const std::string& title,
    size_t default_selection)
{
    if (!submenu_numbers.insert({id, submenu_titles.size()}).second) {
        throw std::runtime_error("Submenu with ID \"" + id + "\" already exists");
    }
    submenu_titles.push_back(title);
    // If the selection_ids array is not yet initialized, apply the default value.
    selection_ids.try_emplace(id, default_selection);
}

bool UiFocus::has_focus(const FocusFilter& focus_filter) const {
    if (!any(focuses.focus() & focus_filter.focus_mask)) {
        return false;
    }
    if (!focus_filter.submenu_ids.empty() && focuses.focus() == Focus::MENU) {
        for (const std::string& submenu_id : focus_filter.submenu_ids) {
            auto it = submenu_numbers.find(submenu_id);
            if (it == submenu_numbers.end()) {
                throw std::runtime_error("Could not find submenu with ID " + submenu_id);
            }
            if (it->second == submenu_number) {
                return true;
            }
        }
        return false;
    }
    return true;
}

Focus Mlib::focus_from_string(const std::string& str) {
    if (str == "menu") {
        return Focus::MENU;
    } else if (str == "loading") {
        return Focus::LOADING;
    } else if (str == "countdown_pending") {
        return Focus::COUNTDOWN_PENDING;
    } else if (str == "countdown_counting") {
        return Focus::COUNTDOWN_COUNTING;
    } else if (str == "countdown_any") {
        return Focus::COUNTDOWN_ANY;
    } else if (str == "game_over_countdown_pending") {
        return Focus::GAME_OVER_COUNTDOWN_PENDING;
    } else if (str == "game_over_countdown_counting") {
        return Focus::GAME_OVER_COUNTDOWN_COUNTING;
    } else if (str == "game_over_countdown_any") {
        return Focus::GAME_OVER_COUNTDOWN_ANY;
    } else if (str == "scene") {
        return Focus::SCENE;
    } else if (str == "game_over") {
        return Focus::GAME_OVER;
    } else if (str == "always") {
        return Focus::ALWAYS;
    } else {
        throw std::runtime_error("Unknown focus name \"" + str + '"');
    }
}

std::string Mlib::focus_to_string(Focus focus) {
    std::string result;
    if (any(focus & Focus::MENU)) result += "m";
    if (any(focus & Focus::LOADING)) result += "l";
    if (any(focus & Focus::COUNTDOWN_PENDING)) result += "p";
    if (any(focus & Focus::COUNTDOWN_COUNTING)) result += "c";
    if (any(focus & Focus::GAME_OVER_COUNTDOWN_PENDING)) result += "P";
    if (any(focus & Focus::GAME_OVER_COUNTDOWN_COUNTING)) result += "C";
    if (any(focus & Focus::SCENE)) result += "s";
    if (any(focus & Focus::GAME_OVER)) result += "o";
    return '(' + result + ')';
}
