#include "Scene.hpp"
#include <Mlib/Geometry/Coordinates/Homogeneous.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Transformed_Colored_Vertex_Array.hpp>
#include <Mlib/Log.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Memory/Recursive_Deletion.hpp>
#include <Mlib/Scene_Graph/Batch_Renderers/IAggregate_Renderer.hpp>
#include <Mlib/Scene_Graph/Batch_Renderers/IInstances_Renderer.hpp>
#include <Mlib/Scene_Graph/Containers/Root_Nodes.hpp>
#include <Mlib/Scene_Graph/Delete_Node_Mutex.hpp>
#include <Mlib/Scene_Graph/Elements/Color_Style.hpp>
#include <Mlib/Scene_Graph/Elements/Renderable.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Instances/Large_Instances_Queue.hpp>
#include <Mlib/Scene_Graph/Instances/Small_Instances_Queues.hpp>
#include <Mlib/Scene_Graph/Interfaces/IParticle_Renderer.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>
#include <Mlib/Scene_Graph/Scene_Graph_Config.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <mutex>

using namespace Mlib;

Scene::Scene(
    DeleteNodeMutex& delete_node_mutex,
    SceneNodeResources* scene_node_resources,
    IParticleRenderer* particle_renderer)
: morn_{ *this },
  root_nodes_{ morn_.create("root_nodes") },
  static_root_nodes_{ morn_.create("static_root_nodes") },
  root_aggregate_nodes_{ morn_.create("root_aggregate_nodes") },
  root_instances_nodes_{ morn_.create("root_instances_nodes") },
  delete_node_mutex_{ delete_node_mutex },
  uuid_{ 0 },
  shutting_down_{ false },
  scene_node_resources_{ scene_node_resources },
  particle_renderer_{ particle_renderer }
{}

void Scene::add_root_node(
    const std::string& name,
    std::unique_ptr<SceneNode>&& scene_node)
{
    std::scoped_lock lock{mutex_};
    LOG_FUNCTION("Scene::add_root_node");
    scene_node->set_scene_and_state(*this, SceneNodeState::DYNAMIC);
    root_nodes_.add_root_node(name, std::move(scene_node));
}

void Scene::add_static_root_node(
    const std::string& name,
    std::unique_ptr<SceneNode>&& scene_node)
{
    std::scoped_lock lock{mutex_};
    scene_node->set_scene_and_state(*this, SceneNodeState::STATIC);
    static_root_nodes_.add_root_node(name, std::move(scene_node));
}

void Scene::add_root_aggregate_node(
    const std::string& name,
    std::unique_ptr<SceneNode>&& scene_node)
{
    std::scoped_lock lock{mutex_};
    scene_node->set_scene_and_state(*this, SceneNodeState::STATIC);
    root_aggregate_nodes_.add_root_node(name, std::move(scene_node));
}

void Scene::add_root_instances_node(
    const std::string& name,
    std::unique_ptr<SceneNode>&& scene_node)
{
    std::scoped_lock lock{mutex_};
    scene_node->set_scene_and_state(*this, SceneNodeState::STATIC);
    root_instances_nodes_.add_root_node(name, std::move(scene_node));
}

void Scene::add_root_imposter_node(SceneNode* scene_node)
{
    std::scoped_lock lock{mutex_};
    scene_node->set_scene_and_state(*this, SceneNodeState::DYNAMIC);
    if (!root_imposter_nodes_.insert(scene_node).second)
    {
        THROW_OR_ABORT("Root imposter node already exists");
    }
}

bool Scene::root_node_scheduled_for_deletion(
    const std::string& name,
    bool must_exist) const
{
    std::shared_lock lock{mutex_};
    return morn_.root_node_scheduled_for_deletion(name, must_exist);
}

void Scene::schedule_delete_root_node(const std::string& name) {
    std::scoped_lock lock{mutex_};
    root_nodes_.schedule_delete_root_node(name);
}

void Scene::delete_scheduled_root_nodes() const {
    std::scoped_lock lock{mutex_};
    morn_.delete_scheduled_root_nodes();
}

void Scene::try_delete_root_node(const std::string& name) {
    std::scoped_lock lock{mutex_};
    delete_node_mutex_.notify_deleting();
    if (nodes_.contains(name)) {
        delete_root_node(name);
    }
}

void Scene::delete_root_imposter_node(SceneNode& scene_node) {
    if (root_imposter_nodes_.erase(&scene_node) != 1) {
        verbose_abort("Could not delete root imposter node");
    }
}

