#include "Check_Points.hpp"
#include <Mlib/Geometry/Coordinates/Homogeneous.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Math/Transformation_Matrix.hpp>
#include <Mlib/Physics/Containers/Advance_Times.hpp>
#include <Mlib/Physics/Containers/Race_State.hpp>
#include <Mlib/Physics/Interfaces/IPlayer.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Delete_Node_Mutex.hpp>
#include <Mlib/Scene_Graph/Elements/Color_Style.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>
#include <Mlib/Scene_Graph/Instantiation_Options.hpp>
#include <Mlib/Scene_Graph/Renderable_Resource_Filter.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resource.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>
#include <Mlib/Scene_Graph/Transformation/Absolute_Movable.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <Mlib/Time.hpp>

using namespace Mlib;

CheckPoints::CheckPoints(
    const std::string& filename,
    size_t nlaps,
    const TransformationMatrix<double, double, 3>* inverse_geographic_mapping,
    AdvanceTimes& advance_times,
    SceneNode& moving_node,
    AbsoluteMovable& moving,
    const std::string& resource_name,
    IPlayer& player,
    size_t nbeacons,
    size_t nth,
    size_t nahead,
    float radius,
    SceneNodeResources& scene_node_resources,
    Scene& scene,
    DeleteNodeMutex& delete_node_mutex,
    const Focuses& focuses,
    bool enable_height_changed_mode,
    const FixedArray<float, 3>& selection_emissivity,
    const FixedArray<float, 3>& deselection_emissivity,
    const std::function<void()>& on_finish)
: advance_times_{advance_times},
  track_reader_{filename, nlaps, inverse_geographic_mapping},
  moving_node_{&moving_node},
  moving_{&moving},
  resource_name_{resource_name},
  player_{player},
  radius_{radius},
  nbeacons_{nbeacons},
  nth_{nth},
  nahead_{nahead},
  i01_{0},
  scene_node_resources_{scene_node_resources},
  scene_{scene},
  delete_node_mutex_{delete_node_mutex},
  focuses_{focuses},
  total_elapsed_seconds_{NAN},
  lap_elapsed_seconds_{NAN},
  race_state_{RaceState::ONGOING},
  enable_height_changed_mode_{enable_height_changed_mode},
  selection_emissivity_{selection_emissivity},
  deselection_emissivity_{deselection_emissivity},
  on_finish_{on_finish}
{
    if (nbeacons == 0) {
        THROW_OR_ABORT("Need at least one beacon node");
    }
    if (nth_ == 0) {
        THROW_OR_ABORT("nth is 0");
    }
    beacon_nodes_.reserve(nbeacons);
    advance_time(0);
    // "moving_node_->destruction_observers.add" must be at the end of the constructor
    // in case the ctor throws an exception, because in this case CheckPoints object is not
    // added to the "advance_times" list.
    moving_node_->destruction_observers.add(*this);
}

CheckPoints::~CheckPoints() = default;

