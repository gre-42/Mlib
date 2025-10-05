#include "Scene.hpp"
#include <Mlib/Array/Chunked_Array.hpp>
#include <Mlib/Geometry/Coordinates/Homogeneous.hpp>
#include <Mlib/Geometry/Instance/Rendering_Dynamics.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Transformed_Colored_Vertex_Array.hpp>
#include <Mlib/Log.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Memory/Recursive_Deletion.hpp>
#include <Mlib/Scene_Graph/Batch_Renderers/IAggregate_Renderer.hpp>
#include <Mlib/Scene_Graph/Batch_Renderers/IInstances_Renderer.hpp>
#include <Mlib/Scene_Graph/Batch_Renderers/Task_Location.hpp>
#include <Mlib/Scene_Graph/Containers/List_Of_Blended.hpp>
#include <Mlib/Scene_Graph/Containers/Root_Nodes.hpp>
#include <Mlib/Scene_Graph/Delete_Node_Mutex.hpp>
#include <Mlib/Scene_Graph/Elements/Blended.hpp>
#include <Mlib/Scene_Graph/Elements/Color_Style.hpp>
#include <Mlib/Scene_Graph/Elements/Light.hpp>
#include <Mlib/Scene_Graph/Elements/Rendering_Strategies.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Instances/Large_Instances_Queue.hpp>
#include <Mlib/Scene_Graph/Instances/Small_Instances_Queues.hpp>
#include <Mlib/Scene_Graph/Interfaces/IDynamic_Lights.hpp>
#include <Mlib/Scene_Graph/Interfaces/ITrail_Renderer.hpp>
#include <Mlib/Scene_Graph/Render_Pass.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>
#include <Mlib/Scene_Graph/Scene_Graph_Config.hpp>
#include <Mlib/Threads/Background_Loop.hpp>
#include <Mlib/Threads/Unlock_Guard.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <Mlib/Time/Fps/Lag_Finder.hpp>
#include <mutex>

using namespace Mlib;

using NodeRawPtrs = ChunkedArray<std::list<std::vector<const SceneNode*>>>;
using NodeDanglingPtrs = ChunkedArray<std::list<std::vector<DanglingBaseClassPtr<const SceneNode>>>>;
static const size_t CHUNK_SIZE = 1000;

Scene::Scene(
    std::string name,
    DeleteNodeMutex& delete_node_mutex,
    SceneNodeResources* scene_node_resources,
    ITrailRenderer* trail_renderer,
    IDynamicLights* dynamic_lights)
    : nodes_{ "Scene node" }
    , delete_node_mutex_{ delete_node_mutex }
    , morn_{ *this }
    , root_nodes_{ morn_.create(VariableAndHash<std::string>{"root_nodes"}) }
    , static_root_nodes_{ morn_.create(VariableAndHash<std::string>{"static_root_nodes"}) }
    , root_aggregate_once_nodes_{ morn_.create(VariableAndHash<std::string>{"root_aggregate_once_nodes"}) }
    , root_aggregate_always_nodes_{ morn_.create(VariableAndHash<std::string>{"root_aggregate_always_nodes"}) }
    , root_instances_once_nodes_{ morn_.create(VariableAndHash<std::string>{"root_instances_once_nodes"}) }
    , root_instances_always_nodes_{ morn_.create(VariableAndHash<std::string>{"root_instances_always_nodes"}) }
    , static_root_physics_nodes_{ morn_.create(VariableAndHash<std::string>{"static_root_physics_nodes"}) }
    , name_{ std::move(name) }
    , uuid_{ 0 }
    , shutting_down_{ false }
    , scene_node_resources_{ scene_node_resources }
    , trail_renderer_{ trail_renderer }
    , dynamic_lights_{ dynamic_lights }
    , ncleanups_required_{ 0 }
{}

void Scene::add_moving_root_node(
    const VariableAndHash<std::string>& name,
    std::unique_ptr<SceneNode>&& scene_node)
{
    std::scoped_lock lock{ mutex_ };
    LOG_FUNCTION("Scene::add_root_node");
    root_nodes_.add_root_node(name, std::move(scene_node), SceneNodeState::DYNAMIC);
}

void Scene::add_static_root_node(
    const VariableAndHash<std::string>& name,
    std::unique_ptr<SceneNode>&& scene_node)
{
    std::scoped_lock lock{ mutex_ };
    static_root_nodes_.add_root_node(name, std::move(scene_node), SceneNodeState::STATIC);
}

void Scene::add_root_aggregate_once_node(
    const VariableAndHash<std::string>& name,
    std::unique_ptr<SceneNode>&& scene_node)
{
    std::scoped_lock lock{ mutex_ };
    root_aggregate_once_nodes_.add_root_node(name, std::move(scene_node), SceneNodeState::STATIC);
}

void Scene::add_root_aggregate_always_node(
    const VariableAndHash<std::string>& name,
    std::unique_ptr<SceneNode>&& scene_node)
{
    std::scoped_lock lock{ mutex_ };
    root_aggregate_always_nodes_.add_root_node(name, std::move(scene_node), SceneNodeState::STATIC);
}

void Scene::add_root_instances_once_node(
    const VariableAndHash<std::string>& name,
    std::unique_ptr<SceneNode>&& scene_node)
{
    std::scoped_lock lock{ mutex_ };
    root_instances_once_nodes_.add_root_node(name, std::move(scene_node), SceneNodeState::STATIC);
}

void Scene::add_root_instances_always_node(
    const VariableAndHash<std::string>& name,
    std::unique_ptr<SceneNode>&& scene_node)
{
    std::scoped_lock lock{ mutex_ };
    root_instances_always_nodes_.add_root_node(name, std::move(scene_node), SceneNodeState::STATIC);
}

