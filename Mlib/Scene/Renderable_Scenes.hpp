#pragma once
#include <Mlib/Scene/Renderable_Scene.hpp>
#include <Mlib/Threads/Recursive_Shared_Mutex.hpp>
#include <list>
#include <map>
#include <mutex>
#include <string>

namespace Mlib {

class RenderableScene;

template <class TIterator>
class GuardedIterable {
    GuardedIterable(const GuardedIterable&) = delete;
    GuardedIterable& operator = (const GuardedIterable&) = delete;
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
    RenderableScenes(const RenderableScenes&) = delete;
    RenderableScenes& operator = (const RenderableScenes&) = delete;
public:
    RenderableScenes();
    ~RenderableScenes();
    GuardedIterable<map_type::iterator> guarded_iterable();
    map_type::iterator unsafe_begin();
    map_type::iterator unsafe_end();
    RenderableScene& operator[](const std::string& name);
    const RenderableScene& operator[](const std::string& name) const;
    bool contains(const std::string& name) const;
    template<class... Args>
    std::pair<map_type::iterator, bool> try_emplace(const std::string& k, Args&&... args) {
        map_type tmp;
        tmp.try_emplace(k, std::forward<Args>(args)...);
        // auto rs = std::make_unique<RenderableScene>(std::forward<Args>(args)...);
        std::scoped_lock lock{mutex_};
        // auto res = renderable_scenes_.try_emplace(k, std::forward<Args>(args)...);
        // auto res = renderable_scenes_.insert({k, std::move(rs)});
        auto res = renderable_scenes_.insert(tmp.extract(k));
        if (res.inserted) {
            renderable_scenes_name_list_.push_back(k);
        }
        return { res.position, res.inserted };
    }
private:
    std::list<std::string> renderable_scenes_name_list_;
    map_type renderable_scenes_;
    mutable RecursiveSharedMutex mutex_;
};

}
