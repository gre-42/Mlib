#include "Scene_Node.hpp"
#include <Mlib/Geometry/Cameras/Camera.hpp>
#include <Mlib/Geometry/Coordinates/Homogeneous.hpp>
#include <Mlib/Geometry/Intersection/Axis_Aligned_Bounding_Box.hpp>
#include <Mlib/Geometry/Intersection/Bounding_Sphere.hpp>
#include <Mlib/Geometry/Material/Blending_Pass_Type.hpp>
#include <Mlib/Geometry/Material/Render_Pass.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Iterator/Un_Guarded_Iterator.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Math/Transformation/Bijection.hpp>
#include <Mlib/Math/Transformation/Quaternion.hpp>
#include <Mlib/Math/Transformation/Tait_Bryan_Angles.hpp>
#include <Mlib/Memory/Recursive_Deletion.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Scene_Graph/Animation/Animation_State_Updater.hpp>
#include <Mlib/Scene_Graph/Containers/List_Of_Blended.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Delete_Node_Mutex.hpp>
#include <Mlib/Scene_Graph/Elements/Animation_State.hpp>
#include <Mlib/Scene_Graph/Elements/Blended.hpp>
#include <Mlib/Scene_Graph/Elements/Color_Style.hpp>
#include <Mlib/Scene_Graph/Elements/Dynamic_Style.hpp>
#include <Mlib/Scene_Graph/Elements/Light.hpp>
#include <Mlib/Scene_Graph/Elements/Renderable.hpp>
#include <Mlib/Scene_Graph/Elements/Renderable_With_Style.hpp>
#include <Mlib/Scene_Graph/Elements/Rendering_Strategies.hpp>
#include <Mlib/Scene_Graph/Elements/Skidmark.hpp>
#include <Mlib/Scene_Graph/Interfaces/IDynamic_Lights.hpp>
#include <Mlib/Scene_Graph/Interfaces/IParticle_Renderer.hpp>
#include <Mlib/Scene_Graph/Interfaces/IRenderable_Scene.hpp>
#include <Mlib/Scene_Graph/Interfaces/Scene_Node/IAbsolute_Movable.hpp>
#include <Mlib/Scene_Graph/Interfaces/Scene_Node/IAbsolute_Observer.hpp>
#include <Mlib/Scene_Graph/Interfaces/Scene_Node/INode_Hider.hpp>
#include <Mlib/Scene_Graph/Interfaces/Scene_Node/INode_Modifier.hpp>
#include <Mlib/Scene_Graph/Interfaces/Scene_Node/IRelative_Movable.hpp>
#include <Mlib/Scene_Graph/Render_Pass.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>
#include <Mlib/Scene_Graph/Scene_Graph_Config.hpp>
#include <Mlib/Threads/Unlock_Guard.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <mutex>

using namespace Mlib;

static const auto NO_ANIMATION = VariableAndHash<std::string>{ "<no_animation>" };

SceneNode::SceneNode(
    const FixedArray<ScenePos, 3>& position,
    const FixedArray<float, 3>& rotation,
    float scale,
    PoseInterpolationMode interpolation_mode,
    SceneNodeDomain domain,
    uint32_t user_id)
    : clearing_observers{ *this }
    , destruction_observers{ *this }
    , user_id_{ user_id }
    , scene_{ nullptr }
    , parent_{ nullptr }
    , absolute_movable_{ nullptr }
    , relative_movable_{ nullptr }
    , absolute_observer_{ nullptr }
    , sticky_absolute_observer_{ nullptr }
    , camera_{ nullptr }
    , renderables_{ "Renderables" }
    , children_{ "Children" }
    , aggregate_children_{ "Aggregate children" }
    , instances_children_{ "Instances children" }
    , collide_only_instances_children_{ "Collide-only instances children" }
    , trafo_{ OffsetAndQuaternion<float, ScenePos>::from_tait_bryan_angles({ rotation, position }) }
    , trafo_history_{ trafo_, std::chrono::steady_clock::now() }
    , trafo_history_invalidated_{ false }
    , scale_{ scale }
    , rotation_matrix_{ tait_bryan_angles_2_matrix(rotation) }
    , interpolation_mode_{ interpolation_mode }
    , domain_{ domain }
    , state_{ SceneNodeState::DETACHED }
    , shutting_down_{ false }
    , shutdown_called_{ false }
{
    if (interpolation_mode == PoseInterpolationMode::UNDEFINED) {
        THROW_OR_ABORT("Scene node pose interpolation mode is undefined");
    }
}

SceneNode::SceneNode(
    PoseInterpolationMode interpolation_mode,
    SceneNodeDomain domain,
    uint32_t user_id)
    : SceneNode{
        fixed_zeros<ScenePos, 3>(),
        fixed_zeros<float, 3>(),
        1.f,
        interpolation_mode,
        domain,
        user_id}
{}

void SceneNode::shutdown() {
    if (state_ == SceneNodeState::STATIC) {
        if (scene_ == nullptr) {
            verbose_abort("ERROR: Scene is null in static node");
        }
        if (parent_ == nullptr) {
            if (!scene_->shutting_down()) {
                verbose_abort("ERROR: Static root node is being deleted but scene not shutting down");
            }
        } else if (!parent_->shutting_down()) {
            verbose_abort("ERROR: Static child node is being deleted but parent not shutting down");
        }
    }
    if (shutting_down_) {
        verbose_abort("Recursive call to SceneNode::shutdown()");
    }
    shutting_down_ = true;
    destruction_observers.clear();
    destruction_pointers.clear();
    on_destroy.clear();
    if (sticky_absolute_observer_ != nullptr) {
        verbose_abort("Sticky absolute observer not null");
    }
    clear_unsafe();
    shutting_down_ = false;
    shutdown_called_ = true;
}

SceneNode::~SceneNode() {
    if (!shutdown_called_) {
        verbose_abort("SceneNode::shutdown not called before dtor");
    }
}

bool SceneNode::shutting_down() const {
    // The destruction order is specified explicitly
    // in the "shutdown()" method.
    return shutting_down_;
}

void SceneNode::set_parent(DanglingRef<SceneNode> parent) {
    std::scoped_lock lock{ mutex_ };
    if (has_parent()) {
        THROW_OR_ABORT("Node already has a parent");
    }
    parent_ = parent.ptr().set_loc(DP_LOC);
}

bool SceneNode::has_parent() const {
    std::shared_lock lock{ mutex_ };
    return (parent_ != nullptr);
}

DanglingRef<SceneNode> SceneNode::parent() {
    std::shared_lock lock{ mutex_ };
    if (!has_parent()) {
        THROW_OR_ABORT("Node has no parent");
    }
    return *parent_;
}

DanglingRef<const SceneNode> SceneNode::parent() const {
    return const_cast<SceneNode*>(this)->parent();
}

void SceneNode::setup_child_unsafe(
    const VariableAndHash<std::string>& name,
    DanglingRef<SceneNode> node,
    ChildRegistrationState child_registration_state,
    ChildParentState child_parent_state)
{
    // Required in SceneNonde::~SceneNode
    if ((child_registration_state == ChildRegistrationState::REGISTERED) && (scene_ == nullptr)) {
        THROW_OR_ABORT("Parent of registered node " + *name + " does not have a scene");
    }
    if (name->empty()) {
        THROW_OR_ABORT("Child node has no name");
    }
    if (child_parent_state == ChildParentState::PARENT_NOT_SET) {
        if (node->parent_ != nullptr) {
            THROW_OR_ABORT("Scene node \"" + *name + "\" already has a parent");
        }
        node->parent_ = DanglingPtr<SceneNode>::from_object(*this, DP_LOC);
    } else if (node->parent_ != DanglingPtr<SceneNode>::from_object(*this, DP_LOC)) {
        THROW_OR_ABORT("Child parent mismatch");
    }
    if ((scene_ != nullptr) != (state_ != SceneNodeState::DETACHED)) {
        THROW_OR_ABORT("Conflicting scene nullness and node state");
    }
    if (scene_ != nullptr) {
        node->set_scene_and_state_unsafe(*scene_, state_);
    }
}

INodeModifier& SceneNode::get_node_modifier() const {
    std::shared_lock lock{ mutex_ };
    if (node_modifier_ == nullptr) {
        THROW_OR_ABORT("Node modifier not set");
    }
    return *node_modifier_;
}

void SceneNode::set_node_modifier(std::unique_ptr<INodeModifier>&& node_modifier)
{
    std::scoped_lock lock{ mutex_ };
    if (node_modifier_ != nullptr) {
        THROW_OR_ABORT("Node modifier already set");
    }
    node_modifier_ = std::move(node_modifier);
}

bool SceneNode::contains_node_hider(
    const DanglingBaseClassPtr<IRenderableScene>& renderable_scene,
    const DanglingBaseClassRef<INodeHider>& node_hider) const
{
    std::shared_lock lock{ mutex_ };
    auto avail = node_hiders_.find(renderable_scene);
    if (avail == node_hiders_.end()) {
        return false;
    }
    return avail->second.contains(node_hider.ptr());
}

