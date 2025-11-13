#include "Check_Points.hpp"
#include <Mlib/Geometry/Coordinates/Homogeneous.hpp>
#include <Mlib/Geometry/Instance/Rendering_Dynamics.hpp>
#include <Mlib/Iterator/Reverse_Iterator.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Math/Transformation/Tait_Bryan_Angles.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Physics/Advance_Times/Countdown_Physics.hpp>
#include <Mlib/Physics/Containers/Race_State.hpp>
#include <Mlib/Physics/Interfaces/IPlayer.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Delete_Node_Mutex.hpp>
#include <Mlib/Scene_Graph/Elements/Color_Style.hpp>
#include <Mlib/Scene_Graph/Elements/Make_Scene_Node.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Instantiation/Child_Instantiation_Options.hpp>
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
    std::string asset_id,
    VariableAndHash<std::string> resource_name,
    const ViewableRemoteObject& remote_viewable,
    const DanglingBaseClassRef<IPlayer>& player,
    size_t nbeacons,
    float distance,
    size_t nahead,
    float radius,
    RenderingResources* rendering_resources,
    SceneNodeResources& scene_node_resources,
    Scene& scene,
    DeleteNodeMutex& delete_node_mutex,
    const CountdownPhysics* countdown_start,
    bool enable_height_changed_mode,
    const FixedArray<float, 3>& selection_emissive,
    const FixedArray<float, 3>& deselection_emissive,
    const RespawnConfig& respawn_config,
    const std::function<void()>& on_finish)
    : track_reader_{
        std::move(sequence),
        nframes,
        nlaps,
        inverse_geographic_mapping,
        TrackElementInterpolationKey::METERS_TO_START,
        TrackReaderInterpolationMode::NEAREST_NEIGHBOR,
        1 } // ntransformations
    , nlaps_{ nlaps }
    , asset_id_{ std::move(asset_id) }
    , resource_name_{ std::move(resource_name) }
    , remote_viewable_{ remote_viewable }
    , player_{ player }
    , radius_{ radius }
    , nbeacons_{ nbeacons }
    , distance_{ distance }
    , nahead_{ nahead }
    , i01_{ 0 }
    , lap_index_{ 0 }
    , progress_{ 0. }
    , straight_progress_{ 0 }
    , rendering_resources_{ rendering_resources }
    , scene_node_resources_{ scene_node_resources }
    , scene_{ scene }
    , delete_node_mutex_{ delete_node_mutex }
    , countdown_start_{ countdown_start }
    , total_elapsed_seconds_{ NAN }
    , lap_elapsed_seconds_{ NAN }
    , race_state_{ RaceState::COUNTDOWN }
    , enable_height_changed_mode_{ enable_height_changed_mode }
    , selection_emissive_{ selection_emissive }
    , deselection_emissive_{ deselection_emissive }
    , respawn_config_{ respawn_config }
    , on_finish_{ on_finish }
{
    if (nbeacons == 0) {
        THROW_OR_ABORT("Need at least one beacon node");
    }
    if (distance_ <= 1e-12) {
        THROW_OR_ABORT("Checkpoint distance too small");
    }
    moving_nodes_ = player_->moving_nodes();
    movings_.reserve(moving_nodes_.size());
    for (auto& n : moving_nodes_) {
        movings_.push_back(&n->get_absolute_movable());
    }
    beacon_nodes_.reserve(nbeacons);
    advance_time(0.f);
    // "moving_node_->clearing_observers.add" must be at the end of the constructor
    // in case the ctor throws an exception, because in this case the CheckPoints
    // object is not added to the "advance_times" list.
    for (auto& n : moving_nodes_) {
        n->clearing_observers.add({ *this, CURRENT_SOURCE_LOCATION });
    }
}

CheckPoints::~CheckPoints() {
    on_destroy.clear();
}

