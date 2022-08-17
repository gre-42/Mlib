#pragma once
#include <list>
#include <map>
#include <string>

namespace Mlib {

class RenderableScene;

class RenderableScenes {
    using map_type = std::map<std::string, RenderableScene>;
public:
    RenderableScenes();
    ~RenderableScenes();
    RenderableScenes(const RenderableScenes&) = delete;
    RenderableScenes& operator = (const RenderableScenes&) = delete;
    map_type::iterator begin();
    map_type::iterator end();
    RenderableScene& operator[](const std::string& name);
    const RenderableScene& operator[](const std::string& name) const;
    template<class... Args>
    std::pair<map_type::iterator, bool> try_emplace(const std::string& k, Args&&... args) {
        auto res = renderable_scenes_.try_emplace(k, std::forward<Args>(args)...);
        if (res.second) {
            renderable_scenes_name_list_.push_back(k);
        }
        return res;
    }
private:
    std::list<std::string> renderable_scenes_name_list_;
    map_type renderable_scenes_;
};

}
