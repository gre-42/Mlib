#pragma once
#include <algorithm>
#include <list>
#include <stdexcept>
#include <string>

namespace Mlib {

enum class Focus {
    NONE = 0,
    BASE = 1,
    MENU = 1 << 1,
    LOADING = 1 << 2,
    COUNTDOWN = 1 << 3,
    SCENE = 1 << 4,
    ALWAYS = BASE | MENU | LOADING | COUNTDOWN | SCENE
};

inline Focus operator | (Focus a, Focus b) {
    return Focus((unsigned int)a | (unsigned int)b);
}

inline Focus operator & (Focus a, Focus b) {
    return Focus((unsigned int)a & (unsigned int)b);
}

class Focuses {
public:
    Focuses() = default;
    Focuses(const std::initializer_list<Focus>& focuses)
    : focuses_{focuses}
    {}
    inline Focus focus() const {
        return focuses_.empty()
            ? Focus::BASE
            : focuses_.back();
    }
    inline std::list<Focus>::const_iterator find(Focus focus) const {
        return std::find(focuses_.begin(), focuses_.end(), focus);
    }
    inline std::list<Focus>::iterator find(Focus focus) {
        return std::find(focuses_.begin(), focuses_.end(), focus);
    }
    inline std::list<Focus>::const_iterator end() const {
        return focuses_.end();
    }
    inline std::list<Focus>::iterator end() {
        return focuses_.end();
    }
    inline void erase(const std::list<Focus>::iterator& it) {
        focuses_.erase(it);
    }
    void pop_back() {
        focuses_.pop_back();
    }
    void push_back(Focus focus) {
        focuses_.push_back(focus);
    }
    bool contains(Focus focus) const {
        return find(focus) != end();
    }
    size_t size() const {
        return focuses_.size();
    }
private:
    std::list<Focus> focuses_;
};

struct UiFocus {
    Focuses focuses;
    size_t submenu_id = 0;
    size_t n_submenus = 0;
    inline void goto_next_submenu() {
        if ((n_submenus != 0) && (submenu_id < n_submenus - 1)) {
            ++submenu_id;
        }
    }
    inline void goto_prev_submenu() {
        if (submenu_id > 0) {
            --submenu_id;
        }
    }
};

inline Focus focus_from_string(const std::string& str) {
    if (str == "menu") {
        return Focus::MENU;
    } else if (str == "loading") {
        return Focus::LOADING;
    } if (str == "countdown") {
        return Focus::COUNTDOWN;
    } if (str == "scene") {
        return Focus::SCENE;
    } if (str == "always") {
        return Focus::ALWAYS;
    } else {
        throw std::runtime_error("Unknown focus name \"" + str + '"');
    }
}

}
