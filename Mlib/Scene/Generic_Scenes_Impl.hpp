#pragma once
#include "Generic_Scenes.hpp"
#include <Mlib/Throw_Or_Abort.hpp>

namespace Mlib {

template <class TScene>
GenericScenes<TScene>::GenericScenes(std::string collection_name)
    : collection_name_{ std::move(collection_name) }
    , state_{ GenericScenesState::RUNNING }
{}

template <class TScene>
GenericScenes<TScene>::~GenericScenes() {
    {
        // Set the state to "STOPPING", so the "try_emplace" routine
        // refuses to create new scenes.
        std::scoped_lock lock{ mutex_ };
        state_ = GenericScenesState::STOPPING;
    }
    {
        // Stop all scenes in arbitrary order and join them
        // (i.e. destroy their threads).
        std::shared_lock lock{ mutex_ };
        for (auto& [_, rs] : generic_scenes_) {
            rs.stop_and_join();
        }
    }
    // Destroy scene nodes in order of creation.
    for (const auto& name : generic_scenes_name_list_) {
        (*this)[name].clear();
    }
    {
        // Set the state to "SHUTTING_DOWN", so index-access to scenes
        // results in an exception.
        std::scoped_lock lock{ mutex_ };
        state_ = GenericScenesState::SHUTTING_DOWN;
    }
    // Erase scenes in order of creation.
    // This requires that dtors are implemented s.t. dtors of new scenes
    // are called after destructors of the old scenes.
    for (const auto& name : generic_scenes_name_list_) {
        if (generic_scenes_.erase(name) != 1) {
            verbose_abort(collection_name_ + "Could not delete scene");
        }
    }
    if (!generic_scenes_.empty()) {
        verbose_abort(collection_name_ + "Scenes not empty");
    }
}

template <class TScene>
GuardedIterable<typename GenericScenes<TScene>::map_type::iterator, std::shared_lock<SafeAtomicRecursiveSharedMutex>>
    GenericScenes<TScene>::guarded_iterable()
{
    if (shutting_down()) {
        verbose_abort(collection_name_ + "Shutting down");
    }
    return { mutex_, unsafe_begin(), unsafe_end() };
}

template <class TScene>
GuardedIterable<typename GenericScenes<TScene>::map_type::const_iterator, std::shared_lock<SafeAtomicRecursiveSharedMutex>>
    GenericScenes<TScene>::guarded_iterable() const
{
    if (shutting_down()) {
        verbose_abort(collection_name_ + "Shutting down");
    }
    return { mutex_, unsafe_begin(), unsafe_end() };
}

template <class TScene>
GenericScenes<TScene>::map_type::iterator GenericScenes<TScene>::unsafe_begin() {
    return generic_scenes_.begin();
}

template <class TScene>
GenericScenes<TScene>::map_type::iterator GenericScenes<TScene>::unsafe_end() {
    return generic_scenes_.end();
}

template <class TScene>
GenericScenes<TScene>::map_type::const_iterator GenericScenes<TScene>::unsafe_begin() const {
    return generic_scenes_.begin();
}

template <class TScene>
GenericScenes<TScene>::map_type::const_iterator GenericScenes<TScene>::unsafe_end() const {
    return generic_scenes_.end();
}

template <class TScene>
TScene& GenericScenes<TScene>::operator[](const std::string& name) {
    std::shared_lock lock{ mutex_ };
    if (shutting_down()) {
        verbose_abort(collection_name_ + "Scenes are shutting down (0)");
    }
    auto wit = generic_scenes_.find(name);
    if (wit == generic_scenes_.end()) {
        THROW_OR_ABORT(collection_name_ + "Could not find scene with name \"" + name + '"');
    }
    return wit->second;
}

template <class TScene>
const TScene& GenericScenes<TScene>::operator[](const std::string& name) const {
    return const_cast<GenericScenes&>(*this)[name];
}

template <class TScene>
TScene* GenericScenes<TScene>::try_get(const std::string& name) {
    std::shared_lock lock{ mutex_ };
    if (shutting_down()) {
        verbose_abort(collection_name_ + "Scenes are shutting down (1)");
    }
    auto res = generic_scenes_.find(name);
    if (res == generic_scenes_.end()) {
        return nullptr;
    }
    return &res->second;
}

template <class TScene>
const TScene* GenericScenes<TScene>::try_get(const std::string& name) const {
    return const_cast<GenericScenes*>(this)->try_get(name);
}

template <class TScene>
bool GenericScenes<TScene>::shutting_down() const {
    return state_ == GenericScenesState::SHUTTING_DOWN;
}

}