void Scene::add_static_root_physics_node(
    const VariableAndHash<std::string>& name,
    std::unique_ptr<SceneNode>&& scene_node)
{
    std::scoped_lock lock{ mutex_ };
    static_root_physics_nodes_.add_root_node(name, std::move(scene_node), SceneNodeState::STATIC);
}

void Scene::auto_add_root_node(
    const VariableAndHash<std::string>& name,
    std::unique_ptr<SceneNode>&& scene_node,
    RenderingDynamics rendering_dynamics)
{
    add_root_node(
        name,
        std::move(scene_node),
        rendering_dynamics,
        scene_node->rendering_strategies());
}

void Scene::add_root_node(
    const VariableAndHash<std::string>& name,
    std::unique_ptr<SceneNode>&& scene_node,
    RenderingDynamics rendering_dynamics,
    RenderingStrategies rendering_strategy)
{
    switch (rendering_strategy) {
    case RenderingStrategies::NONE:
        if (rendering_dynamics != RenderingDynamics::STATIC) {
            THROW_OR_ABORT(
                "Physics root node must be static, "
                "or node accidentally has no renderables");
        }
        add_static_root_physics_node(name, std::move(scene_node));
        return;
    case RenderingStrategies::OBJECT:
        switch (rendering_dynamics) {
        case RenderingDynamics::STATIC:
            add_static_root_node(name, std::move(scene_node));
            return;
        case RenderingDynamics::MOVING:
            add_moving_root_node(name, std::move(scene_node));
            return;
        }
        THROW_OR_ABORT(
            "Unknown rendering dynamics: " + std::to_string((int)rendering_dynamics));
    case RenderingStrategies::MESH_ONCE:
        if (rendering_dynamics != RenderingDynamics::STATIC) {
            THROW_OR_ABORT("Mesh aggregation requires static rendering dynamics");
        }
        add_root_aggregate_once_node(name, std::move(scene_node));
        return;
    case RenderingStrategies::MESH_SORTED_CONTINUOUSLY:
        if (rendering_dynamics != RenderingDynamics::STATIC) {
            THROW_OR_ABORT("Mesh aggregation requires static rendering dynamics");
        }
        add_root_aggregate_always_node(name, std::move(scene_node));
        return;
    case RenderingStrategies::INSTANCES_ONCE:
        if (rendering_dynamics != RenderingDynamics::STATIC) {
            THROW_OR_ABORT("Instances require static rendering dynamics");
        }
        add_root_instances_once_node(name, std::move(scene_node));
        return;
    case RenderingStrategies::INSTANCES_SORTED_CONTINUOUSLY:
        if (rendering_dynamics != RenderingDynamics::STATIC) {
            THROW_OR_ABORT("Instances require static rendering dynamics");
        }
        add_root_instances_always_node(name, std::move(scene_node));
        return;
    }
    THROW_OR_ABORT(
        "Unknown singular rendering strategy: \"" +
        rendering_strategies_to_string(rendering_strategy) + '"');
}

void Scene::add_root_imposter_node(
    const DanglingBaseClassPtr<IRenderableScene>& renderable_scene,
    const DanglingBaseClassRef<SceneNode>& scene_node)
{
    if (scene_node->domain() != SceneNodeDomain::RENDER) {
        THROW_OR_ABORT("Imposter node domain is not \"render\"");
    }
    std::scoped_lock lock{ mutex_ };
    scene_node->set_scene_and_state(*this, SceneNodeState::DYNAMIC);
    if (!root_imposter_nodes_[renderable_scene].insert(scene_node.ptr()).second)
    {
        THROW_OR_ABORT("Root imposter node already exists");
    }
}

void Scene::move_root_node_to_bvh(const VariableAndHash<std::string>& name) {
    if (static_root_nodes_.contains(name)) {
        static_root_nodes_.move_node_to_bvh(name);
    } else if (root_aggregate_once_nodes_.contains(name)) {
        root_aggregate_once_nodes_.move_node_to_bvh(name);
    } else if (root_aggregate_always_nodes_.contains(name)) {
        root_aggregate_always_nodes_.move_node_to_bvh(name);
    } else {
        root_instances_once_nodes_.move_node_to_bvh(name);
    }
}

void Scene::try_delete_root_node(const VariableAndHash<std::string>& name) {
    if (nodes_.contains(name)) {
        delete_root_node(name);
    }
}

void Scene::delete_root_imposter_node(
    const DanglingBaseClassPtr<IRenderableScene>& renderable_scene,
    const DanglingBaseClassRef<SceneNode>& scene_node)
{
    std::scoped_lock lock{ mutex_ };
    scene_node->shutdown();
    auto avail = root_imposter_nodes_.find(renderable_scene);
    if (avail == root_imposter_nodes_.end()) {
        verbose_abort("Could not delete root imposter node (0)");
    }
    if (avail->second.erase(scene_node.ptr()) != 1) {
        verbose_abort("Could not delete root imposter node (1)");
    }
    if (avail->second.empty()) {
        root_imposter_nodes_.erase(avail);
    }
}

void Scene::delete_root_node(const VariableAndHash<std::string>& name) {
    LOG_FUNCTION("Scene::delete_root_node");
    root_nodes_.delete_root_node(name);
}

void Scene::delete_root_nodes(const Mlib::re::cregex& regex) {
    LOG_FUNCTION("Scene::delete_root_nodes");
    root_nodes_.delete_root_nodes(regex);
}