void SceneNode::insert_node_hider(
    const DanglingBaseClassPtr<IRenderableScene>& renderable_scene,
    const DanglingBaseClassRef<INodeHider>& node_hider)
{
    std::scoped_lock lock{ mutex_ };
    auto it = node_hiders_[renderable_scene].insert(node_hider.ptr());
    if (!it.second) {
        lerr() << "old: " << it.first->loc().file_name() << ':' << it.first->loc().line();
        lerr() << "new: " << node_hider.loc().file_name() << ':' << node_hider.loc().line();
        THROW_OR_ABORT("Node hider already inserted");
    }
}

void SceneNode::remove_node_hider(
    const DanglingBaseClassPtr<IRenderableScene>& renderable_scene,
    const DanglingBaseClassRef<INodeHider>& node_hider)
{
    std::scoped_lock lock{ mutex_ };
    auto avail = node_hiders_.find(renderable_scene);
    if (avail == node_hiders_.end()) {
        verbose_abort("Could not find node hider to be deleted (0)");
    }
    if (avail->second.erase(node_hider.ptr()) != 1) {
        verbose_abort("Could not remove node hider");
    }
    if (avail->second.empty()) {
        node_hiders_.erase(avail);
    }
}

void SceneNode::set_absolute_movable(
    DanglingBaseClassRef<IAbsoluteMovable> absolute_movable)
{
    std::scoped_lock lock{ mutex_ };
    if (absolute_movable_ != nullptr) {
        verbose_abort("Absolute movable already set");
    }
    absolute_movable_ = absolute_movable.ptr();
}

IAbsoluteMovable& SceneNode::get_absolute_movable() const {
    std::shared_lock lock{ mutex_ };
    if (absolute_movable_ == nullptr) {
        THROW_OR_ABORT("Absolute movable not set");
    }
    return *absolute_movable_.get();
}

bool SceneNode::has_absolute_movable() const {
    std::shared_lock lock{ mutex_ };
    return (absolute_movable_ != nullptr);
}

void SceneNode::clear_absolute_movable() {
    std::scoped_lock lock{ mutex_ };
    absolute_movable_ = nullptr;
}

IRelativeMovable& SceneNode::get_relative_movable() const {
    std::shared_lock lock{ mutex_ };
    if (relative_movable_ == nullptr) {
        THROW_OR_ABORT("Relative movable not set");
    }
    return *relative_movable_.get();
}

bool SceneNode::has_relative_movable() const {
    std::shared_lock lock{ mutex_ };
    return (relative_movable_ != nullptr);
}

void SceneNode::clear_relative_movable() {
    std::scoped_lock lock{ mutex_ };
    relative_movable_ = nullptr;
}

void SceneNode::set_relative_movable(const observer_ptr<IRelativeMovable, SceneNode&>& relative_movable)
{
    auto mr = relative_model_matrix();
    auto ma = absolute_model_matrix();
    std::scoped_lock lock{ mutex_ };
    if (relative_movable_ != nullptr) {
        THROW_OR_ABORT("Relative movable already set");
    }
    relative_movable_ = relative_movable.get();
    relative_movable_->set_initial_relative_model_matrix(mr);
    relative_movable_->set_absolute_model_matrix(ma);
    if (relative_movable.observer() != nullptr) {
        clearing_observers.add(*relative_movable.observer());
    }
}

bool SceneNode::has_absolute_observer() const {
    std::shared_lock lock{ mutex_ };
    return absolute_observer_ != nullptr;
}

IAbsoluteObserver& SceneNode::get_absolute_observer() const {
    std::shared_lock lock{ mutex_ };
    if (absolute_observer_ == nullptr) {
        THROW_OR_ABORT("Absolute observer not set");
    }
    return *absolute_observer_;
}

bool SceneNode::has_sticky_absolute_observer() const {
    std::shared_lock lock{ mutex_ };
    return sticky_absolute_observer_ != nullptr;
}

IAbsoluteObserver& SceneNode::get_sticky_absolute_observer() const {
    std::shared_lock lock{ mutex_ };
    if (sticky_absolute_observer_ == nullptr) {
        THROW_OR_ABORT("Sticky absolute observer not set");
    }
    return *sticky_absolute_observer_;
}

void SceneNode::set_absolute_observer(IAbsoluteObserver& absolute_observer)
{
    auto m = absolute_model_matrix();
    std::scoped_lock lock{ mutex_ };
    if (absolute_observer_ != nullptr) {
        THROW_OR_ABORT("Absolute observer already set");
    }
    absolute_observer_ = &absolute_observer;
    absolute_observer_->set_absolute_model_matrix(m);
}

void SceneNode::set_sticky_absolute_observer(IAbsoluteObserver& sticky_absolute_observer)
{
    auto m = absolute_model_matrix();
    std::scoped_lock lock{ mutex_ };
    if (sticky_absolute_observer_ != nullptr) {
        THROW_OR_ABORT("Sticky absolute observer already set");
    }
    sticky_absolute_observer_ = &sticky_absolute_observer;
    sticky_absolute_observer_->set_absolute_model_matrix(m);
}

void SceneNode::add_renderable(
    const VariableAndHash<std::string>& name,
    const std::shared_ptr<const Renderable>& renderable)
{
    std::scoped_lock lock{ mutex_ };
    if (name->empty()) {
        THROW_OR_ABORT("Renderable has no name");
    }
    if (!renderables_.try_emplace(name, std::make_shared<RenderableWithStyle>(renderable)).second) {
        THROW_OR_ABORT("Renderable with name " + *name + " already exists");
    }
}

void SceneNode::set_particle_renderer(
    const VariableAndHash<std::string>& name,
    std::shared_ptr<IParticleRenderer> particle_renderer)
{
    std::scoped_lock lock{ mutex_ };
    if (particle_renderer_ != nullptr) {
        THROW_OR_ABORT("Particle renderer already set");
    }
    particle_renderer_ = particle_renderer;
    add_renderable(name, particle_renderer);
}

std::shared_ptr<IParticleRenderer> SceneNode::get_particle_renderer() const {
    std::shared_lock lock{ mutex_ };
    if (particle_renderer_ == nullptr) {
        THROW_OR_ABORT("Particle renderer not set");
    }
    return particle_renderer_;
}

bool SceneNode::has_node_modifier() const {
    if (scene_ != nullptr) {
        scene_->delete_node_mutex().assert_this_thread_is_deleter_thread();
    }
    std::shared_lock lock{ mutex_ };
    return node_modifier_ != nullptr;
}

void SceneNode::clear_renderable_instance(const VariableAndHash<std::string>& name) {
    std::scoped_lock lock{ mutex_ };
    auto it = renderables_.find(name);
    if (it == renderables_.end()) {
        verbose_abort("Could not clear renderable with name \"" + *name + '"');
    }
    assert_true(it->second != nullptr);
    if (it->second->get() == particle_renderer_.get()) {
        particle_renderer_ = nullptr;
    }
    renderables_.erase(it);
}

void SceneNode::clear_absolute_observer() {
    std::scoped_lock lock{ mutex_ };
    if (absolute_observer_ == nullptr) {
        verbose_abort("Absolute observer not set");
    }
    absolute_observer_ = nullptr;
}

void SceneNode::clear_sticky_absolute_observer() {
    std::scoped_lock lock{ mutex_ };
    if (sticky_absolute_observer_ == nullptr) {
        verbose_abort("Sticky absolute observer not set");
    }
    sticky_absolute_observer_ = nullptr;
}

void SceneNode::clear() {
    if (scene_ != nullptr) {
        scene_->delete_node_mutex().assert_this_thread_is_deleter_thread();
    }
    if (shutting_down()) {
        verbose_abort("Node to be cleared is shutting down");
    }
    clear_unsafe();
}

void SceneNode::clear_unsafe() {
    assert_node_can_be_modified_by_this_thread();
    clearing_observers.clear();
    clearing_pointers.clear();
    on_clear.clear();

    std::scoped_lock lock{ mutex_ };

    absolute_movable_ = nullptr;
    relative_movable_ = nullptr;
    node_modifier_ = nullptr;
    node_hiders_.clear();
    absolute_observer_ = nullptr;
    if (camera_ != nullptr) {
        if (scene_ == nullptr) {
            camera_ = nullptr;
        } else {
            scene_->delete_node_mutex().assert_this_thread_is_deleter_thread();
            scene_->add_to_trash_can(std::move(camera_));
        }
    }
    renderables_.clear();
    auto add_child_to_trash = [this](const auto& child){
        if (child.mapped().scene_node->shutting_down()) {
            verbose_abort("Child node \"" + *child.key() + "\" already shutting down");
        }
        child.mapped().scene_node->shutdown();
        if (scene_ != nullptr) {
            if (child.mapped().is_registered) {
                // scene_ is non-null, checked in "add_child".
                scene_->unregister_node(child.key());
            }
            // linfo() << "c add " << child.mapped().scene_node.get(DP_LOC).get() << " " << child.key();
            scene_->add_to_trash_can(std::move(child.mapped().scene_node));
        }
        };
    clear_map_recursively(children_, add_child_to_trash);
    clear_map_recursively(aggregate_children_, add_child_to_trash);
    clear_map_recursively(instances_children_, add_child_to_trash);
    clear_map_recursively(collide_only_instances_children_, add_child_to_trash);
    lights_.clear();
    skidmarks_.clear();
    animation_state_ = nullptr;
    color_styles_.clear();
    animation_state_updater_ = nullptr;
    periodic_animation_ = VariableAndHash<std::string>{};
    aperiodic_animation_ = VariableAndHash<std::string>{};
}

