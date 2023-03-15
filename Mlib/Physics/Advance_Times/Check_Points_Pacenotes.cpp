#include "Check_Points_Pacenotes.hpp"
#include <Mlib/Physics/Advance_Times/Check_Points.hpp>
#include <Mlib/Physics/Containers/Advance_Times.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <mutex>

using namespace Mlib;

CheckPointsPacenotes::CheckPointsPacenotes(
    const std::string& filename,
    const CheckPoints& check_points,
    size_t nlaps,
    size_t pacenotes_nread_ahead,
    AdvanceTimes& advance_times,
    SceneNode& moving_node)
: check_points_{&check_points},
  pacenote_reader_{filename, nlaps, pacenotes_nread_ahead},
  pacenote_{nullptr},
  advance_times_{advance_times},
  moving_node_{&moving_node}
{
    moving_node_->destruction_observers.add(*this);
}

CheckPointsPacenotes::~CheckPointsPacenotes() {
    if (moving_node_ != nullptr) {
        moving_node_->destruction_observers.remove(*this);
    }
}

void CheckPointsPacenotes::advance_time(float dt) {
    if (check_points_ == nullptr) {
        return;
    }
    std::scoped_lock lock{mutex_};
    pacenote_ = pacenote_reader_.read(
        check_points_->frame_index(),
        check_points_->lap_index());
    // if (pacenote_ != nullptr) {
    //     linfo() << *pacenote_;
    // }
}

void CheckPointsPacenotes::notify_destroyed(const Object& destroyed_object) {
    check_points_ = nullptr;
    pacenote_ = nullptr;
    moving_node_ = nullptr;
    advance_times_.schedule_delete_advance_time(*this);
}

const Pacenote* CheckPointsPacenotes::pacenote() const {
    std::shared_lock lock{mutex_};
    return pacenote_;
}