void Scene::try_delete_node(const VariableAndHash<std::string>& name) {
    delete_node_mutex_.assert_this_thread_is_deleter_thread();
    if (nodes_.contains(name)) {
        delete_node(name);
    }
}

void Scene::delete_node(const VariableAndHash<std::string>& name) {
    delete_node_mutex_.assert_this_thread_is_deleter_thread();
    try {
        DanglingBaseClassPtr<SceneNode> node = get_node(name, CURRENT_SOURCE_LOCATION).ptr();
        if (!node->shutting_down()) {
            if (node->has_parent()) {
                DanglingBaseClassRef<SceneNode> parent = node->parent();
                node = nullptr;
                parent->remove_child(name);
            } else {
                node = nullptr;
                delete_root_node(name);
            }
        }
    } catch (const std::exception& e) {
        verbose_abort("Could not delete node with name \"" + *name + "\": " + e.what());
    }
}

void Scene::delete_nodes(const Mlib::re::cregex& regex) {
    delete_node_mutex_.assert_this_thread_is_deleter_thread();
    std::unique_lock lock{ mutex_ };
    for (auto it = nodes_.begin(); it != nodes_.end(); ) {
        auto n = it++;
        if (Mlib::re::regex_match(*n->first, regex)) {
            UnlockGuard ulock{ lock };
            delete_node(n->first);
        }
    }
}

Scene::~Scene() {
    shutdown();
}

void Scene::shutdown() {
    if (shutting_down_) {
        return;
    }
    delete_node_mutex_.clear_deleter_thread();
    delete_node_mutex_.set_deleter_thread();
    shutting_down_ = true;
    morn_.shutdown();
    if (!nodes_.empty()) {
        for (const auto& [name, _] : nodes_) {
            lerr() << *name;
        }
        verbose_abort("Registered nodes remain after shutdown");
    }
    if (!root_imposter_nodes_.empty()) {
        verbose_abort("Imposter nodes remain after shutdown");
    }
    size_t nremaining = try_empty_the_trash_can();
    if (nremaining != 0) {
        for (const auto& o : trash_can_child_nodes_) {
            o->print_references();
        }
        for (const auto& o : trash_can_obj_) {
            o->print_references();
        }
        morn_.print_trash_can_references();
        verbose_abort("Dangling references remain after scene shutdown");
    }
}

bool Scene::contains_node(const VariableAndHash<std::string>& name) const {
    std::shared_lock lock{ mutex_ };
    return nodes_.contains(name);
}

void Scene::add_to_trash_can(std::unique_ptr<SceneNode>&& node) {
    delete_node_mutex_.assert_this_thread_is_deleter_thread();
    trash_can_child_nodes_.emplace_back(std::move(node));
}

void Scene::add_to_trash_can(std::unique_ptr<DanglingBaseClass>&& obj) {
    delete_node_mutex_.assert_this_thread_is_deleter_thread();
    trash_can_obj_.push_back(std::move(obj));
}

size_t Scene::try_empty_the_trash_can() {
    delete_node_mutex_.assert_this_thread_is_deleter_thread();
    for (auto it = trash_can_obj_.begin(); it != trash_can_obj_.end();) {
        auto c = it++;
        if ((*c)->nreferences() == 0) {
            trash_can_obj_.erase(c);
        }
    }
    for (auto it = trash_can_child_nodes_.begin(); it != trash_can_child_nodes_.end();) {
        auto c = it++;
        if ((*c)->nreferences() == 0) {
            trash_can_child_nodes_.erase(c);
        }
    }
    return trash_can_obj_.size() + trash_can_child_nodes_.size() + morn_.try_empty_the_trash_can();
}

void Scene::register_node(
    const VariableAndHash<std::string>& name,
    const DanglingBaseClassRef<SceneNode>& scene_node)
{
    std::scoped_lock lock{ mutex_ };
    if (name->empty()) {
        THROW_OR_ABORT("register_node received empty name");
    }
    if (!nodes_.try_emplace(name, scene_node.ptr()).second) {
        THROW_OR_ABORT("Scene node with name \"" + *name + "\" already exists");
    }
}

void Scene::unregister_node(const VariableAndHash<std::string>& name) {
    std::scoped_lock lock{ mutex_ };
    delete_node_mutex_.assert_this_thread_is_deleter_thread();
    if (nodes_.erase(name) != 1) {
        verbose_abort("Could not find node with name (0) \"" + *name + '"');
    }
}

void Scene::unregister_nodes(const Mlib::re::cregex& regex) {
    std::scoped_lock lock{ mutex_ };
    delete_node_mutex_.assert_this_thread_is_deleter_thread();
    for (auto it = nodes_.begin(); it != nodes_.end(); ) {
        auto n = *it++;
        if (Mlib::re::regex_match(*n.first, regex)) {
            if (nodes_.erase(n.first) != 1) {
                verbose_abort("Could not find node with name (1) \"" + *n.first + '"');
            }
        }
    }
}

DanglingBaseClassRef<SceneNode> Scene::get_node(const VariableAndHash<std::string>& name, SourceLocation loc) const {
    std::shared_lock lock{ mutex_ };
    auto it = nodes_.try_get(name);
    if (it == nullptr) {
        THROW_OR_ABORT("Could not find node with name (2) \"" + *name + '"');
    }
    // Only for debugging purposes, as it
    // overwrites the debug-message with each call.
    // res->set_debug_message(name);
    return **it;
}

DanglingBaseClassPtr<SceneNode> Scene::try_get_node(const VariableAndHash<std::string>& name, SourceLocation loc) const {
    std::shared_lock lock{ mutex_ };
    if (!contains_node(name)) {
        return nullptr;
    }
    return get_node(name, loc).ptr();
}

