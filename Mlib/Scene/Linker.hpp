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
        DanglingRef<SceneNode> moved_node,
        DanglingRef<SceneNode> observed_node,
        std::unique_ptr<TAbsoluteMovable, DeleteFromPool<TAbsoluteMovable>>&& absolute_movable,
        SourceLocation loc) const
    {
        // 1. Set movable, which updates the transformation-matrix.
        AbsoluteMovableSetter ams{ moved_node, std::move(absolute_movable), loc };
        // 2. Add to physics engine.
        advance_times_.add_advance_time({ *ams.absolute_movable, loc }, loc);
        // 3. Observe an additional node.
        observed_node->clearing_observers.add({ *ams.absolute_movable, loc });
        ams.absolute_movable.release();
    }

    template <class TAbsoluteMovable>
    void link_absolute_movable(
        DanglingRef<SceneNode> node,
        std::unique_ptr<TAbsoluteMovable, DeleteFromPool<TAbsoluteMovable>>&& absolute_movable,
        SourceLocation loc) const
    {
        // 1. Set movable, which updates the transformation-matrix.
        AbsoluteMovableSetter ams{ node, std::move(absolute_movable), loc };
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
