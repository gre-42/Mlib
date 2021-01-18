#pragma once
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

struct UiFocus {
    std::list<Focus> focus;
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
    } else {
        throw std::runtime_error("Unknown focus name \"" + str + '"');
    }
}

}