std::list<std::pair<VariableAndHash<std::string>, DanglingBaseClassRef<SceneNode>>> Scene::get_nodes(const Mlib::re::cregex& regex) const {
    std::shared_lock lock{ mutex_ };
    std::list<std::pair<VariableAndHash<std::string>, DanglingBaseClassRef<SceneNode>>> result;
    for (const auto& [name, node] : nodes_) {
        if (Mlib::re::regex_match(*name, regex)) {
            result.emplace_back(name, *node);
        }
    }
    return result;
}

bool Scene::visit_all(const std::function<bool(
    const TransformationMatrix<float, ScenePos, 3>& m,
    const StringWithHashUnorderedMap<std::shared_ptr<RenderableWithStyle>>& renderables)>& func) const
{
    std::shared_lock lock{ mutex_ };
    return
        root_nodes_.visit_all([&func](const auto& node) {
            return node->visit_all(TransformationMatrix<float, ScenePos, 3>::identity(), func);
            }) &&
        static_root_nodes_.visit_all([&func](const auto& node){
            return node->visit_all(TransformationMatrix<float, ScenePos, 3>::identity(), func);
            }) &&
        root_aggregate_once_nodes_.visit_all([&func](const auto& node){
            return node->visit_all(TransformationMatrix<float, ScenePos, 3>::identity(), func);
            }) &&
        root_aggregate_always_nodes_.visit_all([&func](const auto& node){
            return node->visit_all(TransformationMatrix<float, ScenePos, 3>::identity(), func);
            }) &&
        root_instances_once_nodes_.visit_all([&func](const auto& node){
            return node->visit_all(TransformationMatrix<float, ScenePos, 3>::identity(), func);
            }) &&
        root_instances_always_nodes_.visit_all([&func](const auto& node){
            return node->visit_all(TransformationMatrix<float, ScenePos, 3>::identity(), func);
            }) &&
        static_root_physics_nodes_.visit_all([&func](const auto& node){
            return node->visit_all(TransformationMatrix<float, ScenePos, 3>::identity(), func);
            });
}