void Scene::delete_root_node(const std::string& name) {
    LOG_FUNCTION("Scene::delete_root_node");
    std::scoped_lock lock{mutex_};
    root_nodes_.delete_root_node(name);
}

void Scene::delete_root_nodes(const Mlib::regex& regex) {
    LOG_FUNCTION("Scene::delete_root_nodes");
    std::scoped_lock lock{mutex_};
    root_nodes_.delete_root_nodes(regex);
}

void Scene::try_delete_node(const std::string& name) {
    std::scoped_lock lock{mutex_};
    delete_node_mutex_.notify_deleting();
    if (nodes_.contains(name)) {
        delete_node(name);
    }
}

void Scene::delete_node(const std::string& name) {
    std::scoped_lock lock{mutex_};
    delete_node_mutex_.notify_deleting();
    SceneNode& node = get_node_that_may_be_scheduled_for_deletion(name);
    if (!node.shutting_down()) {
        if (node.has_parent()) {
            node.parent().remove_child(name);
        } else {
            delete_root_node(name);
        }
    }
}

void Scene::delete_nodes(const Mlib::regex& regex) {
    std::scoped_lock lock{mutex_};
    delete_node_mutex_.notify_deleting();
    for (auto it = nodes_.begin(); it != nodes_.end(); ) {
        auto n = it++;
        if (Mlib::re::regex_match(n->first, regex)) {
            delete_node(it->first);
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
    clear_nodes_not_allowed_to_be_unregistered();
    if (!delete_node_mutex_.is_locked_by_this_thread()) {
        verbose_abort("Scene::shutdown: delete node mutex is not locked");
    }
    if (!nodes_not_allowed_to_be_unregistered_.empty()) {
        verbose_abort("Scene::shutdown: some nodes are not allowed to be deleted");
    }
    shutting_down_ = true;
    large_aggregate_bg_worker_.shutdown();
    large_instances_bg_worker_.shutdown();
    small_aggregate_bg_worker_.shutdown();
    small_instances_bg_worker_.shutdown();
    morn_.clear();
    if (!nodes_.empty()) {
        for (const auto& [name, _] : nodes_) {
            lerr() << name;
        }
        verbose_abort("Registered nodes remain after shutdown");
    }
    if (!root_imposter_nodes_.empty()) {
        verbose_abort("Imposter nodes remain after shutdown");
    }
}

bool Scene::contains_node(const std::string& name) const {
    std::shared_lock lock{mutex_};
    return nodes_.contains(name);
}

void Scene::register_node(
    const std::string& name,
    SceneNode& scene_node)
{
    std::scoped_lock lock{mutex_};
    if (name.empty()) {
        THROW_OR_ABORT("register_node received empty name");
    }
    if (!nodes_.insert({ name, &scene_node }).second) {
        THROW_OR_ABORT("Scene node with name \"" + name + "\" already exists");
    }
}

void Scene::unregister_node(const std::string& name) {
    std::scoped_lock lock{mutex_};
    delete_node_mutex_.assert_this_thread_is_deleter_thread();
    if (nodes_not_allowed_to_be_unregistered_.contains(name)) {
        verbose_abort("Node \"" + name + "\" may not be unregistered");
    }
    if (!delete_node_mutex_.is_locked_by_this_thread()) {
        verbose_abort("Scene::unregister_node: delete node mutex is not locked");
    }
    if (nodes_.erase(name) != 1) {
        verbose_abort("Could not find node with name (0) \"" + name + '"');
    }
}

void Scene::unregister_nodes(const Mlib::regex& regex) {
    std::scoped_lock lock{mutex_};
    delete_node_mutex_.assert_this_thread_is_deleter_thread();
    if (!delete_node_mutex_.is_locked_by_this_thread()) {
        verbose_abort("Scene::unregister_nodes: delete node mutex is not locked");
    }
    for (auto it = nodes_.begin(); it != nodes_.end(); ) {
        auto n = *it++;
        if (Mlib::re::regex_match(n.first, regex)) {
            if (nodes_not_allowed_to_be_unregistered_.contains(n.first)) {
                verbose_abort("Node \"" + n.first + "\" may not be unregistered");
            }
            if (nodes_.erase(n.first) != 1) {
                verbose_abort("Could not find node with name (1) \"" + n.first + '"');
            }
        }
    }
}

SceneNode& Scene::get_node(const std::string& name) const {
    std::shared_lock lock{mutex_};
    if (morn_.root_node_scheduled_for_deletion(name, false)) {
        THROW_OR_ABORT("Node \"" + name + "\" is scheduled for deletion");
    }
    return get_node_that_may_be_scheduled_for_deletion(name);
}

SceneNode& Scene::get_node_that_may_be_scheduled_for_deletion(const std::string& name) const {
    auto it = nodes_.find(name);
    if (it == nodes_.end()) {
        THROW_OR_ABORT("Could not find node with name (2) \"" + name + '"');
    }
    return *it->second;
}

void Scene::render(
    const FixedArray<double, 4, 4>& vp,
    const TransformationMatrix<float, double, 3>& iv,
    const SceneNode& camera_node,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    const ExternalRenderPass& external_render_pass,
    const std::function<std::function<void()>(std::function<void()>)>& run_in_background) const
{
    std::shared_lock lock{mutex_};
    LOG_FUNCTION("Scene::render");
    if (!delete_node_mutex_.is_locked_by_this_thread()) {
        THROW_OR_ABORT("Scene::render: delete node mutex is not locked");
    }
    std::list<std::pair<TransformationMatrix<float, double, 3>, Light*>> lights;
    std::list<Blended> blended;
    std::list<const ColorStyle*> color_styles;
    for (const auto& s : color_styles_) {
        color_styles.push_back(s.get());
    }
    if (external_render_pass.pass == ExternalRenderPassType::LIGHTMAP_BLACK_NODE) {
        auto it = root_nodes_.find(external_render_pass.black_node_name);
        if (it == root_nodes_.end()) {
            THROW_OR_ABORT("Could not find black node with name \"" + external_render_pass.black_node_name + '"');
        }
        it->second->render(vp, TransformationMatrix<float, double, 3>::identity(), iv, camera_node, lights, blended, render_config, scene_graph_config, external_render_pass, nullptr, color_styles);
    } else {
        if (!external_render_pass.black_node_name.empty()) {
            THROW_OR_ABORT("Expected empty black node");
        }
        // |         |Lights|Blended|Large|Small|Move|
        // |---------|------|-------|-----|-----|----|
        // |Dynamic  |x     |x      |     |     |x   |
        // |Static   |x     |x      |x    |x    |    |
        // |Aggregate|      |       |x    |x    |    |
        LOG_INFO("Scene::render lights");
        for (const auto& [_, node] : root_nodes_) {
            node->append_lights_to_queue(TransformationMatrix<float, double, 3>::identity(), lights);
        }
        for (const auto& [_, node] : static_root_nodes_) {
            node->append_lights_to_queue(TransformationMatrix<float, double, 3>::identity(), lights);
        }
        if (external_render_pass.pass == ExternalRenderPassType::IMPOSTER_NODE) {
            if (external_render_pass.singular_node == nullptr) {
                THROW_OR_ABORT("Imposter node pass without singular node");
            }
            auto parent_m = external_render_pass.singular_node->has_parent()
                ? external_render_pass.singular_node->parent().absolute_model_matrix()
                : TransformationMatrix<float, double, 3>::identity();
            auto parent_mvp = dot2d(vp, parent_m.affine());
            external_render_pass.singular_node->render(parent_mvp, parent_m, iv, camera_node, lights, blended, render_config, scene_graph_config, external_render_pass, nullptr, color_styles);
        } else {
            LOG_INFO("Scene::render non-blended");
            for (const auto& [_, node] : root_nodes_) {
                node->render(vp, TransformationMatrix<float, double, 3>::identity(), iv, camera_node, lights, blended, render_config, scene_graph_config, external_render_pass, nullptr, color_styles);
            }
            for (const auto& node : root_imposter_nodes_) {
                node->render(vp, TransformationMatrix<float, double, 3>::identity(), iv, camera_node, lights, blended, render_config, scene_graph_config, external_render_pass, nullptr, color_styles);
            }
            for (const auto& [_, node] : static_root_nodes_) {
                node->render(vp, TransformationMatrix<float, double, 3>::identity(), iv, camera_node, lights, blended, render_config, scene_graph_config, external_render_pass, nullptr, color_styles);
            }
            {
                bool is_foreground_task = any(external_render_pass.pass & ExternalRenderPassType::IS_STATIC_MASK);
                bool is_background_task = (external_render_pass.pass == ExternalRenderPassType::STANDARD);
                if (is_foreground_task && is_background_task) {
                    THROW_OR_ABORT("Scene::render has both foreground and background task");
                }

                std::shared_ptr<IAggregateRenderer> large_aggregate_renderer = IAggregateRenderer::large_aggregate_renderer();
                if (large_aggregate_renderer != nullptr) {
                    LOG_INFO("Scene::render large_aggregate_renderer");
                    auto large_aggregate_renderer_update_func = [&](){
                        // copy "vp" and "scene_graph_config"
                        return run_in_background([this, iv, scene_graph_config, external_render_pass, large_aggregate_renderer](){
                            std::list<std::shared_ptr<ColoredVertexArray<float>>> aggregate_queue;
                            for (const auto& [_, node] : static_root_nodes_) {
                                node->append_large_aggregates_to_queue(TransformationMatrix<float, double, 3>::identity(), iv.t(), aggregate_queue, scene_graph_config);
                            }
                            for (const auto& [_, node] : root_aggregate_nodes_) {
                                node->append_large_aggregates_to_queue(TransformationMatrix<float, double, 3>::identity(), iv.t(), aggregate_queue, scene_graph_config);
                            }
                            large_aggregate_renderer->update_aggregates(iv.t(), aggregate_queue);
                        });
                    };
                    if (is_foreground_task || (is_background_task && !large_aggregate_renderer->is_initialized())) {
                        large_aggregate_renderer_update_func()();
                    } else if (is_background_task && large_aggregate_bg_worker_.done()) {
                        WorkerStatus status = large_aggregate_bg_worker_.tick(scene_graph_config.large_aggregate_update_interval);
                        if (status == WorkerStatus::IDLE) {
                            large_aggregate_bg_worker_.run(large_aggregate_renderer_update_func());
                        }
                    }
                    large_aggregate_renderer->render_aggregates(vp, iv, lights, scene_graph_config, render_config, external_render_pass, color_styles);
                }

                std::shared_ptr<IInstancesRenderer> large_instances_renderer = IInstancesRenderer::large_instances_renderer();
                if (large_instances_renderer != nullptr) {
                    LOG_INFO("Scene::render large_instances_renderer");
                    auto large_instances_renderer_update_func = [&](){
                        // copy "vp" and "scene_graph_config"
                        return run_in_background([this, vp, iv, scene_graph_config, external_render_pass, large_instances_renderer](){
                            LargeInstancesQueue instances_queue{external_render_pass.pass};
                            for (const auto& [_, node] : static_root_nodes_) {
                                node->append_large_instances_to_queue(vp, TransformationMatrix<float, double, 3>::identity(), iv.t(), PositionAndYAngle{fixed_zeros<double, 3>(), 0.f, UINT32_MAX}, instances_queue, scene_graph_config);
                            }
                            for (const auto& [_, node] : root_aggregate_nodes_) {
                                node->append_large_instances_to_queue(vp, TransformationMatrix<float, double, 3>::identity(), iv.t(), PositionAndYAngle{fixed_zeros<double, 3>(), 0.f, UINT32_MAX}, instances_queue, scene_graph_config);
                            }
                            large_instances_renderer->update_instances(iv.t(), instances_queue.queue());
                        });
                    };
                    if (is_foreground_task || (is_background_task && !large_instances_renderer->is_initialized())) {
                        large_instances_renderer_update_func()();
                    } else if (is_background_task && large_instances_bg_worker_.done()) {
                        WorkerStatus status = large_instances_bg_worker_.tick(scene_graph_config.large_aggregate_update_interval);
                        if (status == WorkerStatus::IDLE) {
                            large_instances_bg_worker_.run(large_instances_renderer_update_func());
                        }
                    }
                    large_instances_renderer->render_instances(vp, iv, lights, scene_graph_config, render_config, external_render_pass);
                }

                std::shared_ptr<IAggregateRenderer> small_sorted_aggregate_renderer = IAggregateRenderer::small_sorted_aggregate_renderer();
                if (small_sorted_aggregate_renderer != nullptr) {
                    // Contains continuous alpha and must therefore be rendered late.
                    LOG_INFO("Scene::render small_sorted_aggregate_renderer");
                    auto small_sorted_aggregate_renderer_update_func = [&](){
                        // copy "vp" and "scene_graph_config"
                        return run_in_background([this, vp, iv, scene_graph_config, external_render_pass, small_sorted_aggregate_renderer](){
                            std::list<std::pair<float, std::shared_ptr<ColoredVertexArray<float>>>> aggregate_queue;
                            for (const auto& [_, node] : static_root_nodes_) {
                                node->append_sorted_aggregates_to_queue(vp, TransformationMatrix<float, double, 3>::identity(), iv.t(), aggregate_queue, scene_graph_config, external_render_pass);
                            }
                            for (const auto& [_, node] : root_aggregate_nodes_) {
                                node->append_sorted_aggregates_to_queue(vp, TransformationMatrix<float, double, 3>::identity(), iv.t(), aggregate_queue, scene_graph_config, external_render_pass);
                            }
                            aggregate_queue.sort([](auto& a, auto& b){ return a.first < b.first; });
                            std::list<std::shared_ptr<ColoredVertexArray<float>>> sorted_aggregate_queue;
                            for (auto& e : aggregate_queue) {
                                sorted_aggregate_queue.push_back(std::move(e.second));
                            }
                            small_sorted_aggregate_renderer->update_aggregates(iv.t(), sorted_aggregate_queue);
                        });
                    };
                    if (is_foreground_task || (is_background_task && !small_sorted_aggregate_renderer->is_initialized())) {
                        small_sorted_aggregate_renderer_update_func()();
                    } else if (is_background_task && small_aggregate_bg_worker_.done()) {
                        WorkerStatus status = small_aggregate_bg_worker_.tick(scene_graph_config.small_aggregate_update_interval);
                        if (status == WorkerStatus::IDLE) {
                            small_aggregate_bg_worker_.run(small_sorted_aggregate_renderer_update_func());
                        }
                    }
                    small_sorted_aggregate_renderer->render_aggregates(vp, iv, lights, scene_graph_config, render_config, external_render_pass, color_styles);
                }

                // Contains continuous alpha and must therefore be rendered late.
                LOG_INFO("Scene::render instances_renderer");
                std::shared_ptr<IInstancesRenderers> small_sorted_instances_renderers = IInstancesRenderer::small_sorted_instances_renderers();
                if (small_sorted_instances_renderers != nullptr) {
                    if ((external_render_pass.pass == ExternalRenderPassType::STANDARD) ||
                        any(external_render_pass.pass & ExternalRenderPassType::IS_STATIC_MASK))
                    {
                        auto small_instances_renderer_update_func = [&](){
                            // copy "vp" and "scene_graph_config"
                            return run_in_background([this, vp, iv, scene_graph_config, external_render_pass,
                                                      small_sorted_instances_renderers]()
                            {
                                // auto start_time = std::chrono::steady_clock::now();
                                std::set<ExternalRenderPassType> black_render_passes;
                                if (external_render_pass.pass == ExternalRenderPassType::STANDARD) {
                                    black_render_passes.insert(ExternalRenderPassType::LIGHTMAP_BLOBS);
                                    black_render_passes.insert(ExternalRenderPassType::LIGHTMAP_BLACK_LOCAL_INSTANCES);
                                }
                                SmallInstancesQueues instances_queues{
                                    external_render_pass.pass,
                                    black_render_passes};
                                for (const auto& [_, node] : static_root_nodes_) {
                                    node->append_small_instances_to_queue(vp, TransformationMatrix<float, double, 3>::identity(), iv.t(), PositionAndYAngle{fixed_zeros<double, 3>(), 0.f, UINT32_MAX}, instances_queues, scene_graph_config);
                                }
                                for (const auto& [_, node] : root_instances_nodes_) {
                                    node->append_small_instances_to_queue(vp, TransformationMatrix<float, double, 3>::identity(), iv.t(), PositionAndYAngle{fixed_zeros<double, 3>(), 0.f, UINT32_MAX}, instances_queues, scene_graph_config);
                                }
                                auto sorted_instances = instances_queues.sorted_instances();
                                small_sorted_instances_renderers->get_instances_renderer(external_render_pass.pass)->update_instances(
                                    iv.t(),
                                    sorted_instances.at(external_render_pass.pass));
                                for (auto rp : black_render_passes) {
                                    small_sorted_instances_renderers->get_instances_renderer(rp)->update_instances(
                                        iv.t(),
                                        sorted_instances.at(rp));
                                }
                                // std::cerr << this << " " << external_render_pass.pass << ", elapsed time: " << std::chrono::duration<float>(std::chrono::steady_clock::now() - start_time).count() << " s" << std::endl;
                            });
                        };
                        if (is_foreground_task || (is_background_task && !small_sorted_instances_renderers->get_instances_renderer(external_render_pass.pass)->is_initialized())) {
                            small_instances_renderer_update_func()();
                        } else if (is_background_task && small_instances_bg_worker_.done()) {
                            WorkerStatus status = small_instances_bg_worker_.tick(scene_graph_config.small_aggregate_update_interval);
                            if (status == WorkerStatus::IDLE) {
                                small_instances_bg_worker_.run(small_instances_renderer_update_func());
                            }
                        }
                    }
                    small_sorted_instances_renderers->get_instances_renderer(external_render_pass.pass)->render_instances(
                        vp, iv, lights, scene_graph_config, render_config, external_render_pass);
                }
            }
        }
    }
    if ((particle_renderer_ != nullptr) &&
        (external_render_pass.pass == ExternalRenderPassType::STANDARD))
    {
        particle_renderer_->render(
            vp,
            iv,
            lights,
            scene_graph_config,
            render_config,
            external_render_pass);
    }
    // Contains continuous alpha and must therefore be rendered late.
    LOG_INFO("Scene::render blended");
    blended.sort([](Blended& a, Blended& b){ return a.sorting_key() > b.sorting_key(); });
    for (const auto& b : blended) {
        b.renderable->render(
            b.mvp,
            b.m,
            iv,
            lights,
            scene_graph_config,
            render_config,
            { external_render_pass, InternalRenderPass::BLENDED },
            b.animation_state,
            &b.color_style);
    }
}

void Scene::move(float dt) {
    LOG_FUNCTION("Scene::move");
    std::scoped_lock lock{mutex_};
    delete_node_mutex_.assert_this_thread_is_deleter_thread();
    if (!morn_.no_root_nodes_scheduled_for_deletion()) {
        THROW_OR_ABORT("Moving with root nodes scheduled for deletion");
    }
    for (auto it = root_nodes_.begin(); it != root_nodes_.end(); ) {
        it->second->move(
            TransformationMatrix<float, double, 3>::identity(),
            dt,
            scene_node_resources_,
            nullptr);  // style
        if (it->second->to_be_deleted()) {
            delete_root_node((it++)->first);
        } else {
            ++it;
        }
    }
    if (particle_renderer_ != nullptr) {
        particle_renderer_->move(dt);
    }
}

size_t Scene::get_uuid() {
    std::scoped_lock lock{uuid_mutex_};
    return uuid_++;
}

std::string Scene::get_temporary_instance_suffix() {
    return "___" + std::to_string(get_uuid());
}

void Scene::print(std::ostream& ostr) const {
    std::shared_lock lock{mutex_};
    ostr << "Scene\n";
    ostr << morn_;
}

bool Scene::shutting_down() const {
    std::shared_lock lock{mutex_};
    return shutting_down_;
}

void Scene::add_node_not_allowed_to_be_unregistered(const std::string& name) {
    std::scoped_lock lock{mutex_};
    delete_node_mutex_.assert_this_thread_is_deleter_thread();
    if (!contains_node(name)) {
        THROW_OR_ABORT("Could not find node with name (3) \"" + name + '"');
    }
    if (!nodes_not_allowed_to_be_unregistered_.insert(name).second) {
        THROW_OR_ABORT("Node \"" + name + "\" already in list of nodes not allowed to be unregistered");
    }
}

void Scene::remove_node_not_allowed_to_be_unregistered(const std::string& name) {
    std::scoped_lock lock{mutex_};
    delete_node_mutex_.assert_this_thread_is_deleter_thread();
    if (!contains_node(name)) {
        verbose_abort("Could not find node with name (4) \"" + name + '"');
    }
    if (nodes_not_allowed_to_be_unregistered_.erase(name) != 1) {
        verbose_abort("Could not find node \"" + name + "\" in list of nodes not allowed to be unregistered");
    }
}

void Scene::clear_nodes_not_allowed_to_be_unregistered() {
    std::scoped_lock lock{mutex_};
    delete_node_mutex_.assert_this_thread_is_deleter_thread();
    nodes_not_allowed_to_be_unregistered_.clear();
}

void Scene::add_color_style(std::unique_ptr<ColorStyle>&& color_style) {
    std::scoped_lock lock{mutex_};
    color_styles_.push_back(std::move(color_style));
}

DeleteNodeMutex& Scene::delete_node_mutex() const {
    return delete_node_mutex_;
}

IParticleInstantiator& Scene::particle_instantiator(const std::string& resource_name) const {
    if (particle_renderer_ == nullptr) {
        THROW_OR_ABORT("Particle renderer not set");
    }
    return particle_renderer_->get_instantiator(resource_name);
}

std::ostream& Mlib::operator << (std::ostream& ostr, const Scene& scene) {
    scene.print(ostr);
    return ostr;
}
