#include "Renderable_Scenes.hpp"
#include <Mlib/Scene/Renderable_Scene.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

RenderableScenes::RenderableScenes()
    : state_{ RenderableScenesState::RUNNING }
{}

RenderableScenes::~RenderableScenes() {
    {
        // Set the state to "STOPPING", so the "try_emplace" routine
        // refuses to create new scenes.
        std::scoped_lock lock{ mutex_ };
        state_ = RenderableScenesState::STOPPING;
    }
    {
        // Stop all scenes in arbitrary order and join them
        // (i.e. destroy their threads).
        std::shared_lock lock{ mutex_ };
        for (auto& [_, rs] : renderable_scenes_) {
            rs.stop_and_join();
        }
    }
    // Destroy scene nodes in order of creation.
    for (const auto& name : renderable_scenes_name_list_) {
        (*this)[name].clear();
    }
    {
        // Set the state to "SHUTTING_DOWN", so index-access to scenes
        // results in an exception.
        std::scoped_lock lock{ mutex_ };
        state_ = RenderableScenesState::SHUTTING_DOWN;
    }
    // Erase scenes in order of creation.
    // This requires that dtors are implemented s.t. dtors of new scenes
    // are called after destructors of the old scenes.
    for (const auto& name : renderable_scenes_name_list_) {
        if (renderable_scenes_.erase(name) != 1) {
            verbose_abort("Could not delete renderable scene");
        }
    }
    if (!renderable_scenes_.empty()) {
        verbose_abort("Renderable scenes not empty");
    }
}

GuardedIterable<RenderableScenes::map_type::iterator, std::shared_lock<SafeAtomicRecursiveSharedMutex>> RenderableScenes::guarded_iterable() {
    if (shutting_down()) {
        verbose_abort("RenderableScenes shutting down");
    }
    return { mutex_, unsafe_begin(), unsafe_end() };
}

RenderableScenes::map_type::iterator RenderableScenes::unsafe_begin() {
    return renderable_scenes_.begin();
}

RenderableScenes::map_type::iterator RenderableScenes::unsafe_end() {
    return renderable_scenes_.end();
}

RenderableScene& RenderableScenes::operator[](const std::string& name) {
    std::shared_lock lock{ mutex_ };
    if (shutting_down()) {
        verbose_abort("Renderable scenes are shutting down (0)");
    }
    auto wit = renderable_scenes_.find(name);
    if (wit == renderable_scenes_.end()) {
        THROW_OR_ABORT("Could not find renderable scene with name \"" + name + '"');
    }
    return wit->second;
}

const RenderableScene& RenderableScenes::operator[](const std::string& name) const {
    return const_cast<RenderableScenes&>(*this)[name];
}

RenderableScene* RenderableScenes::try_get(const std::string& name) {
    std::shared_lock lock{ mutex_ };
    if (shutting_down()) {
        verbose_abort("Renderable scenes are shutting down (1)");
    }
    auto res = renderable_scenes_.find(name);
    if (res == renderable_scenes_.end()) {
        return nullptr;
    }
    return &res->second;
}

const RenderableScene* RenderableScenes::try_get(const std::string& name) const {
    return const_cast<RenderableScenes*>(this)->try_get(name);
}

bool RenderableScenes::shutting_down() const {
    return state_ == RenderableScenesState::SHUTTING_DOWN;
}