void Scene::render(
    const FixedArray<ScenePos, 4, 4>& vp,
    const TransformationMatrix<float, ScenePos, 3>& iv,
    const DanglingBaseClassPtr<const SceneNode>& camera_node,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    const RenderedSceneDescriptor& frame_id,
    const std::function<std::function<void()>(std::function<void()>)>& run_in_background) const
{
    // AperiodicLagFinder lag_finder{ "Render: ", std::chrono::milliseconds{5} };
    LOG_FUNCTION("Scene::render");
    assert_this_thread_is_render_thread();
    std::list<std::pair<TransformationMatrix<float, ScenePos, 3>, std::shared_ptr<Light>>> lights;
    std::list<std::pair<TransformationMatrix<float, ScenePos, 3>, std::shared_ptr<Skidmark>>> skidmarks;
    ListsOfBlended blended;
    std::list<std::shared_ptr<const ColorStyle>> color_styles;
    {
        for (const auto& s : color_styles_.shared()) {
            color_styles.push_back(s);
        }
    }
    std::shared_ptr<AnimationState> animation_state;
    {
        std::shared_lock lock{ mutex_ };
        animation_state = animation_state_;
    }
    if (frame_id.external_render_pass.pass == ExternalRenderPassType::LIGHTMAP_BLACK_NODE) {
        DanglingBaseClassRef<SceneNode> node = [this, &frame_id](){
            std::shared_lock lock{ mutex_ };
            auto res = root_nodes_.try_get(frame_id.external_render_pass.black_node_name, DP_LOC);
            if (!res.has_value()) {
                THROW_OR_ABORT("Could not find black node with name \"" + *frame_id.external_render_pass.black_node_name + '"');
            }
            return *res;
        }();
        node->render(vp, TransformationMatrix<float, ScenePos, 3>::identity(), iv, camera_node, nullptr, lights, skidmarks, blended, render_config, scene_graph_config, frame_id, nullptr, color_styles);
    } else if (frame_id.external_render_pass.pass == ExternalRenderPassType::LIGHTMAP_BLACK_MOVABLES) {
        NodeDanglingPtrs nodes{ CHUNK_SIZE };
        {
            std::shared_lock lock{ mutex_ };
            root_nodes_.visit(iv.t, [&](const auto& node) {
                if (node->is_visible_for_user(frame_id.external_render_pass.user_id)) {
                    nodes.emplace_back(node.ptr());
                }
                return true;
            });
        }
        for (const auto& node : nodes) {
            node->render(vp, TransformationMatrix<float, ScenePos, 3>::identity(), iv, camera_node, {}, lights, skidmarks, blended, render_config, scene_graph_config, frame_id, nullptr, color_styles);
        }
    } else {
        if (!frame_id.external_render_pass.black_node_name->empty()) {
            THROW_OR_ABORT("Expected empty black node");
        }
        // |         |Lights|Blended|Large|Small|Move|
        // |---------|------|-------|-----|-----|----|
        // |Dynamic  |x     |x      |     |     |x   |
        // |Static   |x     |x      |x    |x    |    |
        // |Aggregate|      |       |x    |x    |    |
        LOG_INFO("Scene::render lights");
        NodeDanglingPtrs local_root_nodes{ CHUNK_SIZE };
        NodeRawPtrs local_static_root_nodes{ CHUNK_SIZE };
        {
            std::shared_lock lock{ mutex_ };
            root_nodes_.visit(iv.t, [&](const auto& node) {
                if (node->is_visible_for_user(frame_id.external_render_pass.user_id)) {
                    local_root_nodes.emplace_back(node.ptr());
                }
                return true;
            });
            static_root_nodes_.visit(iv.t, [&](const auto& node) {
                if (node->is_visible_for_user(frame_id.external_render_pass.user_id)) {
                    local_static_root_nodes.emplace_back(&node.get());
                }
                return true;
            });
        }
        for (const auto& node : local_root_nodes) {
            node->append_lights_to_queue(TransformationMatrix<float, ScenePos, 3>::identity(), lights);
            node->append_skidmarks_to_queue(TransformationMatrix<float, ScenePos, 3>::identity(), skidmarks);
        }
        for (const auto& node : local_static_root_nodes) {
            node->append_lights_to_queue(TransformationMatrix<float, ScenePos, 3>::identity(), lights);
        }
        if (any(frame_id.external_render_pass.pass & ExternalRenderPassType::IMPOSTER_OR_ZOOM_NODE)) {
            if (frame_id.external_render_pass.singular_node == nullptr) {
                THROW_OR_ABORT("Imposter or singular node pass without a singular node");
            }
            auto parent_m = frame_id.external_render_pass.singular_node->has_parent()
                ? frame_id.external_render_pass.singular_node->parent()->absolute_model_matrix()
                : TransformationMatrix<float, ScenePos, 3>::identity();
            auto parent_mvp = dot2d(vp, parent_m.affine());
            frame_id.external_render_pass.singular_node->render(parent_mvp, parent_m, iv, camera_node, {}, lights, skidmarks, blended, render_config, scene_graph_config, frame_id, nullptr, color_styles);
        } else {
            if (dynamic_lights_ != nullptr) {
                dynamic_lights_->set_time(frame_id.external_render_pass.time);
            }
            LOG_INFO("Scene::render non-blended");
            for (const auto& node : local_root_nodes) {
                node->render(vp, TransformationMatrix<float, ScenePos, 3>::identity(), iv, camera_node, dynamic_lights_, lights, skidmarks, blended, render_config, scene_graph_config, frame_id, nullptr, color_styles);
            }
            for (const auto& node : local_static_root_nodes) {
                node->render(vp, TransformationMatrix<float, ScenePos, 3>::identity(), iv, nullptr, dynamic_lights_, lights, skidmarks, blended, render_config, scene_graph_config, frame_id, nullptr, color_styles);
            }
            {
                NodeDanglingPtrs cached_imposter_nodes{ CHUNK_SIZE };
                {
                    std::shared_lock lock{ mutex_ };
                    auto avail = root_imposter_nodes_.find(frame_id.external_render_pass.renderable_scene);
                    if (avail != root_imposter_nodes_.end()) {
                        for (const auto& node : avail->second) {
                            cached_imposter_nodes.emplace_back(node);
                        }
                    }
                }
                for (const auto& node : cached_imposter_nodes) {
                    node->render(vp, TransformationMatrix<float, ScenePos, 3>::identity(), iv, camera_node, dynamic_lights_, lights, skidmarks, blended, render_config, scene_graph_config, frame_id, nullptr, color_styles);
                }
            }
            {
                bool is_foreground_task = any(frame_id.external_render_pass.pass & ExternalRenderPassType::IS_GLOBAL_MASK);
                bool is_background_task = any(frame_id.external_render_pass.pass & ExternalRenderPassType::STANDARD_MASK);
                if (is_foreground_task && is_background_task) {
                    THROW_OR_ABORT("Scene::render has both foreground and background task");
                }

                std::shared_ptr<IAggregateRenderer> large_aggregate_renderer = IAggregateRenderer::large_aggregate_renderer();
                if (large_aggregate_renderer != nullptr) {
                    LOG_INFO("Scene::render large_aggregate_renderer");
                    auto large_aggregate_renderer_update_func = [&](TaskLocation task_location){
                        // copy "vp" and "scene_graph_config"
                        return run_in_background([this, iv, scene_graph_config, external_render_pass=frame_id.external_render_pass, large_aggregate_renderer, task_location](){
                            NodeRawPtrs nodes{ CHUNK_SIZE };
                            {
                                std::shared_lock lock{ mutex_ };
                                root_aggregate_once_nodes_.visit(iv.t, [&nodes](const auto& node) { nodes.emplace_back(&node.get()); return true; });
                            }
                            std::list<std::shared_ptr<ColoredVertexArray<float>>> aggregate_queue;
                            for (const auto& node : nodes) {
                                node->append_large_aggregates_to_queue(TransformationMatrix<float, ScenePos, 3>::identity(), iv.t, aggregate_queue, scene_graph_config);
                            }
                            large_aggregate_renderer->update_aggregates(iv.t, aggregate_queue, external_render_pass, task_location);
                        });
                    };
                    if (is_foreground_task) {
                        large_aggregate_renderer_update_func(TaskLocation::FOREGROUND)();
                    } else if (is_background_task) {
                        auto* worker = IAggregateRenderer::large_aggregate_bg_worker();
                        if (worker == nullptr) {
                            THROW_OR_ABORT("Scene::render: large_aggregate_bg_worker not set");
                        }
                        if (!large_aggregate_renderer->is_initialized()) {
                            worker->wait_until_done();
                            large_aggregate_renderer_update_func(TaskLocation::FOREGROUND)();
                        } else if (worker->done()) {
                            auto dist = sum(squared(large_aggregate_renderer->offset() - iv.t));
                            if (dist > squared(scene_graph_config.large_max_offset_deviation)) {
                                worker->run(large_aggregate_renderer_update_func(TaskLocation::BACKGROUND));
                            }
                        }
                    }
                    // AperiodicLagFinder lag_finder{ "Large aggregates: ", std::chrono::milliseconds{5} };
                    large_aggregate_renderer->render_aggregates(
                        vp,
                        iv,
                        lights,
                        skidmarks,
                        scene_graph_config,
                        render_config,
                        frame_id,
                        animation_state.get(),
                        color_styles);
                }

                std::shared_ptr<IInstancesRenderer> large_instances_renderer = IInstancesRenderer::large_instances_renderer();
                if (large_instances_renderer != nullptr) {
                    LOG_INFO("Scene::render large_instances_renderer");
                    auto large_instances_renderer_update_func = [&](TaskLocation task_location){
                        // copy "vp" and "scene_graph_config"
                        return run_in_background([this, vp, iv, scene_graph_config, external_render_pass_type=frame_id.external_render_pass.pass,
                                                  large_instances_renderer, task_location]()
                        {
                            NodeRawPtrs nodes{ CHUNK_SIZE };
                            {
                                std::shared_lock lock{ mutex_ };
                                root_instances_once_nodes_.visit(iv.t, [&nodes](const auto& node) { nodes.emplace_back(&node.get()); return true; });
                            }
                            LargeInstancesQueue instances_queue{external_render_pass_type};
                            for (const auto& node : nodes) {
                                node->append_large_instances_to_queue(vp, TransformationMatrix<float, ScenePos, 3>::identity(), iv.t, PositionAndYAngleAndBillboardId{fixed_zeros<CompressedScenePos, 3>(), BILLBOARD_ID_NONE, 0.f}, instances_queue, scene_graph_config);
                            }
                            large_instances_renderer->update_instances(iv.t, instances_queue.queue(), task_location);
                        });
                    };
                    if (is_foreground_task) {
                        large_instances_renderer_update_func(TaskLocation::FOREGROUND)();
                    } else if (is_background_task) {
                        auto* worker = IInstancesRenderer::large_instances_bg_worker();
                        if (worker == nullptr) {
                            THROW_OR_ABORT("Scene::render: large_instances_bg_worker not set");
                        }
                        if (!large_instances_renderer->is_initialized()) {
                            worker->wait_until_done();
                            large_instances_renderer_update_func(TaskLocation::FOREGROUND)();
                        } else if (worker->done()) {
                            auto dist = sum(squared(large_instances_renderer->offset() - iv.t));
                            if (dist > squared(scene_graph_config.large_max_offset_deviation)) {
                                worker->run(large_instances_renderer_update_func(TaskLocation::BACKGROUND));
                            }
                        }
                    }
                    // AperiodicLagFinder lag_finder{ "large instances: ", std::chrono::milliseconds{5} };
                    large_instances_renderer->render_instances(vp, iv, lights, skidmarks, scene_graph_config, render_config, frame_id);
                }
                {
                    // AperiodicLagFinder lag_finder{ "blended early: ", std::chrono::milliseconds{5} };
                    LOG_INFO("Scene::render early blended");
                    blended.early.render(
                        dynamic_lights_,
                        iv,
                        lights,
                        skidmarks,
                        scene_graph_config,
                        render_config,
                        { frame_id, InternalRenderPass::BLENDED_EARLY });
                }
                std::shared_ptr<IAggregateRenderer> small_sorted_aggregate_renderer = IAggregateRenderer::small_sorted_aggregate_renderer();
                if (small_sorted_aggregate_renderer != nullptr) {
                    // Contains continuous alpha and must therefore be rendered late.
                    LOG_INFO("Scene::render small_sorted_aggregate_renderer");
                    auto small_sorted_aggregate_renderer_update_func = [&](TaskLocation task_location){
                        // copy "vp" and "scene_graph_config"
                        return run_in_background([this, vp, iv, scene_graph_config, external_render_pass=frame_id.external_render_pass, small_sorted_aggregate_renderer, task_location](){
                            NodeRawPtrs nodes{ CHUNK_SIZE };
                            {
                                std::shared_lock lock{ mutex_ };
                                root_aggregate_always_nodes_.visit(iv.t, [&nodes](const auto& node) { nodes.emplace_back(&node.get()); return true; });
                            }
                            std::list<std::pair<float, std::shared_ptr<ColoredVertexArray<float>>>> aggregate_queue;
                            for (const auto& node : nodes) {
                                node->append_sorted_aggregates_to_queue(vp, TransformationMatrix<float, ScenePos, 3>::identity(), iv.t, aggregate_queue, scene_graph_config, external_render_pass);
                            }
                            aggregate_queue.sort([](auto& a, auto& b){ return a.first < b.first; });
                            std::list<std::shared_ptr<ColoredVertexArray<float>>> sorted_aggregate_queue;
                            for (auto& e : aggregate_queue) {
                                sorted_aggregate_queue.push_back(std::move(e.second));
                            }
                            small_sorted_aggregate_renderer->update_aggregates(iv.t, sorted_aggregate_queue, external_render_pass, task_location);
                        });
                    };
                    if (is_foreground_task) {
                        small_sorted_aggregate_renderer_update_func(TaskLocation::FOREGROUND)();
                    } else if (is_background_task) {
                        auto* worker = IAggregateRenderer::small_aggregate_bg_worker();
                        if (worker == nullptr) {
                            THROW_OR_ABORT("Scene::render: large_instances_bg_worker not set");
                        }
                        if (!small_sorted_aggregate_renderer->is_initialized()) {
                            worker->wait_until_done();
                            small_sorted_aggregate_renderer_update_func(TaskLocation::FOREGROUND)();
                        } else if (worker->done()) {
                            WorkerStatus status = worker->tick(scene_graph_config.small_aggregate_update_interval);
                            if (status == WorkerStatus::IDLE) {
                                worker->run(small_sorted_aggregate_renderer_update_func(TaskLocation::BACKGROUND));
                            }
                        }
                    }
                    // AperiodicLagFinder lag_finder{ "Small sorted aggregates: ", std::chrono::milliseconds{5} };
                    small_sorted_aggregate_renderer->render_aggregates(
                        vp,
                        iv,
                        lights,
                        skidmarks,
                        scene_graph_config,
                        render_config,
                        frame_id,
                        animation_state.get(),
                        color_styles);
                }

                // Contains continuous alpha and must therefore be rendered late.
                LOG_INFO("Scene::render instances_renderer");
                std::shared_ptr<IInstancesRenderers> small_sorted_instances_renderers = IInstancesRenderer::small_sorted_instances_renderers();
                if (small_sorted_instances_renderers != nullptr) {
                    if (any(frame_id.external_render_pass.pass & ExternalRenderPassType::STANDARD_MASK) ||
                        any(frame_id.external_render_pass.pass & ExternalRenderPassType::IS_GLOBAL_MASK))
                    {
                        auto small_instances_renderer_update_func = [&](TaskLocation task_location){
                            std::set<ExternalRenderPassType> black_render_passes;
                            if (any(frame_id.external_render_pass.pass & ExternalRenderPassType::STANDARD_MASK)) {
                                for (const auto &[_, l] : lights) {
                                    if (any(l->shadow_render_pass & ExternalRenderPassType::LIGHTMAP_IS_LOCAL_MASK)) {
                                        black_render_passes.insert(l->shadow_render_pass);
                                    }
                                }
                            }
                            // copy "vp" and "scene_graph_config"
                            return run_in_background([this, vp, iv, scene_graph_config, external_render_pass_type=frame_id.external_render_pass.pass,
                                                      small_sorted_instances_renderers, task_location,
                                                      black_render_passes]()
                            {
                                NodeRawPtrs nodes{ CHUNK_SIZE };
                                {
                                    std::shared_lock lock{ mutex_ };
                                    root_instances_always_nodes_.visit(iv.t, [&nodes](const auto& node) { nodes.emplace_back(&node.get()); return true; });
                                }
                                // auto start_time = std::chrono::steady_clock::now();
                                auto main_render_pass = black_render_passes.empty()
                                    ? external_render_pass_type
                                    : ExternalRenderPassType::STANDARD_AND_LOCAL_LIGHTMAP;
                                SmallInstancesQueues instances_queues{
                                    main_render_pass,
                                    black_render_passes};
                                for (const auto& node : nodes) {
                                    node->append_small_instances_to_queue(vp, TransformationMatrix<float, ScenePos, 3>::identity(), iv, iv.t, PositionAndYAngleAndBillboardId{fixed_zeros<CompressedScenePos, 3>(), BILLBOARD_ID_NONE, 0.f}, instances_queues, scene_graph_config);
                                }
                                auto sorted_instances = instances_queues.sorted_instances();
                                small_sorted_instances_renderers->get_instances_renderer(external_render_pass_type)->update_instances(
                                    iv.t,
                                    sorted_instances.at(main_render_pass),
                                    task_location);
                                for (auto rp : black_render_passes) {
                                    small_sorted_instances_renderers->get_instances_renderer(rp)->update_instances(
                                        iv.t,
                                        sorted_instances.at(rp),
                                        task_location);
                                }
                                // lerr() << this << " " << external_render_pass_type << ", elapsed time: " << std::chrono::duration<float>(std::chrono::steady_clock::now() - start_time).count() << " s";
                            });
                        };
                        if (is_foreground_task) {
                            small_instances_renderer_update_func(TaskLocation::FOREGROUND)();
                        } else if (is_background_task) {
                            auto* worker = IInstancesRenderer::small_instances_bg_worker();
                            if (worker == nullptr) {
                                THROW_OR_ABORT("Scene::render: small_instances_bg_worker not set");
                            }
                            if (!small_sorted_instances_renderers->get_instances_renderer(frame_id.external_render_pass.pass)->is_initialized()) {
                                worker->wait_until_done();
                                small_instances_renderer_update_func(TaskLocation::FOREGROUND)();
                            } else if (worker->done()) {
                                WorkerStatus status = worker->tick(scene_graph_config.small_aggregate_update_interval);
                                if (status == WorkerStatus::IDLE) {
                                    worker->run(small_instances_renderer_update_func(TaskLocation::BACKGROUND));
                                }
                            }
                        }
                    }
                    // AperiodicLagFinder lag_finder{ "Small sorted instances: ", std::chrono::milliseconds{5} };
                    small_sorted_instances_renderers->get_instances_renderer(frame_id.external_render_pass.pass)->render_instances(
                        vp, iv, lights, skidmarks, scene_graph_config, render_config, frame_id);
                }
            }
            if (any(frame_id.external_render_pass.pass & ExternalRenderPassType::STANDARD_MASK)) {
                if (trail_renderer_ != nullptr) {
                    trail_renderer_->render(
                        vp,
                        iv,
                        lights,
                        skidmarks,
                        scene_graph_config,
                        render_config,
                        frame_id);
                }
            }
        }
    }
    {
        // AperiodicLagFinder lag_finder{ "blended late: ", std::chrono::milliseconds{5} };
        LOG_INFO("Scene::render late blended");
        blended.late.render(
            dynamic_lights_,
            iv,
            lights,
            skidmarks,
            scene_graph_config,
            render_config,
            { frame_id, InternalRenderPass::BLENDED_LATE });
    }
}

