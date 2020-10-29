#include "Check_Points.hpp"
#include <Mlib/Geometry/Homogeneous.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Physics/Containers/Advance_Times.hpp>
#include <Mlib/Physics/Containers/Players.hpp>
#include <Mlib/Scene_Graph/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Transformation/Absolute_Movable.hpp>
#include <Mlib/Time.hpp>

using namespace Mlib;

CheckPoints::CheckPoints(
    const std::string& filename,
    AdvanceTimes& advance_times,
    SceneNode* moving_node,
    AbsoluteMovable* moving,
    SceneNode* beacon_node0,
    SceneNode* beacon_node1,
    Players* players,
    Player* player,
    size_t nth,
    float radius)
: advance_times_{advance_times},
  track_reader_{filename},
  moving_node_{moving_node},
  moving_{moving},
  beacon_node0_{beacon_node0},
  beacon_node1_{beacon_node1},
  players_{players},
  player_{player},
  current_checkpoint_position_{fixed_nans<float, 3>()},
  radius_{radius},
  nth_{nth},
  i01_{0}
{
    moving_node_->add_destruction_observer(this);
    advance_time(0);
}

void CheckPoints::advance_time(float dt) {
    bool just_started = any(isnan(current_checkpoint_position_));

    if (just_started) {
        start_time_ = std::chrono::steady_clock::now();
    }

    if (just_started ||
        (sum(squared(t3_from_4x4(moving_->get_new_absolute_model_matrix()) - current_checkpoint_position_)) < squared(radius_)))
    {
        for(size_t i = 0; i < nth_; ++i) {
            float time;
            FixedArray<float, 3> rotation;
            if (track_reader_.read(time, current_checkpoint_position_, rotation)) {
                if (i == nth_ - 1) {
                    if (i01_ == 0) {
                        beacon_node0_->set_relative_pose(current_checkpoint_position_, rotation, 1);
                    } else {
                        beacon_node1_->set_relative_pose(current_checkpoint_position_, rotation, 1);
                    }
                    i01_ = (i01_ + 1) % 2;
                }
            }
        }
        if (track_reader_.eof() && !just_started) {
            std::chrono::duration<float> elapsed_time{std::chrono::steady_clock::now() - start_time_};
            std::cerr << "Elapsed time: " << format_minutes_seconds(elapsed_time.count()) << std::endl;
            players_->notify_lap_time(player_, elapsed_time.count());
            track_reader_.restart();
            current_checkpoint_position_ = NAN;
            advance_time(0);
        }
    }
}

void CheckPoints::notify_destroyed(void* obj) {
    moving_node_ = nullptr;
    moving_ = nullptr;
    advance_times_.schedule_delete_advance_time(this);
}
