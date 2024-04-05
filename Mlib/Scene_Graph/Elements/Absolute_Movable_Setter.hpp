#pragma once
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <Mlib/Memory/Optional_Cast.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>

namespace Mlib {

template <class TAbsoluteMovable>
class AbsoluteMovableSetter {
    AbsoluteMovableSetter(const AbsoluteMovableSetter&) = delete;
    AbsoluteMovableSetter& operator = (const AbsoluteMovableSetter&) = delete;
public:
    AbsoluteMovableSetter(
        DanglingRef<SceneNode> node,
        std::unique_ptr<TAbsoluteMovable>&& absolute_movable,
        DanglingBaseClassPtr<DestructionObserver<DanglingRef<SceneNode>>> destruction_observer,
        INodeHider* node_hider)
        : absolute_movable_ptr_{ *absolute_movable, CURRENT_SOURCE_LOCATION }
        , destruction_observer_{ std::move(destruction_observer) }
        , node_hider_{ node_hider }
        , node_{ node }
        , lock_{ node->mutex_ }
    {
        if (node->absolute_movable_ != nullptr) {
            THROW_OR_ABORT("Absolute movable already set");
        }
        absolute_movable->set_absolute_model_matrix(node->absolute_model_matrix(LockingStrategy::NO_LOCK));
        // Initialize after check above so the absolute_movable unique_ptr is not
        // destroyed in case of an error.
        this->absolute_movable = std::move(absolute_movable);
    }
    AbsoluteMovableSetter(
        DanglingRef<SceneNode> node,
        std::unique_ptr<TAbsoluteMovable>&& absolute_movable)
    : AbsoluteMovableSetter{
        node,
        std::move(absolute_movable),
        { *absolute_movable, CURRENT_SOURCE_LOCATION },
        optional_cast<INodeHider*>(absolute_movable.get())}
    {}
    AbsoluteMovableSetter(
        DanglingRef<SceneNode> node,
        TAbsoluteMovable& absolute_movable)
    : absolute_movable_ptr_{absolute_movable.ptr()},
      destruction_observer_{nullptr},
      node_hider_{nullptr},
      node_{node},
      lock_{node->mutex_}
    {}
    ~AbsoluteMovableSetter() {
        if (absolute_movable == nullptr) {
            if (node_->absolute_movable_ != nullptr) {
                verbose_abort("Absolute movable already set (1)");
            }
            node_->absolute_movable_ = absolute_movable_ptr_;
            if (destruction_observer_ != nullptr) {
                node_->clearing_observers.add(*destruction_observer_);
            }
            if (node_hider_ != nullptr) {
                node_->insert_node_hider(*node_hider_);
            }
        }
    }
    std::unique_ptr<TAbsoluteMovable> absolute_movable;
private:
    DanglingBaseClassPtr<IAbsoluteMovable> absolute_movable_ptr_;
    DanglingBaseClassPtr<DestructionObserver<DanglingRef<SceneNode>>> destruction_observer_;
    INodeHider* node_hider_;
    DanglingRef<SceneNode> node_;
    std::scoped_lock<SafeRecursiveSharedMutex> lock_;
};

}