void Scene::move(float dt, std::chrono::steady_clock::time_point time) {
    LOG_FUNCTION("Scene::move");
    delete_node_mutex_.assert_this_thread_is_deleter_thread();
    {
        std::unique_lock lock{mutex_};
        {
            auto &drn = root_nodes_.default_nodes();
            for (auto it = drn.begin(); it != drn.end();) {
                it->second->move(
                    TransformationMatrix<float, ScenePos, 3>::identity(),
                    dt,
                    time,
                    scene_node_resources_,
                    nullptr);  // animation_state
                if (it->second->to_be_deleted(time)) {
                    UnlockGuard ug{lock};
                    delete_root_node((it++)->first);
                } else {
                    ++it;
                }
            }
        }
        if (dynamic_lights_ != nullptr) {
            dynamic_lights_->append_time(time);
        }
        // times_.append(time);
    }
    try_empty_the_trash_can();
}

void Scene::append_physics_to_queue(
    std::list<std::pair<TransformationMatrix<float, ScenePos, 3>, std::shared_ptr<ColoredVertexArray<float>>>>& float_queue,
    std::list<std::pair<TransformationMatrix<float, ScenePos, 3>, std::shared_ptr<ColoredVertexArray<CompressedScenePos>>>>& double_queue) const
{
    LOG_FUNCTION("Scene::append_physics_to_queue");
    std::shared_lock lock{ mutex_ };
    auto zero = PositionAndYAngleAndBillboardId{fixed_zeros<CompressedScenePos, 3>(), BILLBOARD_ID_NONE, 0.f};
    static_root_nodes_.visit_all([&](const auto& node) {
        node->append_physics_to_queue(
            TransformationMatrix<float, ScenePos, 3>::identity(),
            zero,
            float_queue,
            double_queue);
        return true;
        });
    root_aggregate_once_nodes_.visit_all([&](const auto& node) {
        node->append_physics_to_queue(
            TransformationMatrix<float, ScenePos, 3>::identity(),
            zero,
            float_queue,
            double_queue);
        return true;
        });
    root_aggregate_always_nodes_.visit_all([&](const auto& node) {
        node->append_physics_to_queue(
            TransformationMatrix<float, ScenePos, 3>::identity(),
            zero,
            float_queue,
            double_queue);
        return true;
        });
    root_instances_once_nodes_.visit_all([&](const auto& node) {
        node->append_physics_to_queue(
            TransformationMatrix<float, ScenePos, 3>::identity(),
            zero,
            float_queue,
            double_queue);
        return true;
        });
    root_instances_always_nodes_.visit_all([&](const auto& node) {
        node->append_physics_to_queue(
            TransformationMatrix<float, ScenePos, 3>::identity(),
            zero,
            float_queue,
            double_queue);
        return true;
        });
    static_root_physics_nodes_.visit_all([&](const auto& node) {
        node->append_physics_to_queue(
            TransformationMatrix<float, ScenePos, 3>::identity(),
            zero,
            float_queue,
            double_queue);
        return true;
        });
}

