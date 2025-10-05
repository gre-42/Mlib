#pragma once
#include <Mlib/Macro_Executor/Boolean_Expression.hpp>
#include <Mlib/Macro_Executor/Notifying_Json_Macro_Arguments.hpp>
#include <Mlib/Threads/Recursive_Shared_Mutex.hpp>
#include <atomic>
#include <functional>
#include <iosfwd>
#include <list>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace Mlib {

struct FocusFilter;

enum class Focus {
    // Focuses
    NONE = 0,
    MAIN_MENU = 1 << 0,
    NEW_GAME_MENU = 1 << 1,
    SETTINGS_MENU = 1 << 2,
    CONTROLS_MENU = 1 << 3,
    LOADING = 1 << 4,
    SCENE = 1 << 5,
    GAME_OVER = 1 << 6,
    MENU_ANY = MAIN_MENU | NEW_GAME_MENU | SETTINGS_MENU | CONTROLS_MENU,
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
    bool game_over() const;
    bool empty() const;
    size_t size() const;
    mutable SafeAtomicRecursiveSharedMutex mutex;
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
    std::function<bool()> required_;
    inline bool required() const {
        return !required_ || required_();
    }
};

enum class PersistedValueType {
    DEFAULT,
    CUSTOM
};

struct EditFocus {
    std::string menu_id;
    std::string entry_id;
    std::vector<std::string> persisted;
    std::vector<std::string> global;
    std::string value;
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
    std::optional<EditFocus> editing;
    void set_persisted_selection_id(
        const std::string_view& submenu,
        const nlohmann::json& s,
        PersistedValueType cb);
    void set_persisted_selection_id(
        const std::vector<std::string>& submenu,
        const nlohmann::json& s,
        PersistedValueType cb);
    nlohmann::json get_persisted_selection_id(const std::string& submenu) const;
    void set_requires_reload(std::string submenu, std::string reason);
    void clear_requires_reload(const std::string& submenu);
    const std::map<std::string, std::string>& requires_reload() const;
    SubmenuHeader& insert_submenu(
        const std::string& id,
        const SubmenuHeader& header,
        FocusFilter focus_filter,
        size_t default_selection);
    void try_push_back(Focus focus);
    bool has_focus(const FocusFilter& focus_filter) const;
    void clear();
    bool can_load() const;
    bool can_save() const;
    bool has_changes() const;
    void load();
    void save();
    void pop_invalid_focuses();
    FastMutex edit_mutex;
private:
    bool get_has_changes() const;
    template <JsonKey Key>
    void set_persisted_selection_id_generic(const Key& submenu, const nlohmann::json& s, PersistedValueType cb);
    NotifyingJsonMacroArguments loaded_persistent_selection_ids;
    NotifyingJsonMacroArguments current_persistent_selection_ids;
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
    void pop_invalid_focuses();
private:
    std::map<uint32_t, UiFocus> focuses_;
    std::string filename_prefix_;
};

Focus single_focus_from_string(const std::string& str);
Focus focus_from_string(const std::string& str);
std::string focus_to_string(Focus focus);

}
