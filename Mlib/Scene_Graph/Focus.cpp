#include "Focus.hpp"
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

void UiFocus::goto_next_submenu() {
    if ((n_submenus != 0) && (submenu_id < n_submenus - 1)) {
        ++submenu_id;
    }
}

void UiFocus::goto_prev_submenu() {
    if (submenu_id > 0) {
        --submenu_id;
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