size_t Scene::get_uuid() {
    std::scoped_lock lock{ uuid_mutex_ };
    return uuid_++;
}

std::string Scene::get_temporary_instance_suffix() {
    return "___" + std::to_string(get_uuid());
}

void Scene::print(std::ostream& ostr) const {
    std::shared_lock lock{ mutex_ };
    ostr << "Scene\n";
    ostr << morn_;
}

bool Scene::shutting_down() const {
    return shutting_down_;
}

void Scene::set_animation_state(
    std::unique_ptr<AnimationState>&& animation_state,
    AnimationStateAlreadyExistsBehavior already_exists_behavior)
{
    delete_node_mutex_.assert_this_thread_is_deleter_thread();
    std::scoped_lock lock{ mutex_ };
    if ((already_exists_behavior == AnimationStateAlreadyExistsBehavior::THROW) &&
        (animation_state_ != nullptr))
    {
        THROW_OR_ABORT("Animation state already set");
    }
    animation_state_ = std::move(animation_state);
}

void Scene::add_color_style(std::unique_ptr<ColorStyle>&& color_style) {
    color_style->compute_hash();
    color_styles_.push_back(std::move(color_style));
}

void Scene::clear_color_styles() {
    color_styles_.clear();
}

DeleteNodeMutex& Scene::delete_node_mutex() const {
    return delete_node_mutex_;
}

void Scene::set_this_thread_as_render_thread() {
    if (render_thread_id_ != std::thread::id()) {
        verbose_abort("Render thread already set");
    }
    render_thread_id_ = std::this_thread::get_id();
}

void Scene::clear_render_thread() {
    if (render_thread_id_ == std::thread::id()) {
        verbose_abort("Render thread not set");
    }
    render_thread_id_ = std::thread::id();
}

void Scene::assert_this_thread_is_render_thread() const {
    auto id = std::this_thread::get_id();
    if (id != render_thread_id_) {
        std::stringstream sstr;
        sstr << "Thread \"" << id << "\" is not the render thread \"" << render_thread_id_ << '"';
        verbose_abort(sstr.str());
    }
}

void Scene::wait_for_cleanup() const {
    while (ncleanups_required_ > 0);
}

void Scene::notify_cleanup_required() {
    ++ncleanups_required_;
}

void Scene::notify_cleanup_done() {
    --ncleanups_required_;
}

std::ostream& Mlib::operator << (std::ostream& ostr, const Scene& scene) {
    scene.print(ostr);
    return ostr;
}
