#include "Scene.hpp"
#include <Mlib/Geometry/Coordinates/Homogeneous.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Transformed_Colored_Vertex_Array.hpp>
#include <Mlib/Log.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Recursive_Deletion.hpp>
#include <Mlib/Scene_Graph/Aggregate_Renderer.hpp>
#include <Mlib/Scene_Graph/Containers/Root_Nodes.hpp>
#include <Mlib/Scene_Graph/Delete_Node_Mutex.hpp>
#include <Mlib/Scene_Graph/Elements/Color_Style.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Instances_Renderer.hpp>
#include <Mlib/Scene_Graph/Scene_Graph_Config.hpp>

using namespace Mlib;

Scene::Scene(
    DeleteNodeMutex& delete_node_mutex,
    AggregateRenderer* large_aggregate_renderer,
    InstancesRenderer* large_instances_renderer,
    SceneNodeResources* scene_node_resources)
: morn_{ *this },
  root_nodes_{ morn_.create("root_nodes") },
  static_root_nodes_{ morn_.create("static_root_nodes") },
  root_aggregate_nodes_{ morn_.create("root_aggregate_nodes") },
  root_instances_nodes_{ morn_.create("root_instances_nodes") },
  delete_node_mutex_{ delete_node_mutex },
  large_aggregate_renderer_{ large_aggregate_renderer },
  large_instances_renderer_{ large_instances_renderer },
  large_aggregate_renderer_initialized_{ false },
  large_instances_renderer_initialized_{ false },
  uuid_{ 0 },
  shutting_down_{ false },
  scene_node_resources_{ scene_node_resources }
{}

void Scene::add_root_node(
    const std::string& name,
    std::unique_ptr<SceneNode>&& scene_node)
{
    LOG_FUNCTION("Scene::add_root_node");
    root_nodes_.add_root_node(name, std::move(scene_node));
}

void Scene::add_static_root_node(
    const std::string& name,
    std::unique_ptr<SceneNode>&& scene_node)
{
    static_root_nodes_.add_root_node(name, std::move(scene_node));
}

void Scene::add_root_aggregate_node(
    const std::string& name,
    std::unique_ptr<SceneNode>&& scene_node)
{
    root_aggregate_nodes_.add_root_node(name, std::move(scene_node));
}

void Scene::add_root_instances_node(
    const std::string& name,
    std::unique_ptr<SceneNode>&& scene_node)
{
    root_instances_nodes_.add_root_node(name, std::move(scene_node));
}

bool Scene::root_node_scheduled_for_deletion(
    const std::string& name,
    bool must_exist) const
{
    return morn_.root_node_scheduled_for_deletion(name, must_exist);
}

void Scene::schedule_delete_root_node(const std::string& name) {
    root_nodes_.schedule_delete_root_node(name);
}

void Scene::delete_scheduled_root_nodes() const {
    morn_.delete_scheduled_root_nodes();
}

void Scene::delete_root_node(const std::string& name) {
    LOG_FUNCTION("Scene::delete_root_node");
    root_nodes_.delete_root_node(name);
}

void Scene::delete_root_nodes(const Mlib::regex& regex) {
    LOG_FUNCTION("Scene::delete_root_nodes");
    root_nodes_.delete_root_nodes(regex);
}

void Scene::delete_node(const std::string& name) {
    delete_node_mutex_.notify_deleting();
    SceneNode& node = get_node_that_may_be_scheduled_for_deletion(name);
    if (!node.shutting_down()) {
        unregister_node(name);
        if (!morn_.erase(name)) {
            node.parent().remove_child(name);
        }
    }
}

Scene::~Scene() {
    std::lock_guard lock{ delete_node_mutex_ };
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
        throw std::runtime_error("Scene::shutdown: delete node mutex is not locked");
    }
    if (!nodes_not_allowed_to_be_unregistered_.empty()) {
        throw std::runtime_error("Scene::shutdown: some nodes are not allowed to be deleted");
    }
    shutting_down_ = true;
    aggregation_bg_worker_.shutdown();
    instances_bg_worker_.shutdown();
    morn_.clear();
    if (!nodes_.empty()) {
        throw std::runtime_error("Registered nodes remain after shutdown");
    }
}

