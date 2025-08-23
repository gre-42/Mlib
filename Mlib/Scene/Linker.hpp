#pragma once
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Physics/Containers/Advance_Times.hpp>
#include <Mlib/Scene_Graph/Elements/Absolute_Movable_Setter.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>

namespace Mlib {

class Linker {
public:
    explicit Linker(AdvanceTimes& advance_times)
        : advance_times_{ advance_times }
    {}

    template <class TAbsoluteMovable>
    void link_absolute_movable_and_additional_node(
        Scene& scene,
        DanglingRef<SceneNode> moved_node,
        DanglingRef<SceneNode> observed_node,
        INodeSetter& moved_node_setter,
        INodeSetter& observed_node_setter,
        VariableAndHash<std::string> moved_node_name,
        VariableAndHash<std::string> observed_node_name,
        std::unique_ptr<TAbsoluteMovable, DeleteFromPool<TAbsoluteMovable>>&& absolute_movable,
        SourceLocation loc) const
    {
        // 1. Set movable, which updates the transformation-matrix.
        AbsoluteMovableSetter ams{ moved_node, std::move(absolute_movable), loc };
        // 2. Add to physics engine.
        advance_times_.add_advance_time({ *ams.absolute_movable, loc }, loc);
        // 3. Observe an additional node.
        moved_node_setter.set_scene_node(scene, std::move(moved_node), std::move(moved_node_name), loc);
        observed_node_setter.set_scene_node(scene, std::move(observed_node), std::move(observed_node_name), loc);
        ams.absolute_movable.release();
    }

    template <class TAbsoluteMovable>
    void link_absolute_movable(
        Scene& scene,
        DanglingRef<SceneNode> node,
        const VariableAndHash<std::string>& node_name,
        std::unique_ptr<TAbsoluteMovable, DeleteFromPool<TAbsoluteMovable>>&& absolute_movable,
        SourceLocation loc) const
    {
        // 1. Set movable, which updates the transformation-matrix.
        AbsoluteMovableSetter ams{ scene, node, node_name, std::move(absolute_movable), loc };
        // 2. Add to physics engine.
        advance_times_.add_advance_time({ *ams.absolute_movable, loc }, loc);
        ams.absolute_movable.release();
    }

    template <class TRelativeMovable>
    void link_relative_movable(
        const DanglingRef<SceneNode>& node,
        const DanglingBaseClassRef<TRelativeMovable>& relative_movable,
        SourceLocation loc) const
    {
        node->set_relative_movable(relative_movable.ptr());
        advance_times_.add_advance_time(relative_movable, loc);
    }

private:
    AdvanceTimes& advance_times_;
};

}
