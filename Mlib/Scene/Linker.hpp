#pragma once
#include <Mlib/Physics/Containers/Advance_Times.hpp>
#include <Mlib/Scene_Graph/Elements/Absolute_Movable_Setter.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>

namespace Mlib {

class Linker {
public:
    explicit Linker(AdvanceTimes& advance_times)
    : advance_times_{advance_times}
    {}

    template <class TAbsoluteMovable>
    void link_absolute_movable_and_additional_node(
        DanglingRef<SceneNode> moved_node,
        DanglingRef<SceneNode> observed_node,
        std::unique_ptr<TAbsoluteMovable>&& absolute_movable) const
    {
        auto& am = *absolute_movable;
        // 1. Set movable, which updates the transformation-matrix.
        AbsoluteMovableSetter ams{moved_node, std::move(absolute_movable)};
        // 2. Add to physics engine.
        advance_times_.add_advance_time(std::move(ams.absolute_movable));
        // 3. Observe an additional node.
        observed_node->clearing_observers.add({ am, CURRENT_SOURCE_LOCATION });
    }

    template <class TAbsoluteMovable>
    void link_absolute_movable(DanglingRef<SceneNode> node, std::unique_ptr<TAbsoluteMovable>&& absolute_movable) const {
        // 1. Set movable, which updates the transformation-matrix.
        AbsoluteMovableSetter ams{node, std::move(absolute_movable)};
        // 2. Add to physics engine.
        advance_times_.add_advance_time(std::move(ams.absolute_movable));
    }

    template <class TRelativeMovable>
    void link_relative_movable(DanglingRef<SceneNode> node, std::unique_ptr<TRelativeMovable>&& relative_movable) const {
        node->set_relative_movable({*relative_movable, CURRENT_SOURCE_LOCATION});
        advance_times_.add_advance_time(std::move(relative_movable));
    }

private:
    AdvanceTimes& advance_times_;
};

}