bool Scene::contains_node(const std::string& name) const {
    return nodes_.contains(name);
}

void Scene::register_node(
    const std::string& name,
    SceneNode& scene_node)
{
    if (name.empty()) {
        throw std::runtime_error("register_node received empty name");
    }
    if (!nodes_.insert({ name, &scene_node }).second) {
        throw std::runtime_error("Scene node with name \"" + name + "\" already exists");
    }
}

void Scene::unregister_node(const std::string& name) {
    delete_node_mutex_.assert_this_thread_is_deleter_thread();
    if (nodes_not_allowed_to_be_unregistered_.contains(name)) {
        throw std::runtime_error("Node \"" + name + "\" may not be unregistered");
    }
    if (!delete_node_mutex_.is_locked_by_this_thread()) {
        throw std::runtime_error("Scene::unregister_node: delete node mutex is not locked");
    }
    if (nodes_.erase(name) != 1) {
        throw std::runtime_error("Could not find node with name (0) \"" + name + '"');
    }
}

void Scene::unregister_nodes(const Mlib::regex& regex) {
    delete_node_mutex_.assert_this_thread_is_deleter_thread();
    if (!delete_node_mutex_.is_locked_by_this_thread()) {
        throw std::runtime_error("Scene::unregister_nodes: delete node mutex is not locked");
    }
    for (auto it = nodes_.begin(); it != nodes_.end(); ) {
        auto n = *it++;
        if (Mlib::re::regex_match(n.first, regex)) {
            if (nodes_not_allowed_to_be_unregistered_.contains(n.first)) {
                throw std::runtime_error("Node \"" + n.first + "\" may not be unregistered");
            }
            if (nodes_.erase(n.first) != 1) {
                throw std::runtime_error("Could not find node with name (1) \"" + n.first + '"');
            }
        }
    }
}

SceneNode& Scene::get_node(const std::string& name) const {
    if (morn_.root_node_scheduled_for_deletion(name, false)) {
        throw std::runtime_error("Node \"" + name + "\" is scheduled for deletion");
    }
    return get_node_that_may_be_scheduled_for_deletion(name);
}