void SceneNode::add_child(
    const VariableAndHash<std::string>& name,
    DanglingUniquePtr<SceneNode>&& node,
    ChildRegistrationState child_registration_state,
    ChildParentState child_parent_state)
{
    auto nref = node.ref(DP_LOC);
    std::scoped_lock lock{ mutex_ };
    if (!children_.try_emplace(
        name,
        (child_registration_state == ChildRegistrationState::REGISTERED),
        std::move(node)).second)
    {
        THROW_OR_ABORT("Child node with name " + *name + " already exists");
    }
    setup_child_unsafe(name, nref, child_registration_state, child_parent_state);
}

DanglingRef<SceneNode> SceneNode::get_child(const VariableAndHash<std::string>& name) {
    std::shared_lock lock{ mutex_ };
    auto it = children_.try_get(name);
    if (it == nullptr) {
        THROW_OR_ABORT("Node does not have a child with name \"" + *name + '"');
    }
    return it->scene_node.ref(DP_LOC);
}

DanglingRef<const SceneNode> SceneNode::get_child(const VariableAndHash<std::string>& name) const {
    return const_cast<SceneNode*>(this)->get_child(name);
}

void SceneNode::remove_child(const VariableAndHash<std::string>& name) {
    std::scoped_lock lock{ mutex_ };
    if (state_ == SceneNodeState::STATIC) {
        verbose_abort("Cannot remove child \"" + *name + "\" from static node");
    }
    auto it = children_.try_extract(name);
    if (it.empty()) {
        verbose_abort("Cannot not remove child with name \"" + *name + "\" because it does not exist");
    }
    if (it.mapped().scene_node->shutting_down()) {
        verbose_abort("Child node \"" + *name + "\" shutting down (2)");
    }
    it.mapped().scene_node->shutdown();
    if (it.mapped().is_registered) {
        if (scene_ == nullptr) {
            verbose_abort("Can not deregister child \"" + *name + "\" because scene is not set");
        }
        scene_->unregister_node(name);
    }
    if (scene_ != nullptr) {
        scene_->add_to_trash_can(std::move(it.mapped().scene_node));
    }
}

bool SceneNode::contains_child(const VariableAndHash<std::string>& name) const {
    std::shared_lock lock{ mutex_ };
    return children_.contains(name);
}

void SceneNode::add_aggregate_child(
    const VariableAndHash<std::string>& name,
    DanglingUniquePtr<SceneNode>&& node,
    ChildRegistrationState child_registration_state,
    ChildParentState child_parent_state)
{
    auto nref = node.ref(DP_LOC);
    std::scoped_lock lock{ mutex_ };
    if (!aggregate_children_.try_emplace(name, SceneNodeChild{
        .is_registered = (child_registration_state == ChildRegistrationState::REGISTERED),
        .scene_node = std::move(node)}).second)
    {
        THROW_OR_ABORT("Aggregate node with name " + *name + " already exists");
    }
    setup_child_unsafe(name, nref, child_registration_state, child_parent_state);
}

void SceneNode::add_instances_child(
    const VariableAndHash<std::string>& name,
    DanglingUniquePtr<SceneNode>&& node,
    ChildRegistrationState child_registration_state,
    ChildParentState child_parent_state)
{
    std::scoped_lock lock{ mutex_ };
    if (collide_only_instances_children_.contains(name) ||
        instances_children_.contains(name))
    {
        THROW_OR_ABORT("Instances node with name " + *name + " already exists");
    }
    auto emplace_instances_child = [&](StringWithHashUnorderedMap<SceneNodeInstances>& ic)
    {
        if (!ic.try_emplace(
            name,
            (child_registration_state == ChildRegistrationState::REGISTERED),
            std::move(node),
            (CompressedScenePos)0.f,                                                                                // max_center_distance
            SceneNodeInstances::SmallInstances(fixed_full<CompressedScenePos, 3>(CompressedScenePos(15.f)), 12),    // small_instances
            std::list<PositionAndYAngleAndBillboardId<CompressedScenePos>>()).second)                               // large_instances 
        {
            verbose_abort("Internal error: Instances node with name " + *name + " already exists");
        }
    };
    auto nref = node.ref(DP_LOC);
    auto pm = node->physics_attributes();
    if (any(pm & PhysicsMaterial::ATTR_COLLIDE) &&
        !any(pm & PhysicsMaterial::ATTR_VISIBLE))
    {
        emplace_instances_child(collide_only_instances_children_);
    } else {
        if (!any(pm & PhysicsMaterial::ATTR_VISIBLE)) {
            THROW_OR_ABORT(
                "Node \"" + *name +
                "\" is neither collidable nor visible. Physics material: \"" +
                physics_material_to_string(pm) + '"');
        }
        emplace_instances_child(instances_children_);
    }
    setup_child_unsafe(name, nref, child_registration_state, child_parent_state);
}

void SceneNode::add_instances_position(
    const VariableAndHash<std::string>& name,
    const FixedArray<CompressedScenePos, 3>& position,
    float yangle,
    BillboardId billboard_id)
{
    std::scoped_lock lock{ mutex_ };
    {
        auto cit = collide_only_instances_children_.try_get(name);
        if (cit != nullptr) {
            if (billboard_id != BILLBOARD_ID_NONE) {
                THROW_OR_ABORT("Billboard ID set in collide-only node \"" + *name + '"');
            }
            cit->large_instances.push_back(
                PositionAndYAngleAndBillboardId{
                    .position = position,
                    .billboard_id = billboard_id,
                    .yangle = yangle});
            return;
        }
    }
    auto cit = instances_children_.try_get(name);
    if (cit == nullptr) {
        THROW_OR_ABORT("Could not find instance node with name \"" + *name + '"');
    }
    ScenePos mcd2 = cit->scene_node->max_center_distance2(billboard_id);
    if (mcd2 == 0.) {
        THROW_OR_ABORT("Could not determine max_center_distance of node with name \"" + *name + '"');
    }
    if (mcd2 == INFINITY) {
        cit->large_instances.push_back(
            PositionAndYAngleAndBillboardId{
                .position = position,
                .billboard_id = billboard_id,
                .yangle = yangle}
        );
    } else {
        if (!std::isfinite(mcd2)) {
            THROW_OR_ABORT("max_center_distance is not finite");
        }
        cit->max_center_distance = std::max(cit->max_center_distance, (CompressedScenePos)std::sqrt(mcd2));
        if (yangle == 0.f) {
            cit->small_instances.insert(
                PositionAndBillboardId{
                    .position = position,
                    .billboard_id = billboard_id});
        } else {
            cit->small_instances.insert(
                PositionAndYAngleAndBillboardId{
                    .position = position,
                    .billboard_id = billboard_id,
                    .yangle = yangle});
        }
    }
}

void SceneNode::optimize_instances_search_time(std::ostream& ostr) const {
    std::shared_lock lock{ mutex_ };
    for (const auto& [name, i] : instances_children_) {
        ostr << *name << std::endl;
        i.small_instances.optimize_search_time(BvhDataRadiusType::ZERO, ostr);
        // i.small_instances.plot_svg<ScenePos>("/tmp/" + name + ".svg", 0, 1);
    }
}

bool SceneNode::has_camera() const {
    std::shared_lock lock{ mutex_ };
    return camera_ != nullptr;
}

void SceneNode::set_camera(std::unique_ptr<Camera>&& camera) {
    std::scoped_lock lock{ mutex_ };
    if (camera_ != nullptr) {
        THROW_OR_ABORT("Camera already set");
    }
    camera_ = std::move(camera);
}

DanglingBaseClassRef<Camera> SceneNode::get_camera(SourceLocation loc) const {
    std::shared_lock lock{ mutex_ };
    if (camera_ == nullptr) {
        THROW_OR_ABORT("Node has no camera");
    }
    return { *camera_, loc };
}

DanglingBaseClassPtr<Camera> SceneNode::try_get_camera(SourceLocation loc) const {
    std::shared_lock lock{ mutex_ };
    return { camera_.get(), loc };
}

void SceneNode::add_light(std::shared_ptr<Light> light) {
    std::scoped_lock lock{ mutex_ };
    lights_.emplace_back(std::move(light));
}

void SceneNode::add_skidmark(std::shared_ptr<Skidmark> skidmark) {
    std::scoped_lock lock{ mutex_ };
    skidmarks_.emplace_back(std::move(skidmark));
}

bool SceneNode::has_color_style(const VariableAndHash<std::string>& name) const {
    if (scene_ != nullptr) {
        scene_->delete_node_mutex().assert_this_thread_is_deleter_thread();
    }
    std::shared_lock lock{ mutex_ };
    bool style_found = false;
    for (const auto& s : color_styles_) {
        if (!s->matches(name)) {
            continue;
        }
        if (style_found) {
            THROW_OR_ABORT("Node has multiple color styles matching \"" + *name + '"');
        }
        style_found = true;
    }
    return style_found;
}

ColorStyle& SceneNode::color_style(const VariableAndHash<std::string>& name) {
    return const_cast<ColorStyle&>(static_cast<const SceneNode*>(this)->color_style(name));
}

