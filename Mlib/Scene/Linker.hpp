#pragma once
#include <Mlib/Physics/Containers/Advance_Times.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>

namespace Mlib {

class Linker {
public:
    explicit Linker(AdvanceTimes& advance_times)
    : advance_times_{advance_times}
    {}

    template <class TAbsoluteMovable>
    void link_absolute_movable(SceneNode& node, const std::shared_ptr<TAbsoluteMovable>& absolute_movable) const {
        // 1. Set movable, which updates the transformation-matrix
        node.set_absolute_movable(absolute_movable.get());
        // 2. Add to physics engine
        advance_times_.add_advance_time(absolute_movable);
    };
    template <class TRelativeMovable>
    void link_relative_movable(SceneNode& node, const std::shared_ptr<TRelativeMovable>& relative_movable) const {
        node.set_relative_movable(relative_movable.get());
        advance_times_.add_advance_time(relative_movable);
    };
    template <class TAbsoluteObserver>
    void link_absolute_observer(SceneNode& node, const std::shared_ptr<TAbsoluteObserver>& absolute_observer) const {
        node.set_absolute_observer(absolute_observer.get());
        advance_times_.add_advance_time(absolute_observer);
    };
private:
    AdvanceTimes& advance_times_;
};

}
