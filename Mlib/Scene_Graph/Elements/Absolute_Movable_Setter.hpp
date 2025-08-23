#pragma once
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Memory/Optional_Cast.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Interfaces/INode_Setter.hpp>
#include <Mlib/Variable_And_Hash.hpp>

namespace Mlib {

class Scene;

template <class TAbsoluteMovable>
class AbsoluteMovableSetter {
    AbsoluteMovableSetter(const AbsoluteMovableSetter&) = delete;
    AbsoluteMovableSetter& operator = (const AbsoluteMovableSetter&) = delete;
public:
    AbsoluteMovableSetter(
        Scene& scene,
        DanglingRef<SceneNode> node,
        VariableAndHash<std::string> node_name,
        std::unique_ptr<TAbsoluteMovable, DeleteFromPool<TAbsoluteMovable>>&& absolute_movable,
        DanglingBaseClassPtr<INodeSetter> node_setter,
        SourceLocation loc)
        : absolute_movable{ nullptr, nullptr }
        , scene_{ &scene }
        , node_name_{ std::move(node_name) }
        , absolute_movable_ptr_{ *absolute_movable, CURRENT_SOURCE_LOCATION }
        , node_setter_{ std::move(node_setter) }
        , node_{ node }
        , lock_{ node->mutex_ }
        , loc_{ loc }
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
        : absolute_movable{ nullptr, nullptr }
        , scene_{ nullptr }
        , absolute_movable_ptr_{ *absolute_movable, CURRENT_SOURCE_LOCATION }
        , node_setter_{ nullptr }
        , node_{ node }
        , lock_{ node->mutex_ }
        , loc_{ loc }
    {
        if (node->absolute_movable_ != nullptr) {
            THROW_OR_ABORT("Absolute movable already set (1)");
        }
        absolute_movable->set_absolute_model_matrix(node->absolute_model_matrix(LockingStrategy::NO_LOCK));
        // Initialize after check above so the absolute_movable unique_ptr is not
        // destroyed in case of an error.
        this->absolute_movable = std::move(absolute_movable);
    }
    AbsoluteMovableSetter(
        Scene& scene,
        DanglingRef<SceneNode> node,
        VariableAndHash<std::string> node_name,
        std::unique_ptr<TAbsoluteMovable, DeleteFromPool<TAbsoluteMovable>>&& absolute_movable,
        SourceLocation loc)
        : AbsoluteMovableSetter{
            scene,
            std::move(node),
            std::move(node_name),
            std::move(absolute_movable),
            { *absolute_movable, loc },
            loc }
    {}
    AbsoluteMovableSetter(
        DanglingRef<SceneNode> node,
        DanglingBaseClassRef<TAbsoluteMovable> absolute_movable,
        SourceLocation loc)
        : absolute_movable_ptr_{ absolute_movable.ptr() }
        , scene_{ nullptr }
        , node_setter_{ nullptr }
        , node_{ node }
        , lock_{ node->mutex_ }
        , loc_{ loc }
    {}
    ~AbsoluteMovableSetter() {
        if (absolute_movable == nullptr) {
            if (node_setter_ != nullptr) {
                if (scene_ == nullptr) {
                    verbose_abort("~AbsoluteMovableSetter: Scene not set");
                }
                node_setter_->set_scene_node(
                    *scene_,
                    node_,
                    node_name_,
                    CURRENT_SOURCE_LOCATION);
            }
        }
    }
    std::unique_ptr<TAbsoluteMovable, DeleteFromPool<TAbsoluteMovable>> absolute_movable;
private:
    Scene* scene_;
    VariableAndHash<std::string> node_name_;
    DanglingBaseClassPtr<IAbsoluteMovable> absolute_movable_ptr_;
    DanglingBaseClassPtr<INodeSetter> node_setter_;
    DanglingRef<SceneNode> node_;
    std::scoped_lock<SafeAtomicRecursiveSharedMutex> lock_;
    SourceLocation loc_;
};

}
