#pragma once
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>

namespace Mlib {

template <class TAbsoluteMovable>
class AbsoluteMovableSetter {
    AbsoluteMovableSetter(const AbsoluteMovableSetter&) = delete;
    AbsoluteMovableSetter& operator = (const AbsoluteMovableSetter&) = delete;
public:
    AbsoluteMovableSetter(
        SceneNode& node,
        std::unique_ptr<TAbsoluteMovable>&& absolute_movable,
        DestructionObserver* destruction_observer)
    : absolute_movable{std::move(absolute_movable)},
      absolute_movable_ptr_{this->absolute_movable.get()},
      destruction_observer_{destruction_observer},
      node_{node},
      lock_{node.mutex_}
    {
        if (node.absolute_movable_ != nullptr) {
            THROW_OR_ABORT("Absolute movable already set");
        }
        this->absolute_movable->set_absolute_model_matrix(node.absolute_model_matrix());
    }
    AbsoluteMovableSetter(
        SceneNode& node,
        std::unique_ptr<TAbsoluteMovable>&& absolute_movable)
    : AbsoluteMovableSetter{node, std::move(absolute_movable), absolute_movable.get()}
    {}
    ~AbsoluteMovableSetter() {
        if (absolute_movable == nullptr) {
            if (node_.absolute_movable_ != nullptr) {
                verbose_abort("Absolute movable already set (1)");
            }
            node_.absolute_movable_ = absolute_movable_ptr_;
            if (destruction_observer_ != nullptr) {
                node_.destruction_observers.add(*destruction_observer_);
            }
        }
    }
    std::unique_ptr<TAbsoluteMovable> absolute_movable;
private:
    AbsoluteMovable* absolute_movable_ptr_;
    DestructionObserver* destruction_observer_;
    SceneNode& node_;
    std::unique_lock<RecursiveSharedMutex> lock_;
};

}