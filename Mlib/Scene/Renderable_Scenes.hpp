#pragma once
#include <map>
#include <string>

namespace Mlib {

class RenderableScene;

class RenderableScenes {
    using map_type = std::map<std::string, RenderableScene>;
public:
    RenderableScenes();
    ~RenderableScenes();
    map_type::iterator begin();
    map_type::iterator end();
    RenderableScene& operator[](const std::string& name);
    const RenderableScene& operator[](const std::string& name) const;
    template<class... Args>
    std::pair<map_type::iterator, bool> try_emplace(const std::string& k, Args&&... args) {
        return renderable_scenes_.try_emplace(k, std::forward<Args>(args)...);
    }
private:
    map_type renderable_scenes_;
};

}
