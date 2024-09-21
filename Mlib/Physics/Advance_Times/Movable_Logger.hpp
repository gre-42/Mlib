#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Memory/Destruction_Observer.hpp>
#include <Mlib/Physics/Interfaces/IAdvance_Time.hpp>
#include <Mlib/Scene_Graph/Status_Writer.hpp>
#include <memory>

namespace Mlib {

template <class T>
class DanglingRef;
class SceneNode;

class MovableLogger: public DestructionObserver<SceneNode&>, public IAdvanceTime, public virtual DanglingBaseClass {
public:
    MovableLogger(
        DanglingRef<SceneNode> scene_node,
        StatusWriter& status_writer,
        StatusComponents log_components);
    ~MovableLogger();
    virtual void notify_destroyed(SceneNode& destroyed_object) override;
    virtual void advance_time(float dt, const StaticWorld& world) override;
private:
    StatusWriter& status_writer_;
    StatusComponents log_components_;
};

}
