#include "Renderable_Scenes.hpp"
#include <Mlib/Scene/Renderable_Scene.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

RenderableScenes::RenderableScenes() = default;

RenderableScenes::~RenderableScenes() {
    {
        std::shared_lock lock{mutex_};
        for (auto& [_, rs] : renderable_scenes_) {
            rs.stop_and_join();
        }
    }
    {
        std::scoped_lock lock{mutex_};
        for (const auto& name : renderable_scenes_name_list_) {
            renderable_scenes_.erase(name);
        }
    }
}

GuardedIterable<RenderableScenes::map_type::iterator> RenderableScenes::guarded_iterable() {
    return {mutex_, *this};
}

RenderableScenes::map_type::iterator RenderableScenes::unsafe_begin() {
    return renderable_scenes_.begin();
}

RenderableScenes::map_type::iterator RenderableScenes::unsafe_end() {
    return renderable_scenes_.end();
}

RenderableScene& RenderableScenes::operator[](const std::string& name) {
    std::shared_lock lock{mutex_};
    auto wit = renderable_scenes_.find(name);
    if (wit == renderable_scenes_.end()) {
        THROW_OR_ABORT("Could not find renderable scene with name \"" + name + '"');
    }
    return wit->second;
}

const RenderableScene& RenderableScenes::operator[](const std::string& name) const {
    return const_cast<RenderableScenes&>(*this)[name];
}

bool RenderableScenes::contains(const std::string& name) const {
    std::shared_lock lock{mutex_};
    return renderable_scenes_.contains(name);
}
