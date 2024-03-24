#include "Check_Points.hpp"
#include <Mlib/Geometry/Coordinates/Homogeneous.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Math/Transformation/Tait_Bryan_Angles.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
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
#include <Mlib/Scene_Graph/Interfaces/IScene_Node_Resource.hpp>
#include <Mlib/Scene_Graph/Interfaces/Scene_Node/IAbsolute_Movable.hpp>
#include <Mlib/Scene_Graph/Resources/Renderable_Resource_Filter.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <Mlib/Time/Format.hpp>

using namespace Mlib;

CheckPoints::CheckPoints(
    std::unique_ptr<ITrackElementSequence>&& sequence,
    size_t nframes,
    size_t nlaps,
    const TransformationMatrix<double, double, 3>* inverse_geographic_mapping,
    AdvanceTimes& advance_times,
    std::string asset_id,
    std::vector<DanglingPtr<SceneNode>> moving_nodes,
    const std::string& resource_name,
    IPlayer& player,
    size_t nbeacons,
    float distance,
    size_t nahead,
    float radius,
    RenderingResources* rendering_resources,
    SceneNodeResources& scene_node_resources,
    Scene& scene,
    DeleteNodeMutex& delete_node_mutex,
    const Focuses& focuses,
    bool enable_height_changed_mode,
    const FixedArray<float, 3>& selection_emissive,
    const FixedArray<float, 3>& deselection_emissive,
    const std::function<void()>& on_finish)
: advance_times_{advance_times},
  track_reader_{
    std::move(sequence),
    nframes,
    nlaps,
    inverse_geographic_mapping,
    TrackElementInterpolationKey::METERS_TO_START,
    TrackReaderInterpolationMode::NEAREST_NEIGHBOR,
    1},
  nlaps_{nlaps},
  asset_id_{std::move(asset_id)},
  moving_nodes_{std::move(moving_nodes)},
  resource_name_{resource_name},
  player_{player},
  radius_{radius},
  nbeacons_{nbeacons},
  distance_{distance},
  nahead_{nahead},
  i01_{0},
  lap_index_{0},
  progress_{0.},
  rendering_resources_{rendering_resources},
  scene_node_resources_{scene_node_resources},
  scene_{scene},
  delete_node_mutex_{delete_node_mutex},
  focuses_{focuses},
  total_elapsed_seconds_{NAN},
  lap_elapsed_seconds_{NAN},
  race_state_{RaceState::COUNTDOWN},
  enable_height_changed_mode_{enable_height_changed_mode},
  selection_emissive_{selection_emissive},
  deselection_emissive_{deselection_emissive},
  on_finish_{on_finish},
  shutting_down_{false}
{
    if (nbeacons == 0) {
        THROW_OR_ABORT("Need at least one beacon node");
    }
    if (distance_ <= 1e-12) {
        THROW_OR_ABORT("Checkpoint distance too small");
    }
    movings_.reserve(moving_nodes_.size());
    for (auto& n : moving_nodes_) {
        movings_.push_back(&n->get_absolute_movable());
    }
    beacon_nodes_.reserve(nbeacons);
    advance_time(0.f);
    // "moving_node_->clearing_observers.add" must be at the end of the constructor
    // in case the ctor throws an exception, because in this case CheckPoints object is not
    // added to the "advance_times" list.
    for (auto& n : moving_nodes_) {
        n->clearing_observers.add(ref<DestructionObserver<DanglingRef<SceneNode>>>(CURRENT_SOURCE_LOCATION));
    }
}

CheckPoints::~CheckPoints() {
    if (!shutting_down_) {
        verbose_abort("CheckPoints dtor without shutdown");
    }
}