const ColorStyle& SceneNode::color_style(const VariableAndHash<std::string>& name) const {
    std::shared_lock lock{ mutex_ };
    const ColorStyle* result = nullptr;
    for (const auto& s : color_styles_) {
        if (!s->matches(name)) {
            continue;
        }
        if (result != nullptr) {
            THROW_OR_ABORT("Node has multiple color styles matching \"" + *name + '"');
        }
        result = s.get();
    }
    if (result == nullptr) {
        THROW_OR_ABORT("Node has no style matching \"" + *name + '"');
    }
    return *result;
}

void SceneNode::add_color_style(std::unique_ptr<ColorStyle>&& color_style) {
    if (scene_ != nullptr) {
        scene_->delete_node_mutex().assert_this_thread_is_deleter_thread();
    }
    std::scoped_lock lock{ mutex_ };
    if ((state_ != SceneNodeState::DETACHED) && !renderables_.empty()) {
        THROW_OR_ABORT(
            "Color style was set after renderables on a non-detached node. "
            "This leads to a race condition.");
    }
    color_style->compute_hash();
    color_styles_.push_back(std::move(color_style));
}

void SceneNode::set_animation_state(
    std::unique_ptr<AnimationState>&& animation_state,
    AnimationStateAlreadyExistsBehavior already_exists_behavior)
{
    if (scene_ != nullptr) {
        scene_->delete_node_mutex().assert_this_thread_is_deleter_thread();
    }
    std::scoped_lock lock{ mutex_ };
    // if (!renderables_.empty()) {
    //     THROW_OR_ABORT("Animation state was set after renderables, this leads to a race condition");
    // }
    if ((already_exists_behavior == AnimationStateAlreadyExistsBehavior::THROW) &&
        (animation_state_ != nullptr))
    {
        THROW_OR_ABORT("Animation state already set");
    }
    animation_state_ = std::move(animation_state);
}

void SceneNode::set_animation_state_updater(std::unique_ptr<AnimationStateUpdater>&& animation_state_updater) {
    if (scene_ != nullptr) {
        scene_->delete_node_mutex().assert_this_thread_is_deleter_thread();
    }
    std::scoped_lock lock{ mutex_ };
    // if (!renderables_.empty()) {
    //     THROW_OR_ABORT("Animation state updater was set after renderables, this leads to a race condition");
    // }
    if (animation_state_updater_ != nullptr) {
        THROW_OR_ABORT("Scene node already has an animation state updater");
    }
    animation_state_updater_ = std::move(animation_state_updater);
}

void SceneNode::move(
    const TransformationMatrix<float, ScenePos, 3>& v,
    float dt,
    std::chrono::steady_clock::time_point time,
    SceneNodeResources* scene_node_resources,
    const AnimationState* animation_state)
{
    if (scene_ != nullptr) {
        scene_->delete_node_mutex().assert_this_thread_is_deleter_thread();
    }
    {
        std::unique_lock lock{ mutex_ };
        if (state_ == SceneNodeState::STATIC) {
            THROW_OR_ABORT("Cannot move static node");
        }
        if (node_modifier_ != nullptr) {
            node_modifier_->modify_node();
        }
        if (animation_state_ != nullptr) {
            if (animation_state_->aperiodic_animation_frame.active()) {
                animation_state_->aperiodic_animation_frame.advance_time(dt);
            } else {
                animation_state_->periodic_skelletal_animation_frame.advance_time(dt);
            }
            if (animation_state_updater_ != nullptr) {
                auto s = animation_state_updater_->update_animation_state(*animation_state_);
                if (s != nullptr) {
                    set_animation_state(std::move(s), AnimationStateAlreadyExistsBehavior::REPLACE);
                }
            }
        }
        const AnimationState* estate = animation_state_ != nullptr
            ? animation_state_.get()
            : animation_state;
        if (!bone_.name->empty()) {
            if (estate == nullptr) {
                THROW_OR_ABORT("Bone name is not empty, but animation state is not set");
            }
            auto apply_scene_node_animation = [&](float time, const VariableAndHash<std::string>& animation_name){
                if (animation_name->empty() || animation_name == NO_ANIMATION) {
                    return;
                }
                if (scene_node_resources == nullptr) {
                    THROW_OR_ABORT("Scene node animation without scene node resources");
                }
                if (std::isnan(time)) {
                    THROW_OR_ABORT("Scene node animation loop time is NAN");
                }
                auto poses = scene_node_resources->get_absolute_poses(
                    animation_name,
                    time);
                const auto* it = poses.try_get(bone_.name);
                if (it == nullptr) {
                    THROW_OR_ABORT("Could not find bone with name \"node\" in animation \"" + *animation_name + '"');
                }
                OffsetAndQuaternion<float, ScenePos> q1{it->t.casted<ScenePos>(), it->q};
                auto res_pose = trafo_.slerp(q1, 1.f - bone_.smoothness);
                res_pose.q = res_pose.q.slerp(Quaternion<float>::identity(), 1.f - bone_.rotation_strength);
                set_relative_pose(
                    res_pose.t,
                    res_pose.q.to_tait_bryan_angles(),
                    scale(),
                    SUCCESSOR_POSE);
            };
            if (estate->aperiodic_animation_frame.active()) {
                apply_scene_node_animation(
                    estate->aperiodic_animation_frame.time(),
                    aperiodic_animation_->empty()
                        ? estate->aperiodic_skelletal_animation_name
                        : aperiodic_animation_);
            } else {
                apply_scene_node_animation(
                    estate->periodic_skelletal_animation_frame.time(),
                    periodic_animation_->empty()
                        ? estate->periodic_skelletal_animation_name
                        : periodic_animation_);
            }
        }
        TransformationMatrix<float, ScenePos, 3> v2 = uninitialized;
        if ((absolute_movable_ != nullptr) && (relative_movable_ != nullptr)) {
            auto ma = absolute_movable_->get_new_absolute_model_matrix();
            auto mr = v * ma;
            relative_movable_->set_updated_relative_model_matrix(mr);
            relative_movable_->set_absolute_model_matrix(ma);
            auto mr2 = relative_movable_->get_new_relative_model_matrix();
            set_relative_pose(mr2.t, matrix_2_tait_bryan_angles(mr2.R), 1.f, SUCCESSOR_POSE);
            v2 = relative_view_matrix() * v;
            absolute_movable_->set_absolute_model_matrix(v2.inverted_scaled());
        } else {
            if (absolute_movable_ != nullptr) {
                auto m = absolute_movable_->get_new_absolute_model_matrix();
                m = v * m;
                set_relative_pose(m.t, matrix_2_tait_bryan_angles(m.R), 1, SUCCESSOR_POSE);
            }
            v2 = relative_view_matrix() * v;
            if (relative_movable_ != nullptr) {
                relative_movable_->set_absolute_model_matrix(v2.inverted_scaled());
                auto m = relative_movable_->get_new_relative_model_matrix();
                set_relative_pose(m.t, matrix_2_tait_bryan_angles(m.R), 1, SUCCESSOR_POSE);
                v2 = relative_view_matrix() * v;
            }
        }
        if (absolute_observer_ != nullptr) {
            absolute_observer_->set_absolute_model_matrix(v2.inverted_scaled());
        }
        if (sticky_absolute_observer_ != nullptr) {
            sticky_absolute_observer_->set_absolute_model_matrix(v2.inverted_scaled());
        }
        for (auto it = children_.begin(); it != children_.end(); ) {
            OptionalUnlockGuard ulock{ lock, state_ == SceneNodeState::STATIC };
            it->second.scene_node->move(v2, dt, time, scene_node_resources, estate);
            if (it->second.scene_node->to_be_deleted(time)) {
                remove_child((it++)->first);
            } else {
                ++it;
            }
        }
    }
    std::scoped_lock lock{ pose_mutex_ };
    if ((interpolation_mode_ == PoseInterpolationMode::DISABLED) ||
        trafo_history_invalidated_)
    {
        trafo_history_.clear();
        trafo_history_invalidated_ = false;
    }
    trafo_history_.append(trafo_, time);
}

bool SceneNode::to_be_deleted(std::chrono::steady_clock::time_point time) const {
    std::shared_lock lock{ mutex_ };
    return
        (animation_state_ != nullptr) &&
        animation_state_->delete_node_when_aperiodic_animation_finished &&
        (animation_state_->aperiodic_animation_frame.ran_to_completion() ||
         std::visit(
            [&time]<class RefTime>(const RefTime& reference_time) -> bool
            {
                if constexpr (std::is_same_v<RefTime, AperiodicReferenceTime>) {
                    return reference_time.ran_to_completion(time);
                }
                return false;
            },
            animation_state_->reference_time));
}

void SceneNode::set_bone(const SceneNodeBone& bone) {
    std::scoped_lock lock{ mutex_ };
    bone_ = bone;
}

void SceneNode::set_periodic_animation(const VariableAndHash<std::string>& name) {
    std::scoped_lock lock{ mutex_ };
    periodic_animation_ = name;
}

void SceneNode::set_aperiodic_animation(const VariableAndHash<std::string>& name) {
    std::scoped_lock lock{ mutex_ };
    aperiodic_animation_ = name;
}

