#include "Focus.hpp"
#include <Mlib/Scene_Graph/Focus_Filter.hpp>
#include <iostream>

using namespace Mlib;

Focuses::Focuses()
{}

Focuses::Focuses(const std::initializer_list<Focus>& focuses)
: focuses_{focuses}
{}

Focuses::Focuses(const std::vector<Focus>& focuses)
: focuses_(focuses.begin(), focuses.end())
{}

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
    focuses_.erase(it);
}

void Focuses::pop_back() {
    focuses_.pop_back();
}

void Focuses::push_back(Focus focus) {
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

size_t Focuses::size() const {
    return focuses_.size();
}

std::ostream& Mlib::operator << (std::ostream& ostr, const Focuses& focuses) {
    for (const auto& f : focuses.focuses_) {
        ostr << focus_to_string(f) << " ";
    }
    return ostr;
}

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
    selection_ids.insert({ id, default_selection });
}

bool UiFocus::has_focus(const FocusFilter& focus_filter) const {
    if ((focuses.focus() & focus_filter.focus_mask) == Focus::NONE) {
        return false;
    }
    if (!focus_filter.submenu_id.empty() && focuses.focus() == Focus::MENU) {
        auto it = submenu_numbers.find(focus_filter.submenu_id);
        if (it == submenu_numbers.end()) {
            throw std::runtime_error("Could not find submenu with ID " + focus_filter.submenu_id);
        }
        if (it->second != submenu_number) {
            return false;
        }
    }
    return true;
}

void UiFocus::goto_next_submenu() {
    if ((submenu_titles.size() != 0) && (submenu_number < submenu_titles.size() - 1)) {
        ++submenu_number;
    }
}

void UiFocus::goto_prev_submenu() {
    if (submenu_number > 0) {
        --submenu_number;
    }
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
    } else if (str == "scene") {
        return Focus::SCENE;
    } else if (str == "always") {
        return Focus::ALWAYS;
    } else {
        throw std::runtime_error("Unknown focus name \"" + str + '"');
    }
}

std::string Mlib::focus_to_string(Focus focus) {
    std::string result;
    if ((focus & Focus::MENU) != Focus::NONE) result += "m";
    if ((focus & Focus::LOADING) != Focus::NONE) result += "l";
    if ((focus & Focus::COUNTDOWN_PENDING) != Focus::NONE) result += "p";
    if ((focus & Focus::COUNTDOWN_COUNTING) != Focus::NONE) result += "c";
    if ((focus & Focus::SCENE) != Focus::NONE) result += "s";
    return '(' + result + ')';
}