void CheckPoints::advance_time(float dt) {
    if (moving_nodes_.size() != movings_.size()) {
        verbose_abort("Inconsistent movings size");
    }
    if (moving_nodes_.empty()) {
        return;
    }
    bool just_started = false;
    if (race_state_ == RaceState::COUNTDOWN) {
        std::shared_lock lock{focuses_.mutex};
        if (!focuses_.countdown_active()) {
            race_state_ = RaceState::ONGOING;
            just_started = true;
        }
    }
    if (race_state_ != RaceState::ONGOING) {
        return;
    }
    if (just_started) {
        total_elapsed_seconds_ = 0.f;
        lap_elapsed_seconds_ = 0.f;
        player_.notify_race_started();
    }
    total_elapsed_seconds_ += dt / s;
    lap_elapsed_seconds_ += dt / s;

    TrackElement te{.elapsed_seconds = total_elapsed_seconds_};
    te.transformations.reserve(movings_.size());
    for (const auto& m : movings_) {
        auto am = m->get_new_absolute_model_matrix();
        te.transformations.push_back(OffsetAndTaitBryanAngles<float, double, 3>{am.R(), am.t()});;
    }
    movable_track_.push_back(te);
    while ((checkpoints_ahead_.size() < nahead_) && (!track_reader_.finished())) {
        auto old_progress = track_reader_.has_value()
            ? track_reader_.progress()
            : NAN;
        if (!track_reader_.read(progress_)) {
            break;
        }
        progress_ += distance_ / meters;
        if (!std::isnan(old_progress) && (track_reader_.progress() == old_progress)) {
            continue;
        }
        checkpoints_ahead_.push_back(CheckPointPose{
            .track_element = track_reader_.track_element(),
            .lap_index = track_reader_.lap_id()});
        if (i01_ == beacon_nodes_.size()) {
            auto node = make_dunique<SceneNode>();
            node->add_color_style(std::make_unique<ColorStyle>(ColorStyle{.selector = Mlib::compile_regex("")}));
            auto& beacon_info = beacon_nodes_.emplace_back(BeaconNode{
                .beacon_node_name = "check_point_beacon_" + std::to_string(i01_),
                .beacon_node = node.get(DP_LOC)});
            scene_node_resources_.instantiate_renderable(
                resource_name_,
                InstantiationOptions{
                    .rendering_resources = rendering_resources_,
                    .instance_name = "beacon",
                    .scene_node = node.ref(DP_LOC),
                    .renderable_resource_filter = RenderableResourceFilter{}});
            node->clearing_observers.add(ref<DestructionObserver<DanglingRef<SceneNode>>>(CURRENT_SOURCE_LOCATION));
            scene_.add_root_node(beacon_info.beacon_node_name, std::move(node));
        } else if (beacon_nodes_[i01_].check_point_pose != nullptr) {
            beacon_nodes_[i01_].check_point_pose->beacon_node = nullptr;
        }
        beacon_nodes_[i01_].beacon_node->color_style("").emissive = selection_emissive_;
        checkpoints_ahead_.back().beacon_node = &beacon_nodes_[i01_];
        beacon_nodes_[i01_].check_point_pose = &checkpoints_ahead_.back();
        const auto& t = track_reader_.track_element().transformation();
        beacon_nodes_[i01_].beacon_node->set_relative_pose(
            t.position(),
            t.rotation(),
            1,
            INITIAL_POSE);
        i01_ = (i01_ + 1) % nbeacons_;
    }

    if (enable_height_changed_mode_) {
        auto y = (*moving_nodes_.begin())->position()(1);
        for (auto& b : beacon_nodes_) {
            auto pos = b.beacon_node->position();
            pos(1) = y;
            b.beacon_node->set_position(pos, INITIAL_POSE);
        }
        if (checkpoints_ahead_.empty()) {
            checkpoints_ahead_.front().track_element.set_y_position(y);
        }
    }

    if (!checkpoints_ahead_.empty()) {
        if (sum(squared((*moving_nodes_.begin())->position() - checkpoints_ahead_.front().track_element.transformation().position())) < squared(radius_)) {
            lap_index_ = checkpoints_ahead_.front().lap_index;
            if (checkpoints_ahead_.front().beacon_node != nullptr) {
                checkpoints_ahead_.front().beacon_node->beacon_node->color_style("").emissive = deselection_emissive_;
                checkpoints_ahead_.front().beacon_node->check_point_pose = nullptr;
            }
            checkpoints_ahead_.pop_front();
        }
        if ((!checkpoints_ahead_.empty() && (lap_index_ == lap_times_seconds_.size() + 1)) ||
            checkpoints_ahead_.empty())
        {
            linfo() << "Elapsed time: " << format_minutes_seconds(total_elapsed_seconds_);
            lap_times_seconds_.push_back(lap_elapsed_seconds_);
            std::vector<FixedArray<float, 3>> vehicle_colors;
            {
                const std::string chassis = "chassis";
                for (const auto& n : moving_nodes_) {
                    if (!n->has_color_style(chassis)) {
                        vehicle_colors.push_back(FixedArray<float, 3>(1.f, 1.f, 1.f));
                        continue;
                    }
                    const auto& style = n->color_style(chassis);
                    if (!all(style.ambient == style.diffuse)) {
                        THROW_OR_ABORT("Could not determine unique vehicle color");
                    }
                    vehicle_colors.push_back(style.ambient);
                }
            }
            race_state_ = player_.notify_lap_finished(
                total_elapsed_seconds_,
                asset_id_,
                vehicle_colors,
                lap_times_seconds_,
                movable_track_);
            if (race_state_ == RaceState::ONGOING) {
                lap_elapsed_seconds_ = 0.f;
            } else if (race_state_ == RaceState::FINISHED) {
                on_finish_();
            } else {
                THROW_OR_ABORT("Unknown race state");
            }
        } else if ((lap_index_ != lap_times_seconds_.size()) &&
                   (lap_index_ != lap_times_seconds_.size() - 1))
        {
            THROW_OR_ABORT("Unexpected lap index");
        }
    }
}

void CheckPoints::notify_destroyed(DanglingRef<SceneNode> destroyed_object) {
    if (shutting_down_) {
        verbose_abort("CheckPoints received multiple shutdown requests");
    }

    shutting_down_ = true;

    for (auto& n : moving_nodes_) {
        if (!n->shutting_down()) {
            n->clearing_observers.remove(ref<DestructionObserver<DanglingRef<SceneNode>>>(CURRENT_SOURCE_LOCATION));
        }
    }
    moving_nodes_.clear();
    movings_.clear();

    {
        // Scene destruction happens before physics destruction,
        // so the nodes are deleted here and not in the destructor.
        std::scoped_lock lock{ delete_node_mutex_ };
        for (auto& b : beacon_nodes_) {
            if (b.beacon_node->shutting_down()) {
                b.beacon_node = nullptr;
            }
            else {
                b.beacon_node->clearing_observers.remove(ref<DestructionObserver<DanglingRef<SceneNode>>>(CURRENT_SOURCE_LOCATION));
                b.beacon_node = nullptr;
                scene_.delete_root_node(b.beacon_node_name);
            }
        }
    }
    advance_times_.schedule_delete_advance_time(*this, CURRENT_SOURCE_LOCATION);
}

bool CheckPoints::has_meters_to_start() const {
    return !checkpoints_ahead_.empty();
}

double CheckPoints::meters_to_start() const {
    if (checkpoints_ahead_.empty()) {
        THROW_OR_ABORT("meters_to_start on empty checkpoints list");
    }
    return checkpoints_ahead_.front().track_element.meters_to_start;
}

size_t CheckPoints::lap_index() const {
    return lap_index_;
}