bool SceneNode::visit_all(
    const TransformationMatrix<float, ScenePos, 3>& parent_m,
    const std::function<bool(
        const TransformationMatrix<float, ScenePos, 3>& m,
        const StringWithHashUnorderedMap<std::shared_ptr<RenderableWithStyle>>& renderables)>& func) const
{
    auto m = parent_m * relative_model_matrix();
    std::shared_lock lock{ mutex_ };
    if (!func(m, renderables_)) {
        return false;
    }
    for (const auto& [_, n] : children_) {
        if (!n.scene_node->visit_all(m, func)) {
            return false;
        }
    }
    return true;
}

PhysicsMaterial SceneNode::physics_attributes() const {
    PhysicsMaterial result = PhysicsMaterial::NONE;
    std::shared_lock lock{ mutex_ };
    for (const auto& [_, r] : renderables_) {
        result |= (*r)->physics_attributes();
    }
    for (const auto& [_, n] : children_) {
        result |= n.scene_node->physics_attributes();
    }
    for (const auto& [_, n] : aggregate_children_) {
        result |= n.scene_node->physics_attributes();
    }
    for (const auto& [_, n] : instances_children_) {
        result |= n.scene_node->physics_attributes();
    }
    for (const auto& [_, n] : collide_only_instances_children_) {
        result |= n.scene_node->physics_attributes();
    }
    return result;
}

RenderingStrategies SceneNode::rendering_strategies() const {
    RenderingStrategies result = RenderingStrategies::NONE;
    std::shared_lock lock{ mutex_ };
    for (const auto& [_, r] : renderables_) {
        result |= (*r)->rendering_strategies();
    }
    for (const auto& [_, n] : children_) {
        result |= n.scene_node->rendering_strategies();
    }
    for (const auto& [_, n] : aggregate_children_) {
        result |= n.scene_node->rendering_strategies();
    }
    for (const auto& [_, n] : instances_children_) {
        result |= n.scene_node->rendering_strategies();
    }
    return result;
}

bool SceneNode::requires_render_pass(ExternalRenderPassType render_pass) const {
    std::shared_lock lock{ mutex_ };
    for (const auto& [_, r] : renderables_) {
        if ((*r)->requires_render_pass(render_pass)) {
            return true;
        }
    }
    for (const auto& [_, n] : children_) {
        if (n.scene_node->requires_render_pass(render_pass)) {
            return true;
        }
    }
    return false;
}

void SceneNode::render(
    const FixedArray<ScenePos, 4, 4>& parent_mvp,
    const TransformationMatrix<float, ScenePos, 3>& parent_m,
    const TransformationMatrix<float, ScenePos, 3>& iv,
    const DanglingPtr<const SceneNode>& camera_node,
    const IDynamicLights* dynamic_lights,
    const std::list<std::pair<TransformationMatrix<float, ScenePos, 3>, std::shared_ptr<Light>>>& lights,
    const std::list<std::pair<TransformationMatrix<float, ScenePos, 3>, std::shared_ptr<Skidmark>>>& skidmarks,
    ListsOfBlended& blended,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    const RenderedSceneDescriptor& frame_id,
    const std::shared_ptr<const AnimationState>& animation_state,
    const std::list<const ColorStyle*>& color_styles,
    SceneNodeVisibility visibility) const
{
    assert_true(is_visible_for_user(frame_id.external_render_pass.user_id));
    auto child_m = relative_model_matrix(frame_id.external_render_pass.time);
    std::shared_lock lock{ mutex_ };
    if (state_ == SceneNodeState::DETACHED) {
        THROW_OR_ABORT("Cannot render detached node");
    }
    auto visit_node_hiders = [&](const auto& avail){
        for (const auto& nh : avail) {
            // Note that the INodeHider may depend on this function being called,
            // so there should not be any additional check above this line.
            if (nh->node_shall_be_hidden(camera_node, frame_id.external_render_pass)) {
                visibility = SceneNodeVisibility::INVISIBLE;
            }
        }
    };
    if (auto avail = node_hiders_.find(nullptr); avail != node_hiders_.end()) {
        visit_node_hiders(avail->second);
    }
    if (frame_id.external_render_pass.renderable_scene != nullptr) {
        if (auto avail = node_hiders_.find(frame_id.external_render_pass.renderable_scene); avail != node_hiders_.end()) {
            visit_node_hiders(avail->second);
        }
    }
    // OpenGL matrices are transposed in memory,
    // https://stackoverflow.com/a/17718408/2292832.
    // "Note that post-multiplying with column-major matrices
    // produces the same result as pre-multiplying with
    // row-major matrices."
    FixedArray<ScenePos, 4, 4> mvp = dot2d(parent_mvp, child_m.affine());
    auto m = parent_m * child_m;
    const std::shared_ptr<const AnimationState>& estate = animation_state_ != nullptr
        ? animation_state_
        : animation_state;
    std::list<const ColorStyle*> ecolor_styles = color_styles;
    for (const auto& s : color_styles_) {
        ecolor_styles.push_back(s.get());
    }
    if (visibility == SceneNodeVisibility::VISIBLE) {
        DynamicStyle dynamic_style{dynamic_lights == nullptr
            ? fixed_zeros<float, 3>()
            : dynamic_lights->get_color(m.t) };
        for (const auto& [n, r] : renderables_) {
            OptionalUnlockGuard ulock{ lock, state_ == SceneNodeState::STATIC };
            if ((*r)->requires_render_pass(frame_id.external_render_pass.pass)) {
                (*r)->render(
                    mvp,
                    m,
                    iv,
                    &dynamic_style,
                    lights,
                    skidmarks,
                    scene_graph_config,
                    render_config,
                    { frame_id, InternalRenderPass::INITIAL },
                    estate.get(),
                    r->style(ecolor_styles, n));
            }
            auto bp = (*r)->required_blending_passes(frame_id.external_render_pass.pass);
            if (any(bp & BlendingPassType::EARLY)) {
                blended.early.list.emplace_back(
                    r,
                    n,
                    mvp,
                    m,
                    estate,
                    ecolor_styles);
            }
            if (any(bp & BlendingPassType::LATE)) {
                blended.late.list.emplace_back(
                    r,
                    n,
                    mvp,
                    m,
                    estate,
                    ecolor_styles);
            }
        }
    }
    for (const auto& [_, c] : children_) {
        if (!c.scene_node->is_visible_for_user(frame_id.external_render_pass.user_id)) {
            continue;
        }
        OptionalUnlockGuard ulock{ lock, state_ == SceneNodeState::STATIC };
        c.scene_node->render(
            mvp,
            m,
            iv,
            camera_node,
            dynamic_lights,
            lights,
            skidmarks,
            blended,
            render_config,
            scene_graph_config,
            frame_id,
            estate,
            ecolor_styles,
            visibility);
    }
}

void SceneNode::append_sorted_aggregates_to_queue(
    const FixedArray<ScenePos, 4, 4>& parent_mvp,
    const TransformationMatrix<float, ScenePos, 3>& parent_m,
    const FixedArray<ScenePos, 3>& offset,
    std::list<std::pair<float, std::shared_ptr<ColoredVertexArray<float>>>>& aggregate_queue,
    const SceneGraphConfig& scene_graph_config,
    const ExternalRenderPass& external_render_pass) const
{
    auto child_m = relative_model_matrix(external_render_pass.time);
    std::shared_lock lock{ mutex_ };
    if (state_ != SceneNodeState::STATIC) {
        THROW_OR_ABORT("Cannot append sorted aggregates to queue for a non-static node");
    }
    // OpenGL matrices are transposed in memory,
    // https://stackoverflow.com/a/17718408/2292832.
    // "Note that post-multiplying with column-major matrices
    // produces the same result as pre-multiplying with
    // row-major matrices."
    FixedArray<ScenePos, 4, 4> mvp = dot2d(parent_mvp, child_m.affine());
    auto m = parent_m * child_m;
    for (const auto& [_, r] : un_guarded_iterator(renderables_, lock)) {
        (*r)->append_sorted_aggregates_to_queue(mvp, m, offset, scene_graph_config, external_render_pass, aggregate_queue);
    }
    for (const auto& [_, c] : un_guarded_iterator(children_, lock)) {
        c.scene_node->append_sorted_aggregates_to_queue(mvp, m, offset, aggregate_queue, scene_graph_config, external_render_pass);
    }
    for (const auto& [_, a] : un_guarded_iterator(aggregate_children_, lock)) {
        a.scene_node->append_sorted_aggregates_to_queue(mvp, m, offset, aggregate_queue, scene_graph_config, external_render_pass);
    }
}

void SceneNode::append_large_aggregates_to_queue(
    const TransformationMatrix<float, ScenePos, 3>& parent_m,
    const FixedArray<ScenePos, 3>& offset,
    std::list<std::shared_ptr<ColoredVertexArray<float>>>& aggregate_queue,
    const SceneGraphConfig& scene_graph_config) const
{
    TransformationMatrix<float, ScenePos, 3> m = parent_m * relative_model_matrix();
    std::shared_lock lock{ mutex_ };
    if (state_ != SceneNodeState::STATIC) {
        THROW_OR_ABORT("Cannot append large aggregates to queue for a non-static node");
    }
    for (const auto& [_, r] : un_guarded_iterator(renderables_, lock)) {
        (*r)->append_large_aggregates_to_queue(m, offset, scene_graph_config, aggregate_queue);
    }
    for (const auto& [_, c] : un_guarded_iterator(children_, lock)) {
        c.scene_node->append_large_aggregates_to_queue(m, offset, aggregate_queue, scene_graph_config);
    }
    for (const auto& [_, a] : un_guarded_iterator(aggregate_children_, lock)) {
        a.scene_node->append_large_aggregates_to_queue(m, offset, aggregate_queue, scene_graph_config);
    }
}

