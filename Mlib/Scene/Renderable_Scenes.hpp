#pragma once
#include <Mlib/Threads/Recursive_Shared_Mutex.hpp>
#include <list>
#include <map>
#include <mutex>
#include <string>

namespace Mlib {

class RenderableScene;

template <class TIterator>
class GuardedIterable {
public:
    template <class TContainer>
    GuardedIterable(RecursiveSharedMutex& mutex, TContainer& container)
    : lock_{mutex},
      begin_{container.unsafe_begin()},
      end_{container.unsafe_end()}
    {}
    TIterator begin() {
        return begin_;
    }
    TIterator end() {
        return end_;
    }
private:
    std::shared_lock<RecursiveSharedMutex> lock_;
    TIterator begin_;
    TIterator end_;
};

class RenderableScenes {
    using map_type = std::map<std::string, RenderableScene>;
public:
    RenderableScenes();
    ~RenderableScenes();
    RenderableScenes(const RenderableScenes&) = delete;
    RenderableScenes& operator = (const RenderableScenes&) = delete;
    GuardedIterable<map_type::iterator> guarded_iterable();
    map_type::iterator unsafe_begin();
    map_type::iterator unsafe_end();
    RenderableScene& operator[](const std::string& name);
    const RenderableScene& operator[](const std::string& name) const;
    bool contains(const std::string& name) const;
    template<class... Args>
    std::pair<map_type::iterator, bool> try_emplace(const std::string& k, Args&&... args) {
        std::unique_lock lock{mutex_};
        auto res = renderable_scenes_.try_emplace(k, std::forward<Args>(args)...);
        if (res.second) {
            renderable_scenes_name_list_.push_back(k);
        }
        return res;
    }
private:
    std::list<std::string> renderable_scenes_name_list_;
    map_type renderable_scenes_;
    mutable RecursiveSharedMutex mutex_;
};

}