void CheckPoints::advance_time(float dt, const StaticWorld& world) {
    advance_time(dt);
    reset_player();
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
        if ((countdown_start_ == nullptr) || countdown_start_->finished()) {
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
        player_->notify_race_started();
    }
    total_elapsed_seconds_ += dt / seconds;
    lap_elapsed_seconds_ += dt / seconds;

    TrackElement te{.elapsed_seconds = total_elapsed_seconds_};
    te.transformations.reserve(movings_.size());
    for (const auto& m : movings_) {
        auto am = m->get_new_absolute_model_matrix();
        te.transformations.emplace_back(OffsetAndTaitBryanAngles<float, ScenePos, 3>{am.R, am.t});
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
            .history = std::move(track_reader_.history()),
            .progress = track_reader_.progress(),
            .lap_index = track_reader_.lap_id()});
        if (i01_ == beacon_nodes_.size()) {
            auto node = make_unique_scene_node(
                PoseInterpolationMode::ENABLED,
                SceneNodeDomain::RENDER | SceneNodeDomain::PHYSICS,
                remote_viewable_);
            node->add_color_style(std::make_unique<ColorStyle>());
            if (rendering_resources_ != nullptr) {
                scene_node_resources_.instantiate_child_renderable(
                    resource_name_,
                    ChildInstantiationOptions{
                        .rendering_resources = rendering_resources_,
                        .instance_name = VariableAndHash<std::string>{ "beacon" },
                        .scene_node = node.ref(CURRENT_SOURCE_LOCATION),
                        .interpolation_mode = PoseInterpolationMode::DISABLED,
                        .renderable_resource_filter = RenderableResourceFilter{}});
            }
            node->clearing_observers.add({ *this, CURRENT_SOURCE_LOCATION });
            auto& beacon_info = beacon_nodes_.emplace_back(BeaconNode{
                .beacon_node_name = VariableAndHash<std::string>{"checkpoint_beacon_node_" + scene_.get_temporary_instance_suffix()},
                .beacon_node = node.get(CURRENT_SOURCE_LOCATION)});
            scene_.auto_add_root_node(beacon_info.beacon_node_name, std::move(node), RenderingDynamics::MOVING);
        } else if (beacon_nodes_[i01_].check_point_pose != nullptr) {
            beacon_nodes_[i01_].check_point_pose->beacon_node = nullptr;
        }
        {
            auto& style = beacon_nodes_[i01_].beacon_node->color_style(VariableAndHash<std::string>{""});
            style.set_emissive(selection_emissive_);
        }
        checkpoints_ahead_.back().beacon_node = &beacon_nodes_[i01_];
        beacon_nodes_[i01_].check_point_pose = &checkpoints_ahead_.back();
        const auto& t = track_reader_.track_element().transformation();
        beacon_nodes_[i01_].beacon_node->set_relative_pose(
            t.position,
            t.rotation,
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
        if (!checkpoints_ahead_.empty()) {
            checkpoints_ahead_.front().track_element.set_y_position(y);
        }
    }

    if (!checkpoints_ahead_.empty()) {
        auto& history = checkpoints_ahead_.front().history;
        double progress = checkpoints_ahead_.front().progress;
        const auto& new_element = checkpoints_ahead_.front().track_element;
        const auto& new_location = new_element.transformation();
        if (sum(squared((*moving_nodes_.begin())->position() - new_location.position)) < squared(radius_)) {
            history_.splice(history_.end(), history);
            auto new_meters_to_start = new_element.progress(TrackElementInterpolationKey::METERS_TO_START);
            history_.remove_if([&](const TrackElementExtended& p){
                return new_meters_to_start - p.progress(TrackElementInterpolationKey::METERS_TO_START) > respawn_config_.vehicle_length;
            });
            straight_checkpoints_.remove_if([&](const TrafoAndMetersToStart& p){
                return new_meters_to_start - p.meters_to_start > respawn_config_.max_respawn_distance;
            });
            if (last_reached_checkpoint_.has_value()) {
                auto dpos = new_location.position - *last_reached_checkpoint_;
                auto dpos_l2 = sum(squared(dpos));
                if (dpos_l2 > 1e-12) {
                    auto new_direction = dpos / std::sqrt(dpos_l2);
                    if (last_direction_.has_value()) {
                        assert_true(last_reached_checkpoint_.has_value());
                        if ((dot0d(new_direction, *last_direction_) < std::cos(respawn_config_.max_horizontal_angle)) ||
                            std::abs(new_direction(1)) > std::sin(respawn_config_.max_vertical_angle))
                        {
                            straight_progress_ = progress;
                        } else if (progress - straight_progress_ > respawn_config_.vehicle_length) {
                            auto& lsc = straight_checkpoints_.emplace_back(new_location.to_matrix(), new_meters_to_start);
                            for (const auto& e : history_) {
                                lsc.trafo.t(1) = std::max(
                                    lsc.trafo.t(1),
                                    e.element.transformation().position(1));
                            }
                        }
                    }
                    last_direction_ = new_direction;
                }
            }
            last_reached_checkpoint_ = new_location.position;
            lap_index_ = checkpoints_ahead_.front().lap_index;
            if (checkpoints_ahead_.front().beacon_node != nullptr) {
                auto& style = checkpoints_ahead_.front().beacon_node->beacon_node->color_style(VariableAndHash<std::string>{""});
                style.set_emissive(deselection_emissive_);
                checkpoints_ahead_.front().beacon_node->check_point_pose = nullptr;
            }
            checkpoints_ahead_.pop_front();
        }
        if ((!checkpoints_ahead_.empty() && (lap_index_ == lap_times_seconds_.size() + 1)) ||
            checkpoints_ahead_.empty())
        {
            linfo() << "Elapsed time: " << format_minutes_seconds(total_elapsed_seconds_);
            lap_times_seconds_.push_back(lap_elapsed_seconds_);
            UUVector<FixedArray<float, 3>> vehicle_colors;
            {
                const auto chassis = VariableAndHash<std::string>{ "chassis" };
                for (const auto& n : moving_nodes_) {
                    if (!n->has_color_style(chassis)) {
                        vehicle_colors.emplace_back(1.f, 1.f, 1.f);
                        continue;
                    }
                    const auto& style = n->color_style(chassis);
                    if (style.ambient != style.diffuse) {
                        THROW_OR_ABORT("Could not determine unique vehicle color");
                    }
                    vehicle_colors.emplace_back(style.ambient);
                }
            }
            race_state_ = player_->notify_lap_finished(
                total_elapsed_seconds_,
                asset_id_,
                vehicle_colors,
                lap_times_seconds_,
                movable_track_);
            if (race_state_ == RaceState::ONGOING) {
                lap_elapsed_seconds_ = 0.f;
            } else if (race_state_ == RaceState::FINISHED) {
                last_reached_checkpoint_.reset();
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

void CheckPoints::reset_player() {
    if (player_->reset_vehicle_requested()) {
        for (const auto& sc : reverse(straight_checkpoints_)) {
            if (!player_->can_reset_vehicle(sc.trafo)) {
                continue;
            }
            for (auto& n : moving_nodes_) {
                n->clearing_observers.remove({ *this, CURRENT_SOURCE_LOCATION });
            }
            moving_nodes_.clear();
            movings_.clear();

            if (!player_->try_reset_vehicle(sc.trafo)) {
                verbose_abort("Cannot reset player vehicle");
            }

            moving_nodes_ = player_->moving_nodes();
            movings_.reserve(moving_nodes_.size());
            for (auto& n : moving_nodes_) {
                movings_.push_back(&n->get_absolute_movable());
                n->clearing_observers.add({ *this, CURRENT_SOURCE_LOCATION });
            }
            break;
        }
    }
}

void CheckPoints::notify_destroyed(SceneNode& destroyed_object) {
    delete_node_mutex_.assert_this_thread_is_deleter_thread();
    for (auto& n : moving_nodes_) {
        if (n->shutdown_phase() == ShutdownPhase::NONE) {
            n->clearing_observers.remove({ *this, CURRENT_SOURCE_LOCATION });
        }
    }
    moving_nodes_.clear();
    movings_.clear();

    {
        // Scene destruction happens before physics destruction,
        // so the nodes are deleted here and not in the destructor.
        for (auto& b : beacon_nodes_) {
            if (b.beacon_node->shutdown_phase() != ShutdownPhase::NONE) {
                b.beacon_node = nullptr;
            }
            else {
                b.beacon_node->clearing_observers.remove({ *this, CURRENT_SOURCE_LOCATION });
                b.beacon_node = nullptr;
                scene_.delete_root_node(b.beacon_node_name);
            }
        }
    }
    global_object_pool.remove(this);
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