void SceneNode::append_small_instances_to_queue(
    const FixedArray<ScenePos, 4, 4>& parent_mvp,
    const TransformationMatrix<float, ScenePos, 3>& parent_m,
    const TransformationMatrix<float, ScenePos, 3>& iv,
    const FixedArray<ScenePos, 3>& offset,
    const PositionAndYAngleAndBillboardId<CompressedScenePos>& delta_pose,
    SmallInstancesQueues& instances_queues,
    const SceneGraphConfig& scene_graph_config) const
{
    TransformationMatrix<float, ScenePos, 3> rel = relative_model_matrix();
    std::shared_lock lock{ mutex_ };
    if (state_ != SceneNodeState::STATIC) {
        THROW_OR_ABORT("Cannot append small instances to queue for a non-static node");
    }
    rel.t += delta_pose.position.casted<ScenePos>();
    if (delta_pose.yangle != 0) {
        rel.R = dot2d(rel.R, rodrigues2(FixedArray<float, 3>{0.f, 1.f, 0.f}, delta_pose.yangle));
    }
    FixedArray<ScenePos, 4, 4> mvp = dot2d(parent_mvp, rel.affine());
    TransformationMatrix<float, ScenePos, 3> m = parent_m * rel;
    for (const auto& [_, r] : un_guarded_iterator(renderables_, lock)) {
        (*r)->append_sorted_instances_to_queue(mvp, m, iv, offset, delta_pose.billboard_id, scene_graph_config, instances_queues);
    }
    for (const auto& [_, c] : un_guarded_iterator(children_, lock)) {
        c.scene_node->append_small_instances_to_queue(mvp, m, iv, offset, PositionAndYAngleAndBillboardId{ fixed_zeros<CompressedScenePos, 3>(), BILLBOARD_ID_NONE, 0.f }, instances_queues, scene_graph_config);
    }
    for (const auto& [_, i] : un_guarded_iterator(instances_children_, lock)) {
        std::shared_lock ilock{ i.mutex };
        // The transformation is swapped, meaning
        // y = P * V * M * INSTANCE * NODE * x.
        for (const auto& j : un_guarded_iterator(i.large_instances, ilock)) {
            i.scene_node->append_small_instances_to_queue(mvp, m, iv, offset, j, instances_queues, scene_graph_config);
        }
        if (!i.small_instances.empty()) {
            auto camera_position = m.inverted_scaled().transform(iv.t);
            i.small_instances.visit(
                AxisAlignedBoundingBox<CompressedScenePos, 3>::from_center_and_radius(camera_position.casted<CompressedScenePos>(), i.max_center_distance),
                [&, &i = i](const PositionAndYAngleAndBillboardId<CompressedScenePos>& j) {
                    UnlockGuard ulock{ ilock };
                    i.scene_node->append_small_instances_to_queue(mvp, m, iv, offset, j, instances_queues, scene_graph_config);
                    return true;
                },
                [&, &i = i](const PositionAndBillboardId<CompressedScenePos>& j) {
                    UnlockGuard ulock{ ilock };
                    i.scene_node->append_small_instances_to_queue(mvp, m, iv, offset, {j.position, j.billboard_id, 0.f}, instances_queues, scene_graph_config);
                    return true;
                });
        }
    }
}

void SceneNode::append_large_instances_to_queue(
    const FixedArray<ScenePos, 4, 4>& parent_mvp,
    const TransformationMatrix<float, ScenePos, 3>& parent_m,
    const FixedArray<ScenePos, 3>& offset,
    const PositionAndYAngleAndBillboardId<CompressedScenePos>& delta_pose,
    LargeInstancesQueue& instances_queue,
    const SceneGraphConfig& scene_graph_config) const
{
    TransformationMatrix<float, ScenePos, 3> rel = relative_model_matrix();
    std::shared_lock lock{ mutex_ };
    if (state_ != SceneNodeState::STATIC) {
        THROW_OR_ABORT("Cannot append large instances to queue for a non-static node");
    }
    rel.t += delta_pose.position.casted<ScenePos>();
    if (delta_pose.yangle != 0) {
        rel.R = dot2d(rel.R, rodrigues2(FixedArray<float, 3>{0.f, 1.f, 0.f}, delta_pose.yangle));
    }
    FixedArray<ScenePos, 4, 4> mvp = dot2d(parent_mvp, rel.affine());
    TransformationMatrix<float, ScenePos, 3> m = parent_m * rel;
    for (const auto& [_, r] : un_guarded_iterator(renderables_, lock)) {
        (*r)->append_large_instances_to_queue(mvp, m, offset, delta_pose.billboard_id, scene_graph_config, instances_queue);
    }
    for (const auto& [_, c] : un_guarded_iterator(children_, lock)) {
        c.scene_node->append_large_instances_to_queue(mvp, m, offset, PositionAndYAngleAndBillboardId{fixed_zeros<CompressedScenePos, 3>(), BILLBOARD_ID_NONE, 0.f}, instances_queue, scene_graph_config);
    }
    for (const auto& [_, i] : un_guarded_iterator(instances_children_, lock)) {
        std::shared_lock ilock{ i.mutex };
        // The transformation is swapped, meaning
        // y = P * V * M * INSTANCE * NODE * x.
        for (const auto& j : un_guarded_iterator(i.large_instances, ilock)) {
            i.scene_node->append_large_instances_to_queue(mvp, m, offset, j, instances_queue, scene_graph_config);
        }
        i.small_instances.visit_all(
            [&, &i=i](const auto& j){
                UnlockGuard ulock{ ilock };
                i.scene_node->append_large_instances_to_queue(mvp, m, offset, j.payload(), instances_queue, scene_graph_config);
                return true;
            },
            [&, &i=i](const auto& j){
                UnlockGuard ulock{ ilock };
                const auto& J = j.payload();
                i.scene_node->append_large_instances_to_queue(mvp, m, offset, {J.position, J.billboard_id, 0.f}, instances_queue, scene_graph_config);
                return true;
            });
    }
}

void SceneNode::append_physics_to_queue(
    const TransformationMatrix<float, ScenePos, 3>& parent_m,
    const PositionAndYAngleAndBillboardId<CompressedScenePos>& delta_pose,
    std::list<std::pair<TransformationMatrix<float, ScenePos, 3>, std::shared_ptr<ColoredVertexArray<float>>>>& float_queue,
    std::list<std::pair<TransformationMatrix<float, ScenePos, 3>, std::shared_ptr<ColoredVertexArray<CompressedScenePos>>>>& double_queue) const
{
    TransformationMatrix<float, ScenePos, 3> rel = relative_model_matrix();
    std::shared_lock lock{ mutex_ };
    if (state_ != SceneNodeState::STATIC) {
        THROW_OR_ABORT("Cannot append physics to queue for a non-static node");
    }
    rel.t += delta_pose.position.casted<ScenePos>();
    if (delta_pose.yangle != 0) {
        rel.R = dot2d(rel.R, rodrigues2(FixedArray<float, 3>{0.f, 1.f, 0.f}, delta_pose.yangle));
    }
    TransformationMatrix<float, ScenePos, 3> m = parent_m * rel;
    for (const auto& [_, r] : un_guarded_iterator(renderables_, lock)) {
        std::list<std::shared_ptr<ColoredVertexArray<float>>> float_q;
        std::list<std::shared_ptr<ColoredVertexArray<CompressedScenePos>>> double_q;
        (*r)->append_physics_to_queue(float_q, double_q);
        for (const auto& cva : float_q) {
            float_queue.emplace_back(m, cva);
        }
        for (const auto& cva : double_q) {
            double_queue.emplace_back(m, cva);
        }
    }
    auto zero = PositionAndYAngleAndBillboardId{fixed_zeros<CompressedScenePos, 3>(), BILLBOARD_ID_NONE, 0.f};
    for (const auto& [_, c] : un_guarded_iterator(children_, lock)) {
        c.scene_node->append_physics_to_queue(m, zero, float_queue, double_queue);
    }
    for (const auto& [_, c] : un_guarded_iterator(aggregate_children_, lock)) {
        c.scene_node->append_physics_to_queue(m, zero, float_queue, double_queue);
    }
    auto append_instances_children = [&](
        const StringWithHashUnorderedMap<SceneNodeInstances>& instances_children)
    {
        for (const auto& [_, i] : un_guarded_iterator(instances_children, lock)) {
            std::shared_lock ilock{ i.mutex };
            // The transformation is swapped, meaning
            // y = P * V * M * INSTANCE * NODE * x.
            for (const auto& j : un_guarded_iterator(i.large_instances, ilock)) {
                i.scene_node->append_physics_to_queue(m, j, float_queue, double_queue);
            }
            if (!i.small_instances.empty()) {
                i.small_instances.visit_all(
                    [&, &i = i](const PositionAndYAngleAndBillboardId<CompressedScenePos>& j) {
                        UnlockGuard ulock{ ilock };
                        i.scene_node->append_physics_to_queue(m, j, float_queue, double_queue);
                        return true;
                    },
                    [&, &i = i](const PositionAndBillboardId<CompressedScenePos>& j) {
                        UnlockGuard ulock{ ilock };
                        i.scene_node->append_physics_to_queue(m, {j.position, j.billboard_id, 0.f}, float_queue, double_queue);
                        return true;
                    });
            }
        }
    };
    append_instances_children(instances_children_);
    append_instances_children(collide_only_instances_children_);
}

