#pragma once
#include <Mlib/Threads/Checked_Mutex.hpp>
#include <atomic>
#include <iosfwd>
#include <list>
#include <map>
#include <string>
#include <vector>

namespace Mlib {

struct FocusFilter;

enum class Focus {
    NONE = 0,
    BASE = 1,
    MENU = 1 << 1,
    LOADING = 1 << 2,
    COUNTDOWN_PENDING = 1 << 3,
    COUNTDOWN_COUNTING = 1 << 4,
    GAME_OVER_COUNTDOWN_PENDING = 1 << 5,
    GAME_OVER_COUNTDOWN_COUNTING = 1 << 6,
    SCENE = 1 << 7,
    GAME_OVER = 1 << 8,  // currently not in use, countdown is used instead.
    COUNTDOWN_ANY = COUNTDOWN_PENDING | COUNTDOWN_COUNTING,
    GAME_OVER_COUNTDOWN_ANY = GAME_OVER_COUNTDOWN_PENDING | GAME_OVER_COUNTDOWN_COUNTING,
    ALWAYS =
        BASE |
        MENU |
        LOADING |
        COUNTDOWN_PENDING |
        COUNTDOWN_COUNTING |
        GAME_OVER_COUNTDOWN_PENDING |
        GAME_OVER_COUNTDOWN_COUNTING |
        SCENE |
        GAME_OVER
};

inline Focus operator | (Focus a, Focus b) {
    return Focus((unsigned int)a | (unsigned int)b);
}

inline Focus operator & (Focus a, Focus b) {
    return Focus((unsigned int)a & (unsigned int)b);
}

inline Focus& operator |= (Focus& a, Focus b) {
    a = a | b;
    return a;
}

inline bool any(Focus f) {
    return f != Focus::NONE;
}

class Focuses {
    friend std::ostream& operator << (std::ostream& ostr, const Focuses& focuses);
public:
    Focuses();
    Focuses(const std::initializer_list<Focus>& focuses);
    Focuses(const Focuses&) = delete;
    Focuses& operator = (const Focuses&) = delete;
    void set_focuses(const std::initializer_list<Focus>& focuses);
    void set_focuses(const std::vector<Focus>& focuses);
    Focus focus() const;
    std::list<Focus>::const_iterator find(Focus focus) const;
    std::list<Focus>::iterator find(Focus focus);
    std::list<Focus>::const_iterator end() const;
    std::list<Focus>::iterator end();
    void erase(const std::list<Focus>::iterator& it);
    void pop_back();
    void push_back(Focus focus);
    bool contains(Focus focus) const;
    bool countdown_active() const;
    bool game_over_countdown_active() const;
    size_t size() const;
    mutable CheckedMutex mutex;
private:
    std::list<Focus> focuses_;
};

std::ostream& operator << (std::ostream& ostr, const Focuses& focuses);

struct SubmenuHeader {
    std::string title;
    std::string icon;
    std::vector<std::string> requires_;
};

struct UiFocus {
    UiFocus();
    ~UiFocus();
    UiFocus(const UiFocus&) = delete;
    UiFocus& operator = (const UiFocus&) = delete;
    Focuses focuses;
    std::atomic_size_t submenu_number = 0;
    std::map<std::string, size_t> submenu_numbers;
    std::vector<SubmenuHeader> submenu_headers;
    std::map<std::string, std::atomic_size_t> selection_ids;
    void insert_submenu(
        const std::string& id,
        const SubmenuHeader& header,
        size_t default_selection);
    bool has_focus(const FocusFilter& focus_filter) const;
};

Focus single_focus_from_string(const std::string& str);
Focus focus_from_string(const std::string& str);
std::string focus_to_string(Focus focus);

}
