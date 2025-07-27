#pragma once
#include <Mlib/Macro_Executor/Boolean_Expression.hpp>
#include <Mlib/Threads/Checked_Mutex.hpp>
#include <Mlib/Threads/Containers/Thread_Safe_String.hpp>
#include <atomic>
#include <iosfwd>
#include <list>
#include <map>
#include <string>
#include <vector>

namespace Mlib {

struct FocusFilter;
class MacroLineExecutor;

enum class Focus {
    // Focuses
    NONE = 0,
    MAIN_MENU = 1 << 0,
    NEW_GAME_MENU = 1 << 1,
    SETTINGS_MENU = 1 << 2,
    CONTROLS_MENU = 1 << 3,
    LOADING = 1 << 4,
    COUNTDOWN_PENDING = 1 << 5,
    COUNTDOWN_COUNTING = 1 << 6,
    GAME_OVER_COUNTDOWN_PENDING = 1 << 7,
    GAME_OVER_COUNTDOWN_COUNTING = 1 << 8,
    SCENE = 1 << 9,
    GAME_OVER = 1 << 10,  // currently not in use, countdown is used instead.
    MENU_ANY = MAIN_MENU | NEW_GAME_MENU | SETTINGS_MENU | CONTROLS_MENU,
    COUNTDOWN_ANY = COUNTDOWN_PENDING | COUNTDOWN_COUNTING,
    GAME_OVER_COUNTDOWN_ANY = GAME_OVER_COUNTDOWN_PENDING | GAME_OVER_COUNTDOWN_COUNTING,
    // Queries
    QUERY_CONTAINS = 1 << 16,
    QUERY_ALL = 1 << 17,
    ANY_QUERY = QUERY_CONTAINS | QUERY_ALL,
    ALWAYS = QUERY_CONTAINS | QUERY_ALL | NONE
};

inline Focus operator | (Focus a, Focus b) {
    return Focus((int)a | (int)b);
}

inline Focus operator & (Focus a, Focus b) {
    return Focus((int)a & (int)b);
}

inline Focus& operator |= (Focus& a, Focus b) {
    a = a | b;
    return a;
}

inline Focus operator ~ (Focus a) {
    return Focus(~(int)a);
}

inline bool any(Focus f) {
    return f != Focus::NONE;
}

inline bool all(Focus f, Focus required) {
    return !any(~f & required);
}

class Focuses {
    friend std::ostream& operator << (std::ostream& ostr, const Focuses& focuses);
public:
    Focuses();
    ~Focuses();
    Focuses(const std::initializer_list<Focus>& focuses);
    Focuses(const Focuses&) = delete;
    Focuses& operator = (const Focuses&) = delete;
    void set_focuses(const std::initializer_list<Focus>& focuses);
    void set_focuses(const std::vector<Focus>& focuses);
    bool operator == (const std::vector<Focus>& focuses) const;
    bool operator != (const std::vector<Focus>& focuses) const;
    void replace(Focus old, Focus new_);
    void remove(Focus focus);
    void pop_back();
    void force_push_back(Focus focus);
    bool has_focus(Focus focus) const;
    bool countdown_active() const;
    bool game_over_countdown_active() const;
    bool empty() const;
    size_t size() const;
    mutable CheckedMutex mutex;
private:
    void compute_focus_merge();
    Focus back_or_none() const;
    std::list<Focus> focus_stack_;
    Focus focus_merge_;
};

std::ostream& operator << (std::ostream& ostr, const Focuses& focuses);

struct SubmenuHeader {
    std::string title;
    std::string icon;
    BooleanExpression required;
};

enum class PersistedValueType {
    DEFAULT,
    CUSTOM
};

class UiFocus {
public:
    explicit UiFocus(std::string filename);
    ~UiFocus();
    UiFocus(const UiFocus&) = delete;
    UiFocus& operator = (const UiFocus&) = delete;
    Focuses focuses;
    std::map<std::string, std::atomic_size_t> menu_selection_ids;
    std::map<std::string, size_t> submenu_numbers;
    std::vector<SubmenuHeader> submenu_headers;
    std::vector<FocusFilter> focus_filters;
    std::map<std::string, std::atomic_size_t> all_selection_ids;
    void set_persisted_selection_id(const std::string& submenu, const std::string& s, PersistedValueType cb);
    std::string get_persisted_selection_id(const std::string& submenu) const;
    void set_requires_reload(std::string submenu, std::string reason);
    void clear_requires_reload(const std::string& submenu);
    const std::map<std::string, std::string>& requires_reload() const;
    void insert_submenu(
        const std::string& id,
        const SubmenuHeader& header,
        FocusFilter focus_filter,
        size_t default_selection);
    void try_push_back(Focus focus, const MacroLineExecutor& mle);
    bool has_focus(const FocusFilter& focus_filter) const;
    void clear();
    bool can_load() const;
    bool can_save() const;
    bool has_changes() const;
    void load();
    void save();
    void pop_invalid_focuses(const MacroLineExecutor& mle);
private:
    bool get_has_changes() const;
    std::map<std::string, ThreadSafeString> loaded_persistent_selection_ids;
    std::map<std::string, ThreadSafeString> current_persistent_selection_ids;
    std::string filename_;
    bool has_changes_;
    std::map<std::string, std::string> requires_reload_;
};

class UiFocuses {
public:
    explicit UiFocuses(std::string filename_prefix);
    ~UiFocuses();
    UiFocuses(const UiFocuses&) = delete;
    UiFocuses& operator = (const UiFocuses&) = delete;
    UiFocus& operator [] (uint32_t user_id);
    const UiFocus& operator [] (uint32_t user_id) const;
    void trim(uint32_t user_count);
    void try_load();
    void try_save();
    void clear();
    void clear_focuses();
    void pop_invalid_focuses(MacroLineExecutor& mle);
private:
    std::map<uint32_t, UiFocus> focuses_;
    std::string filename_prefix_;
};

Focus single_focus_from_string(const std::string& str);
Focus focus_from_string(const std::string& str);
std::string focus_to_string(Focus focus);

}