void SceneNode::append_lights_to_queue(
    const TransformationMatrix<float, ScenePos, 3>& parent_m,
    std::list<std::pair<TransformationMatrix<float, ScenePos, 3>, std::shared_ptr<Light>>>& lights) const
{
    TransformationMatrix<float, ScenePos, 3> m = parent_m * relative_model_matrix();
    std::shared_lock lock{ mutex_ };
    for (const auto& l : lights_) {
        lights.emplace_back(m, l);
    }
    for (const auto& [_, c] : children_) {
        OptionalUnlockGuard ulock{ lock, state_ == SceneNodeState::STATIC };
        c.scene_node->append_lights_to_queue(m, lights);
    }
}

void SceneNode::append_skidmarks_to_queue(
    const TransformationMatrix<float, ScenePos, 3>& parent_m,
    std::list<std::pair<TransformationMatrix<float, ScenePos, 3>, std::shared_ptr<Skidmark>>>& skidmarks) const
{
    TransformationMatrix<float, ScenePos, 3> m = parent_m * relative_model_matrix();
    std::shared_lock lock{ mutex_ };
    for (const auto& s : skidmarks_) {
        skidmarks.push_back(std::make_pair(m, s));
    }
    for (const auto& [_, c] : children_) {
        OptionalUnlockGuard ulock{ lock, state_ == SceneNodeState::STATIC };
        c.scene_node->append_skidmarks_to_queue(m, skidmarks);
    }
}

const FixedArray<ScenePos, 3>& SceneNode::position() const {
    std::shared_lock lock{ mutex_ };
    return trafo_.t;
}

FixedArray<float, 3> SceneNode::rotation() const {
    std::shared_lock lock{ mutex_ };
    return matrix_2_tait_bryan_angles(rotation_matrix_);
}

float SceneNode::scale() const {
    std::shared_lock lock{ mutex_ };
    return scale_;
}

void SceneNode::set_position(
    const FixedArray<ScenePos, 3>& position,
    std::optional<std::chrono::steady_clock::time_point> time)
{
    {
        std::scoped_lock lock{ mutex_ };
        if (state_ == SceneNodeState::STATIC) {
            THROW_OR_ABORT("Cannot set position for a static node");
        }
    }
    std::scoped_lock lock{ pose_mutex_ };
    trafo_.t = position;
    if (!time.has_value()) {
        // Do nothing
    } else if (*time == std::chrono::steady_clock::time_point()) {
        trafo_history_.clear();
        trafo_history_.append(trafo_, std::chrono::steady_clock::now());
    } else {
        if (interpolation_mode_ == PoseInterpolationMode::DISABLED) {
            THROW_OR_ABORT("Attempt to set interpolated position of a node with interpolation disabled");
        }
        trafo_history_.append(trafo_, *time);
    }
}

void SceneNode::set_rotation(
    const FixedArray<float, 3>& rotation,
    std::optional<std::chrono::steady_clock::time_point> time)
{
    {
        std::scoped_lock lock{ mutex_ };
        if (state_ == SceneNodeState::STATIC) {
            THROW_OR_ABORT("Cannot set rotation for a static node");
        }
    }
    std::scoped_lock lock{ pose_mutex_ };
    trafo_.q = Quaternion<float>::from_tait_bryan_angles(rotation);
    rotation_matrix_ = tait_bryan_angles_2_matrix(rotation);
    if (!time.has_value()) {
        // Do nothing
    } else if (*time == std::chrono::steady_clock::time_point()) {
        trafo_history_.clear();
        trafo_history_.append(trafo_, std::chrono::steady_clock::now());
    } else {
        if (interpolation_mode_ == PoseInterpolationMode::DISABLED) {
            THROW_OR_ABORT("Attempt to set interpolated rotation of a node with interpolation disabled");
        }
        trafo_history_.append(trafo_, *time);
    }
}

void SceneNode::set_scale(float scale)
{
    std::scoped_lock lock{ pose_mutex_ };
    if (state_ == SceneNodeState::STATIC) {
        THROW_OR_ABORT("Cannot set scale for a static node");
    }
    scale_ = scale;
}

void SceneNode::set_relative_pose(
    const FixedArray<ScenePos, 3>& position,
    const FixedArray<float, 3>& rotation,
    float scale,
    std::optional<std::chrono::steady_clock::time_point> time)
{
    std::scoped_lock lock{ pose_mutex_ };
    set_position(position, SUCCESSOR_POSE);
    set_rotation(rotation, SUCCESSOR_POSE);
    set_scale(scale);
    if (!time.has_value()) {
        // Do nothing
    } else if (*time == std::chrono::steady_clock::time_point()) {
        trafo_history_.clear();
        trafo_history_.append(trafo_, std::chrono::steady_clock::now());
    } else {
        if (interpolation_mode_ == PoseInterpolationMode::DISABLED) {
            THROW_OR_ABORT("Attempt to set interpolated position of a node with interpolation disabled");
        }
        trafo_history_.append(trafo_, *time);
    }
}

TransformationMatrix<float, ScenePos, 3> SceneNode::relative_model_matrix(std::chrono::steady_clock::time_point time) const {
    std::shared_lock lock{ pose_mutex_ };
    if ((time == std::chrono::steady_clock::time_point()) ||
        (interpolation_mode_ == PoseInterpolationMode::DISABLED))
    {
        return TransformationMatrix{rotation_matrix_ * scale_, trafo_.t};
    } else {
        auto res = trafo_history_.get(time);
        return TransformationMatrix{res.q.to_rotation_matrix() * scale_, res.t};
    }
}

TransformationMatrix<float, ScenePos, 3> SceneNode::absolute_model_matrix(std::chrono::steady_clock::time_point time) const {
    return absolute_model_matrix(LockingStrategy::ACQUIRE_LOCK, time);
}

TransformationMatrix<float, ScenePos, 3> SceneNode::absolute_model_matrix(
    LockingStrategy locking_strategy,
    std::chrono::steady_clock::time_point time) const
{
    auto result = relative_model_matrix(time);
    std::shared_lock lock{ pose_mutex_, std::defer_lock };
    if (locking_strategy == LockingStrategy::ACQUIRE_LOCK) {
        lock.lock();
    }
    // if (state_ == SceneNodeState::DYNAMIC) {
    //     scene_->delete_node_mutex().notify_reading();
    // }
    if (parent_ != nullptr) {
        if (locking_strategy == LockingStrategy::ACQUIRE_LOCK) {
            lock.unlock();
        }
        return parent_->absolute_model_matrix(time) * result;
    } else {
        return result;
    }
}

TransformationMatrix<float, ScenePos, 3> SceneNode::relative_view_matrix(std::chrono::steady_clock::time_point time) const {
    std::shared_lock lock{ pose_mutex_ };
    if ((time == std::chrono::steady_clock::time_point()) ||
        (interpolation_mode_ == PoseInterpolationMode::DISABLED))
    {
        return TransformationMatrix<float, ScenePos, 3>::inverse(rotation_matrix_ / scale_, trafo_.t);
    } else {
        auto res = trafo_history_.get(time);
        return TransformationMatrix<float, ScenePos, 3>::inverse(res.q.to_rotation_matrix() / scale_, res.t);
    }
}

TransformationMatrix<float, ScenePos, 3> SceneNode::absolute_view_matrix(std::chrono::steady_clock::time_point time) const {
    auto result = relative_view_matrix(time);
    std::shared_lock lock{ pose_mutex_ };
    // if (state_ == SceneNodeState::DYNAMIC) {
    //     scene_->delete_node_mutex().notify_reading();
    // }
    if (parent_ != nullptr) {
        lock.unlock();
        return result * parent_->absolute_view_matrix(time);
    } else {
        return result;
    }
}

Bijection<TransformationMatrix<float, ScenePos, 3>> SceneNode::relative_bijection(
    std::chrono::steady_clock::time_point time) const
{
    std::shared_lock lock{ pose_mutex_ };
    return {
        .model = relative_model_matrix(time),
        .view = relative_view_matrix(time)
    };
}

Bijection<TransformationMatrix<float, ScenePos, 3>> SceneNode::absolute_bijection(
    std::chrono::steady_clock::time_point time) const
{
    auto result = relative_bijection(time);
    std::shared_lock lock{ pose_mutex_ };
    if (parent_ != nullptr) {
        lock.unlock();
        return parent_->absolute_bijection(time) * result;
    } else {
        return result;
    }
}