SceneNode& Scene::get_node_that_may_be_scheduled_for_deletion(const std::string& name) const {
    auto it = nodes_.find(name);
    if (it == nodes_.end()) {
        throw std::runtime_error("Could not find node with name (2) \"" + name + '"');
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
    LOG_FUNCTION("Scene::render");
    if (!delete_node_mutex_.is_locked_by_this_thread()) {
        throw std::runtime_error("Scene::render: delete node mutex is not locked");
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
            throw std::runtime_error("Could not find black node with name \"" + external_render_pass.black_node_name + '"');
        }
        it->second->render(vp, TransformationMatrix<float, double, 3>::identity(), iv, camera_node, lights, blended, render_config, scene_graph_config, external_render_pass, nullptr, color_styles);
    } else {
        if (!external_render_pass.black_node_name.empty()) {
            throw std::runtime_error("Expected empty black node");
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
        LOG_INFO("Scene::render non-blended");
        for (const auto& [_, node] : root_nodes_) {
            node->render(vp, TransformationMatrix<float, double, 3>::identity(), iv, camera_node, lights, blended, render_config, scene_graph_config, external_render_pass, nullptr, color_styles);
        }
        for (const auto& [_, node] : static_root_nodes_) {
            node->render(vp, TransformationMatrix<float, double, 3>::identity(), iv, camera_node, lights, blended, render_config, scene_graph_config, external_render_pass, nullptr, color_styles);
        }
        LOG_INFO("Scene::render large_aggregate_renderer");
        if (large_aggregate_renderer_ != nullptr) {
            if (!large_aggregate_renderer_initialized_) {
                std::list<std::shared_ptr<ColoredVertexArray<float>>> aggregate_queue;
                for (const auto& [_, node] : static_root_nodes_) {
                    node->append_large_aggregates_to_queue(TransformationMatrix<float, double, 3>::identity(), iv.t(), aggregate_queue, scene_graph_config);
                }
                for (const auto& [_, node] : root_aggregate_nodes_) {
                    node->append_large_aggregates_to_queue(TransformationMatrix<float, double, 3>::identity(), iv.t(), aggregate_queue, scene_graph_config);
                }
                large_aggregate_renderer_->update_aggregates(iv.t(), aggregate_queue);
                large_aggregate_renderer_initialized_ = true;
            }
            large_aggregate_renderer_->render_aggregates(vp, iv, lights, scene_graph_config, render_config, external_render_pass);
        }
        LOG_INFO("Scene::render large_instances_renderer");
        if (large_instances_renderer_ != nullptr) {
            if (!large_instances_renderer_initialized_) {
                std::list<TransformedColoredVertexArray> instances_queue;
                for (const auto& [_, node] : static_root_nodes_) {
                    node->append_large_instances_to_queue(TransformationMatrix<float, double, 3>::identity(), iv.t(), PositionAndYAngle{fixed_zeros<double, 3>(), 0.f, UINT32_MAX}, instances_queue, scene_graph_config);
                }
                for (const auto& [_, node] : root_aggregate_nodes_) {
                    node->append_large_instances_to_queue(TransformationMatrix<float, double, 3>::identity(), iv.t(), PositionAndYAngle{fixed_zeros<double, 3>(), 0.f, UINT32_MAX}, instances_queue, scene_graph_config);
                }
                large_instances_renderer_->update_instances(iv.t(), instances_queue);
                large_instances_renderer_initialized_ = true;
            }
            large_instances_renderer_->render_instances(vp, iv, lights, scene_graph_config, render_config, external_render_pass);
        }
        {
            bool is_foreground_task = ((external_render_pass.pass == ExternalRenderPassType::LIGHTMAP_GLOBAL_STATIC) ||
                                       (external_render_pass.pass == ExternalRenderPassType::LIGHTMAP_BLACK_GLOBAL_STATIC) ||
                                       (external_render_pass.pass == ExternalRenderPassType::DIRTMAP));
            bool is_background_task = (external_render_pass.pass == ExternalRenderPassType::STANDARD);
            bool is_render_task = (external_render_pass.pass != ExternalRenderPassType::LIGHTMAP_BLACK_LOCAL_INSTANCES);
            bool is_black_task = (external_render_pass.pass == ExternalRenderPassType::LIGHTMAP_BLACK_LOCAL_INSTANCES);
            if (is_foreground_task && is_background_task) {
                throw std::runtime_error("Scene::render has both foreground and background task");
            }
            std::shared_ptr<AggregateRenderer> small_sorted_aggregate_renderer = AggregateRenderer::small_sorted_aggregate_renderer();
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
                if (is_foreground_task) {
                    small_sorted_aggregate_renderer_update_func()();
                } else if (is_background_task && aggregation_bg_worker_.done()) {
                    WorkerStatus status = aggregation_bg_worker_.tick(scene_graph_config.aggregate_update_interval);
                    if (status == WorkerStatus::IDLE) {
                        aggregation_bg_worker_.run(small_sorted_aggregate_renderer_update_func());
                    }
                }
                small_sorted_aggregate_renderer->render_aggregates(vp, iv, lights, scene_graph_config, render_config, external_render_pass);
            }

            // Contains continuous alpha and must therefore be rendered late.
            LOG_INFO("Scene::render instances_renderer");
            std::shared_ptr<InstancesRenderer> small_instances_renderer = InstancesRenderer::small_instances_renderer();
            std::shared_ptr<InstancesRenderer> black_small_instances_renderer = InstancesRenderer::black_small_instances_renderer();
            if (small_instances_renderer != nullptr) {
                auto small_instances_renderer_update_func = [&](){
                    // copy "vp" and "scene_graph_config"
                    return run_in_background([this, vp, iv, scene_graph_config, external_render_pass, small_instances_renderer, black_small_instances_renderer](){
                        std::list<std::pair<float, TransformedColoredVertexArray>> instances_queue;
                        for (const auto& [_, node] : static_root_nodes_) {
                            node->append_small_instances_to_queue(vp, TransformationMatrix<float, double, 3>::identity(), iv.t(), PositionAndYAngle{fixed_zeros<double, 3>(), 0.f, UINT32_MAX}, instances_queue, scene_graph_config, external_render_pass);
                        }
                        for (const auto& [_, node] : root_instances_nodes_) {
                            node->append_small_instances_to_queue(vp, TransformationMatrix<float, double, 3>::identity(), iv.t(), PositionAndYAngle{fixed_zeros<double, 3>(), 0.f, UINT32_MAX}, instances_queue, scene_graph_config, external_render_pass);
                        }
                        instances_queue.sort([](auto& a, auto& b){ return a.first < b.first; });
                        std::list<TransformedColoredVertexArray> sorted_instances_queue;
                        std::list<TransformedColoredVertexArray> black_sorted_instances_queue;
                        for (auto& [_, e] : instances_queue) {
                            sorted_instances_queue.push_back(e);
                            if (e.is_black) {
                                black_sorted_instances_queue.push_back(e);
                            }
                        }
                        small_instances_renderer->update_instances(iv.t(), sorted_instances_queue);
                        black_small_instances_renderer->update_instances(iv.t(), black_sorted_instances_queue);
                    });
                };
                if (is_foreground_task) {
                    small_instances_renderer_update_func()();
                } else if (is_background_task && instances_bg_worker_.done()) {
                    WorkerStatus status = instances_bg_worker_.tick(scene_graph_config.aggregate_update_interval);
                    if (status == WorkerStatus::IDLE) {
                        instances_bg_worker_.run(small_instances_renderer_update_func());
                    }
                }
                if (is_render_task) {
                    small_instances_renderer->render_instances(vp, iv, lights, scene_graph_config, render_config, external_render_pass);
                }
                if (is_black_task) {
                    black_small_instances_renderer->render_instances(vp, iv, lights, scene_graph_config, render_config, external_render_pass);
                }
            }
        }
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
    delete_node_mutex_.assert_this_thread_is_deleter_thread();
    if (!morn_.no_root_nodes_scheduled_for_deletion()) {
        throw std::runtime_error("Moving with root nodes scheduled for deletion");
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
}

size_t Scene::get_uuid() {
    std::lock_guard guard{uuid_mutex_};
    return uuid_++;
}

void Scene::print(std::ostream& ostr) const {
    ostr << "Scene\n";
    ostr << morn_;
}

bool Scene::shutting_down() const {
    return shutting_down_;
}

void Scene::add_node_not_allowed_to_be_unregistered(const std::string& name) {
    delete_node_mutex_.assert_this_thread_is_deleter_thread();
    if (!contains_node(name)) {
        throw std::runtime_error("Could not find node with name (3) \"" + name + '"');
    }
    if (!nodes_not_allowed_to_be_unregistered_.insert(name).second) {
        throw std::runtime_error("Node \"" + name + "\" already in list of nodes not allowed to be unregistered");
    }
}

void Scene::remove_node_not_allowed_to_be_unregistered(const std::string& name) {
    delete_node_mutex_.assert_this_thread_is_deleter_thread();
    if (!contains_node(name)) {
        throw std::runtime_error("Could not find node with name (4) \"" + name + '"');
    }
    if (nodes_not_allowed_to_be_unregistered_.erase(name) != 1) {
        throw std::runtime_error("Could not find node \"" + name + "\" in list of nodes not allowed to be unregistered");
    }
}

void Scene::clear_nodes_not_allowed_to_be_unregistered() {
    delete_node_mutex_.assert_this_thread_is_deleter_thread();
    nodes_not_allowed_to_be_unregistered_.clear();
}

void Scene::add_color_style(std::unique_ptr<ColorStyle>&& color_style) {
    color_styles_.push_back(std::move(color_style));
}

std::ostream& Mlib::operator << (std::ostream& ostr, const Scene& scene) {
    scene.print(ostr);
    return ostr;
}
