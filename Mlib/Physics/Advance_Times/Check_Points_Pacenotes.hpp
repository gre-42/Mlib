#pragma once
#include <Mlib/Memory/Destruction_Observer.hpp>
#include <Mlib/Physics/Interfaces/Advance_Time.hpp>
#include <Mlib/Physics/Misc/Pacenote_Reader.hpp>
#include <shared_mutex>

namespace Mlib {

class AdvanceTimes;
class SceneNode;
class CheckPoints;

class CheckPointsPacenotes: public DestructionObserver, public AdvanceTime {
public:
    CheckPointsPacenotes(
        const std::string& filename,
        const CheckPoints& check_points,
        size_t nlaps,
        size_t pacenotes_nread_ahead,
        AdvanceTimes& advance_times,
        SceneNode& moving_node);
    ~CheckPointsPacenotes();
    virtual void advance_time(float dt) override;
    virtual void notify_destroyed(const Object& destroyed_object) override;
    const Pacenote* pacenote() const;
private:
    const CheckPoints* check_points_;
    PacenoteReader pacenote_reader_;
    const Pacenote* pacenote_;
    AdvanceTimes& advance_times_;
    SceneNode* moving_node_;
    mutable std::shared_mutex mutex_;
};

}
