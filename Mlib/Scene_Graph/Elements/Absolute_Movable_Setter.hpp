#pragma once
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Memory/Optional_Cast.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Interfaces/Scene_Node/INode_Hider.hpp>

namespace Mlib {

template <class TAbsoluteMovable>
class AbsoluteMovableSetter {
    AbsoluteMovableSetter(const AbsoluteMovableSetter&) = delete;
    AbsoluteMovableSetter& operator = (const AbsoluteMovableSetter&) = delete;
public:
    AbsoluteMovableSetter(
        DanglingRef<SceneNode> node,
        std::unique_ptr<TAbsoluteMovable, DeleteFromPool<TAbsoluteMovable>>&& absolute_movable,
        DanglingBaseClassPtr<DestructionObserver<SceneNode&>> destruction_observer,
        DanglingBaseClassPtr<INodeHider> node_hider)
        : absolute_movable{ nullptr, nullptr }
        , absolute_movable_ptr_{ *absolute_movable, CURRENT_SOURCE_LOCATION }
        , destruction_observer_{ std::move(destruction_observer) }
        , node_hider_{ std::move(node_hider) }
        , node_{ node }
        , lock_{ node->mutex_ }
    {
        if (node->absolute_movable_ != nullptr) {
            THROW_OR_ABORT("Absolute movable already set (0)");
        }
        absolute_movable->set_absolute_model_matrix(node->absolute_model_matrix(LockingStrategy::NO_LOCK));
        // Initialize after check above so the absolute_movable unique_ptr is not
        // destroyed in case of an error.
        this->absolute_movable = std::move(absolute_movable);
    }
    AbsoluteMovableSetter(
        DanglingRef<SceneNode> node,
        std::unique_ptr<TAbsoluteMovable, DeleteFromPool<TAbsoluteMovable>>&& absolute_movable,
        SourceLocation loc)
        : AbsoluteMovableSetter{
            node,
            std::move(absolute_movable),
            { *absolute_movable, loc },
            { optional_cast<INodeHider*>(absolute_movable.get()), loc } }
    {}
    AbsoluteMovableSetter(
        DanglingRef<SceneNode> node,
        DanglingBaseClassRef<TAbsoluteMovable> absolute_movable)
        : absolute_movable_ptr_{ absolute_movable.ptr() }
        , destruction_observer_{ nullptr }
        , node_hider_{ nullptr }
        , node_{ node }
        , lock_{ node->mutex_ }
    {}
    ~AbsoluteMovableSetter() {
        if (absolute_movable == nullptr) {
            node_->set_absolute_movable(*absolute_movable_ptr_);
            if (destruction_observer_ != nullptr) {
                node_->clearing_observers.add(*destruction_observer_);
            }
            if (node_hider_ != nullptr) {
                node_->insert_node_hider(nullptr, *node_hider_);
            }
        }
    }
    std::unique_ptr<TAbsoluteMovable, DeleteFromPool<TAbsoluteMovable>> absolute_movable;
private:
    DanglingBaseClassPtr<IAbsoluteMovable> absolute_movable_ptr_;
    DanglingBaseClassPtr<DestructionObserver<SceneNode&>> destruction_observer_;
    DanglingBaseClassPtr<INodeHider> node_hider_;
    DanglingRef<SceneNode> node_;
    std::scoped_lock<SafeAtomicRecursiveSharedMutex> lock_;
};

}
