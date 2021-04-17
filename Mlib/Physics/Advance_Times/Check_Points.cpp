#include "Check_Points.hpp"
#include <Mlib/Geometry/Homogeneous.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Math/Transformation_Matrix.hpp>
#include <Mlib/Physics/Containers/Advance_Times.hpp>
#include <Mlib/Physics/Containers/Players.hpp>
#include <Mlib/Scene_Graph/Scene.hpp>
#include <Mlib/Scene_Graph/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resource.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>
#include <Mlib/Scene_Graph/Transformation/Absolute_Movable.hpp>
#include <Mlib/Time.hpp>

using namespace Mlib;

CheckPoints::CheckPoints(
    const std::string& filename,
    AdvanceTimes& advance_times,
    SceneNode* moving_node,
    AbsoluteMovable* moving,
    const std::string& resource_name,
    Players* players,
    Player* player,
    size_t nbeacons,
    size_t nth,
    size_t nahead,
    float radius,
    SceneNodeResources& scene_node_resources,
    Scene& scene,
    bool enable_height_changed_mode)
: advance_times_{advance_times},
  track_reader_{filename},
  moving_node_{moving_node},
  moving_{moving},
  resource_name_{resource_name},
  players_{players},
  player_{player},
  radius_{radius},
  nbeacons_{nbeacons},
  nth_{nth},
  nahead_{nahead},
  i01_{0},
  scene_node_resources_{scene_node_resources},
  scene_{scene},
  enable_height_changed_mode_{enable_height_changed_mode}
{
    if (nbeacons == 0) {
        throw std::runtime_error("Need at least one beacon node");
    }
    if (nth_ == 0) {
        throw std::runtime_error("nth is 0");
    }
    beacon_nodes_.reserve(nbeacons);
    advance_time(0);
    // "moving_node_->add_destruction_observer" must be at the end of the constructor
    // in case the ctor throws an exception, because in this case CheckPoints object is not
    // added to the "advance_times" list.
    moving_node_->add_destruction_observer(this);
}

CheckPoints::~CheckPoints() {
    // Scene destruction happens before physics destruction,
    // so the checkpoints might already have been deleted.
    static const DECLARE_REGEX(re, "^check_point_beacon_.*");
    scene_.delete_root_nodes(re);
}

void CheckPoints::advance_time(float dt) {
    bool just_started = checkpoints_ahead_.empty();

    if (just_started) {
        start_time_ = std::chrono::steady_clock::now();
    }

    while ((checkpoints_ahead_.size() < nahead_) && (!track_reader_.eof())) {
        for (size_t i = 0; i < nth_; ++i) {
            float time;
            FixedArray<float, 3> position;
            FixedArray<float, 3> rotation;
            if (track_reader_.read(time, position, rotation) &&
                (i == nth_ - 1))
            {
                checkpoints_ahead_.push_back({.position = position, .rotation = rotation});
                if (i01_ == beacon_nodes_.size()) {
                    std::unique_ptr<SceneNode> node{new SceneNode};
                    scene_node_resources_.instantiate_renderable(resource_name_, "check_point_beacon_" + std::to_string(i01_), *node, SceneNodeResourceFilter());
                    scene_.add_root_node("check_point_beacon_" + std::to_string(i01_), node.get());
                    beacon_nodes_.push_back(node.release());
                }
                beacon_nodes_[i01_]->set_relative_pose(position, rotation, 1);
                i01_ = (i01_ + 1) % nbeacons_;
            }
        }
    }

    if (enable_height_changed_mode_) {
        for (SceneNode* b : beacon_nodes_) {
            auto pos = b->position();
            pos(1) = moving_node_->position()(1);
            b->set_position(pos);
        }
        checkpoints_ahead_.front().position(1) = moving_node_->position()(1);
    }

    if (!checkpoints_ahead_.empty() &&
        (sum(squared(moving_->get_new_absolute_model_matrix().t() - checkpoints_ahead_.front().position)) < squared(radius_)))
    {
        checkpoints_ahead_.pop_front();
    }

    if (!just_started && checkpoints_ahead_.empty() && track_reader_.eof())
    {
        std::chrono::duration<float> elapsed_time{std::chrono::steady_clock::now() - start_time_};
        std::cerr << "Elapsed time: " << format_minutes_seconds(elapsed_time.count()) << std::endl;
        players_->notify_lap_time(player_, elapsed_time.count());
        track_reader_.restart();
        advance_time(0);
    }
}

void CheckPoints::notify_destroyed(void* obj) {
    moving_node_ = nullptr;
    moving_ = nullptr;
    advance_times_.schedule_delete_advance_time(this);
}
