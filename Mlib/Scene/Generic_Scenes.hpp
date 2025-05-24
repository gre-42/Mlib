#pragma once
#include <Mlib/Iterator/Guarded_Iterable.hpp>
#include <Mlib/Threads/Recursive_Shared_Mutex.hpp>
#include <list>
#include <map>
#include <mutex>
#include <string>

namespace Mlib {

enum class GenericScenesState {
    RUNNING,
    STOPPING,
    SHUTTING_DOWN
};

enum class InsertionStatus {
    SUCCESS,
    FAILURE_NAME_COLLISION,
    FAILURE_SHUTDOWN
};

template <class TScene>
class GenericScenes {
    GenericScenes(const GenericScenes&) = delete;
    GenericScenes& operator = (const GenericScenes&) = delete;
public:
    using map_type = std::map<std::string, TScene>;
    explicit GenericScenes(std::string collection_name);
    ~GenericScenes();
    GuardedIterable<typename map_type::iterator, std::shared_lock<SafeAtomicRecursiveSharedMutex>> guarded_iterable();
    GuardedIterable<typename map_type::const_iterator, std::shared_lock<SafeAtomicRecursiveSharedMutex>> guarded_iterable() const;
    map_type::iterator unsafe_begin();
    map_type::iterator unsafe_end();
    map_type::const_iterator unsafe_begin() const;
    map_type::const_iterator unsafe_end() const;
    TScene& operator[](const std::string& name);
    const TScene& operator[](const std::string& name) const;
    TScene* try_get(const std::string& name);
    const TScene* try_get(const std::string& name) const;
    template<class... Args>
    std::pair<typename map_type::iterator, InsertionStatus> try_emplace(const std::string& k, Args&&... args) {
        // 1. Construct the scene without the "mutex_"
        //    The ctor will lock mutexes in the renderables and should therefore not
        //    lock the "mutex_" variable.
        map_type tmp;
        tmp.try_emplace(k, std::forward<Args>(args)...);
        // auto rs = std::make_unique<TScene>(std::forward<Args>(args)...);
        // 2. Acquire the "mutex_" and append the scene to the list of scenes.
        std::scoped_lock lock{ mutex_ };
        if (state_ != GenericScenesState::RUNNING) {
            return { generic_scenes_.end(), InsertionStatus::FAILURE_SHUTDOWN };
        }
        // auto res = generic_scenes_.try_emplace(k, std::forward<Args>(args)...);
        // auto res = generic_scenes_.insert({k, std::move(rs)});
        auto res = generic_scenes_.insert(tmp.extract(k));
        if (res.inserted) {
            generic_scenes_name_list_.push_back(k);
            return { res.position, InsertionStatus::SUCCESS };
        }
        return { res.position, InsertionStatus::FAILURE_NAME_COLLISION };
    }
    bool shutting_down() const;
private:
    std::string collection_name_;
    GenericScenesState state_;
    std::list<std::string> generic_scenes_name_list_;
    map_type generic_scenes_;
    mutable SafeAtomicRecursiveSharedMutex mutex_;
};

}