void CheckPoints::advance_time(float dt) {
    if (moving_node_ == nullptr || moving_ == nullptr) {
        return;
    }
    {
        std::shared_lock lock{focuses_.mutex};
        if (focuses_.countdown_active()) {
            return;
        }
    }
    if (race_state_ == RaceState::ONGOING) {
        bool just_started = checkpoints_ahead_.empty();
        if (just_started) {
            total_elapsed_seconds_ = 0.f;
            lap_elapsed_seconds_ = 0.f;
            player_.notify_race_started();
        }
        total_elapsed_seconds_ += dt / s;
        lap_elapsed_seconds_ += dt / s;
    }

    auto am = moving_->get_new_absolute_model_matrix();
    movable_track_.push_back(TrackElement{
        .elapsed_seconds = total_elapsed_seconds_,
        .position = am.t(),
        .rotation = matrix_2_tait_bryan_angles(am.R())});
    while ((checkpoints_ahead_.size() < nahead_) && (!track_reader_.eof())) {
        size_t nperiods = lap_times_seconds_.size();
        for (size_t i = 0; i < nth_; ++i) {
            TrackElement track_element;
            if (track_reader_.read(track_element, nperiods, dt) &&
                (i == nth_ - 1))
            {
                checkpoints_ahead_.push_back(CheckPointPose{
                    .position = track_element.position,
                    .rotation = track_element.rotation,
                    .nperiods = nperiods});
                if (i01_ == beacon_nodes_.size()) {
                    auto node = std::make_unique<SceneNode>();
                    node->add_color_style(std::make_unique<ColorStyle>(ColorStyle{.selector = Mlib::compile_regex("")}));
                    beacon_nodes_.push_back(BeaconNode{ .beacon_node = node.get() });
                    scene_node_resources_.instantiate_renderable(
                        resource_name_,
                        InstantiationOptions{
                            .instance_name = "check_point_beacon_" + std::to_string(i01_),
                            .scene_node = *node,
                            .renderable_resource_filter = RenderableResourceFilter{}});
                    scene_.add_root_node("check_point_beacon_" + std::to_string(i01_), std::move(node));
                } else if (beacon_nodes_[i01_].check_point_pose != nullptr) {
                    beacon_nodes_[i01_].check_point_pose->beacon_node = nullptr;
                }
                beacon_nodes_[i01_].beacon_node->color_style("").emissivity = selection_emissivity_;
                checkpoints_ahead_.back().beacon_node = &beacon_nodes_[i01_];
                beacon_nodes_[i01_].check_point_pose = &checkpoints_ahead_.back();
                beacon_nodes_[i01_].beacon_node->set_relative_pose(track_element.position, track_element.rotation, 1);
                i01_ = (i01_ + 1) % nbeacons_;
            }
        }
    }

    if (enable_height_changed_mode_) {
        for (auto& b : beacon_nodes_) {
            auto pos = b.beacon_node->position();
            pos(1) = moving_node_->position()(1);
            b.beacon_node->set_position(pos);
        }
        if (checkpoints_ahead_.empty()) {
            checkpoints_ahead_.front().position(1) = moving_node_->position()(1);
        }
    }

    if (!checkpoints_ahead_.empty()) {
        size_t nperiods = checkpoints_ahead_.front().nperiods;
        if (sum(squared(am.t() - checkpoints_ahead_.front().position)) < squared(radius_)) {
            if (checkpoints_ahead_.front().beacon_node != nullptr) {
                checkpoints_ahead_.front().beacon_node->beacon_node->color_style("").emissivity = deselection_emissivity_;
                checkpoints_ahead_.front().beacon_node->check_point_pose = nullptr;
            }
            checkpoints_ahead_.pop_front();
        }
        if ((!checkpoints_ahead_.empty() && (nperiods == lap_times_seconds_.size() + 1)) ||
            (checkpoints_ahead_.empty() && (nperiods == lap_times_seconds_.size())))
        {
            linfo() << "Elapsed time: " << format_minutes_seconds(total_elapsed_seconds_);
            lap_times_seconds_.push_back(lap_elapsed_seconds_);
            race_state_ = player_.notify_lap_finished(total_elapsed_seconds_, lap_times_seconds_, movable_track_);
            if (race_state_ == RaceState::ONGOING) {
                lap_elapsed_seconds_ = 0.f;
            } else if (race_state_ == RaceState::FINISHED) {
                on_finish_();
            } else {
                THROW_OR_ABORT("Unknown race state");
            }
        } else if ((nperiods != lap_times_seconds_.size()) &&
                   (nperiods != lap_times_seconds_.size() - 1))
        {
            THROW_OR_ABORT("Unexpected nperiods");
        }
    }
}

void CheckPoints::notify_destroyed(const Object& destroyed_object) {
    moving_node_ = nullptr;
    moving_ = nullptr;
    advance_times_.schedule_delete_advance_time(*this);

    // Scene destruction happens before physics destruction,
    // so the nodes are deleted here and not in the destructor.
    static const DECLARE_REGEX(re, "^check_point_beacon_.*");
    std::scoped_lock lock{ delete_node_mutex_ };
    scene_.delete_root_nodes(re);
}