FixedArray<float, 3> SceneNode::velocity(
    std::chrono::steady_clock::time_point time,
    std::chrono::steady_clock::duration dt) const
{
    std::shared_lock lock{ pose_mutex_ };
    auto p0 = absolute_model_matrix(LockingStrategy::NO_LOCK, time - dt);
    auto p1 = absolute_model_matrix(LockingStrategy::NO_LOCK, time + dt);
    return (p1.t - p0.t).casted<float>() / (2.f * std::chrono::duration<float>{dt}.count() * seconds);
}

void SceneNode::set_absolute_pose(
    const FixedArray<ScenePos, 3>& position,
    const FixedArray<float, 3>& rotation,
    float scale,
    std::optional<std::chrono::steady_clock::time_point> time)
{
    std::scoped_lock lock{ pose_mutex_ };
    if (parent_ == nullptr) {
        set_relative_pose(
            position,
            rotation,
            scale,
            time);
    } else {
        auto p_v = parent_->absolute_view_matrix();
        auto m = TransformationMatrix<float, ScenePos, 3>{
            tait_bryan_angles_2_matrix(rotation) * scale,
            position};
        auto rel_trafo = p_v * m;
        float rel_scale = rel_trafo.get_scale();
        set_relative_pose(
            rel_trafo.t,
            matrix_2_tait_bryan_angles(rel_trafo.R / rel_scale),
            rel_scale,
            time);
    }
}

ExtremalAxisAlignedBoundingBox<ScenePos, 3> SceneNode::relative_aabb() const {
    std::shared_lock lock{ mutex_ };
    ExtremalAxisAlignedBoundingBox<ScenePos, 3> result = ExtremalBoundingVolume::EMPTY;
    if (!renderables_.empty()) {
        result = ExtremalBoundingVolume::EMPTY;
    }
    for (const auto& [_, r] : renderables_) {
        result.extend((*r)->aabb().casted<ScenePos>());
    }
    for (const auto& [_, c] : children_) {
        auto cb = c.scene_node->relative_aabb();
        if (cb.full()) {
            return ExtremalBoundingVolume::FULL;
        } else if (!cb.empty()) {
            auto m = c.scene_node->relative_model_matrix();
            result.extend(cb.data().transformed(m));
        }
    }
    return result;
}

ExtremalBoundingSphere<ScenePos, 3> SceneNode::relative_bounding_sphere() const {
    std::shared_lock lock{ mutex_ };
    ExtremalBoundingSphere<ScenePos, 3> result = ExtremalBoundingVolume::EMPTY;
    for (const auto& [_, r] : renderables_) {
        result.extend((*r)->bounding_sphere().casted<ScenePos>());
    }
    for (const auto& [_, c] : children_) {
        auto cb = c.scene_node->relative_bounding_sphere();
        if (cb.full()) {
            return ExtremalBoundingVolume::FULL;
        } else if (!cb.empty()) {
            auto m = c.scene_node->relative_model_matrix();
            result.extend(cb.data().transformed(m));
        }
    }
    return result;
}

ScenePos SceneNode::max_center_distance2(BillboardId billboard_id) const {
    std::shared_lock lock{ mutex_ };
    if (!instances_children_.empty()) {
        return INFINITY;
    }
    ScenePos result = 0.;
    for (const auto& [_, r] : renderables_) {
        result = std::max(result, (*r)->max_center_distance2(billboard_id));
    }
    for (const auto& [_, c] : children_) {
        auto cb = c.scene_node->max_center_distance2(BILLBOARD_ID_NONE);
        if (cb == INFINITY) {
            return INFINITY;
        }
        if (cb != 0.f) {
            auto m = c.scene_node->relative_model_matrix();
            if (any(m.t != ScenePos(0))) {
                THROW_OR_ABORT("Detected node translation in max_center_distance");
            }
            result = std::max(result, cb);
        }
    }
    return result;
}

void SceneNode::print(std::ostream& ostr, size_t recursion_depth) const {
    std::shared_lock lock{ mutex_ };
    std::string ind0(3 * recursion_depth, '-');
    std::string ind1(3 * recursion_depth + 1, '-');
    std::string ind2(3 * recursion_depth + 2, '-');
    ostr << " " << ind0 << " Node\n";
    ostr << " " << ind1 << " Position " << position() << '\n';
    ostr << " " << ind1 << " Rotation " << rotation() << '\n';
    if (!renderables_.empty()) {
        ostr << " " << ind1 << " Renderables (" << renderables_.size() << ")\n";
        for (const auto& [n, _] : renderables_) {
            ostr << " " << ind2 << " " << *n << '\n';
        }
    }
    if (!children_.empty()) {
        ostr << " " << ind1 << " Children (" << children_.size() << ")\n";
        for (const auto& [n, c] : children_) {
            ostr << " " << ind2 << " " << *n << " #" << c.scene_node.nreferences() << '\n';
            c.scene_node->print(ostr, recursion_depth + 1);
        }
    }
    if (!aggregate_children_.empty()) {
        ostr << " " << ind1 << " Aggregates (" << aggregate_children_.size() << ")\n";
        for (const auto& [n, c] : aggregate_children_) {
            ostr << " " << ind2 << " " << *n << " #" << c.scene_node.nreferences() << '\n';
            c.scene_node->print(ostr, recursion_depth + 1);
        }
    }
    if (!instances_children_.empty()) {
        ostr << " " << ind1 << " Instances (" << instances_children_.size() << ")\n";
        for (const auto& [n, c] : instances_children_) {
            ostr << " " << ind2 << " " << *n << " #" << c.scene_node.nreferences() <<
                " #small=" << c.small_instances.size() <<
                " #large=" << c.large_instances.size() << '\n';
            c.scene_node->print(ostr, recursion_depth + 1);
        }
    }
    if (!collide_only_instances_children_.empty()) {
        ostr << " " << ind1 << " Collide-only instances (" << collide_only_instances_children_.size() << ")\n";
        for (const auto& [n, c] : collide_only_instances_children_) {
            ostr << " " << ind2 << " " << *n << " #" << c.scene_node.nreferences() <<
                " #small=" << c.small_instances.size() <<
                " #large=" << c.large_instances.size() << '\n';
            c.scene_node->print(ostr, recursion_depth + 1);
        }
    }
    ostr << " " << ind0 << " End\n";
}

void SceneNode::set_scene_and_state(Scene& scene, SceneNodeState state) {
    std::scoped_lock lock{ mutex_ };
    set_scene_and_state_unsafe(scene, state);
}

SceneNodeState SceneNode::state() const {
    return state_;
}

void SceneNode::set_scene_and_state_unsafe(Scene& scene, SceneNodeState state) {
    if (scene_ != nullptr) {
        THROW_OR_ABORT("Scene node already has a scene");
    }
    if (state_ != SceneNodeState::DETACHED) {
        THROW_OR_ABORT("Node state already set");
    }
    if (state == SceneNodeState::DETACHED) {
        THROW_OR_ABORT("Cannot set node state to \"detached\"");
    }
    if ((state == SceneNodeState::STATIC) && (interpolation_mode_ == PoseInterpolationMode::ENABLED)) {
        THROW_OR_ABORT("Static node requires disabled pose interpolation");
    }
    for (auto& [_, c] : children_) {
        c.scene_node->set_scene_and_state(scene, state);
    }
    for (auto& [_, c] : aggregate_children_) {
        c.scene_node->set_scene_and_state(scene, state);
    }
    for (auto& [_, c] : instances_children_) {
        c.scene_node->set_scene_and_state(scene, state);
    }
    for (auto& [_, c] : collide_only_instances_children_) {
        c.scene_node->set_scene_and_state(scene, state);
    }
    scene_ = &scene;
    state_ = state;
}

Scene& SceneNode::scene() {
    std::shared_lock lock{ mutex_ };
    if (scene_ == nullptr) {
        THROW_OR_ABORT("Scene not set");
    }
    return *scene_;
}

const Scene& SceneNode::scene() const {
    return const_cast<SceneNode*>(this)->scene();
}

void SceneNode::set_debug_message(std::string message) {
    std::scoped_lock lock{ mutex_ };
    debug_message_ = message;
}

std::string SceneNode::debug_message() const {
    std::shared_lock lock{ mutex_ };
    return debug_message_;
}

PoseInterpolationMode SceneNode::pose_interpolation_mode() const {
    std::shared_lock lock{ mutex_ };
    return interpolation_mode_;
}

SceneNodeDomain SceneNode::domain() const {
    std::shared_lock lock{ mutex_ };
    return domain_;
}

void SceneNode::assert_node_can_be_modified_by_this_thread() const {
    if (scene_ != nullptr) {
        if (any(domain_ & SceneNodeDomain::PHYSICS)) {
            scene_->delete_node_mutex().assert_this_thread_is_deleter_thread();
        } else if (domain_ == SceneNodeDomain::RENDER) {
            scene_->assert_this_thread_is_render_thread();
        } else {
            verbose_abort("Unsupported scene node domain: \"" + std::to_string((int)domain_));
        }
    }
}

void SceneNode::invalidate_transformation_history() {
    std::scoped_lock lock{ mutex_ };
    trafo_history_invalidated_ = true;
}

std::ostream& Mlib::operator << (std::ostream& ostr, DanglingPtr<const SceneNode> node) {
    if (node != nullptr) {
        node->print(ostr);
    } else {
        ostr << "<null node>\n";
    }
    return ostr;
}
