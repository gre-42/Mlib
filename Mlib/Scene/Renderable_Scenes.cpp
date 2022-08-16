#include "Renderable_Scenes.hpp"
#include <Mlib/Scene/Renderable_Scene.hpp>

using namespace Mlib;

RenderableScenes::RenderableScenes()
{}

RenderableScenes::~RenderableScenes() {
    for (auto& [_, s] : renderable_scenes_) {
        s.stop_and_join();
    }
}

std::map<std::string, RenderableScene>::iterator RenderableScenes::begin() {
    return renderable_scenes_.begin();
}

std::map<std::string, RenderableScene>::iterator RenderableScenes::end() {
    return renderable_scenes_.end();
}

RenderableScene& RenderableScenes::operator[](const std::string& name) {
    auto wit = renderable_scenes_.find(name);
    if (wit == renderable_scenes_.end()) {
        throw std::runtime_error("Could not find renderable scene with name \"" + name + '"');
    }
    return wit->second;
}

const RenderableScene& RenderableScenes::operator[](const std::string& name) const {
    return const_cast<RenderableScenes&>(*this)[name];
}
