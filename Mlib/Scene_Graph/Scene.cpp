#include "Scene.hpp"
#include <Mlib/Geometry/Homogeneous.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Transformed_Colored_Vertex_Array.hpp>
#include <Mlib/Log.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Scene_Graph/Aggregate_Renderer.hpp>
#include <Mlib/Scene_Graph/Instances_Renderer.hpp>
#include <Mlib/Scene_Graph/Scene_Graph_Config.hpp>
#include <Mlib/Scene_Graph/Style.hpp>

using namespace Mlib;

Scene::Scene(
    AggregateRenderer* large_aggregate_renderer,
    InstancesRenderer* large_instances_renderer)
: large_aggregate_renderer_{large_aggregate_renderer},
  large_instances_renderer_{large_instances_renderer},
  large_aggregate_renderer_initialized_{false},
  large_instances_renderer_initialized_{false},
  uuid_{0},
  shutting_down_{false}
{}

void Scene::add_root_node(
    const std::string& name,
    SceneNode* scene_node)
{
    LOG_FUNCTION("Scene::add_root_node");
    if (root_nodes_to_delete_.contains(name)) {
        throw std::runtime_error("Node \"" + name + "\" is scheduled for deletion");
    }
    register_node(name, scene_node);
    if (!root_nodes_.insert(std::make_pair(name, scene_node)).second) {
        throw std::runtime_error("add_root_node could not insert node");
    };
}

void Scene::add_static_root_node(
    const std::string& name,
    SceneNode* scene_node)
{
    register_node(name, scene_node);
    if (!static_root_nodes_.insert(std::make_pair(name, scene_node)).second) {
        throw std::runtime_error("add_static_root_node could not insert node");
    }
}

void Scene::add_root_aggregate_node(
    const std::string& name,
    SceneNode* scene_node)
{
    register_node(name, scene_node);
    if (!root_aggregate_nodes_.insert(std::make_pair(name, scene_node)).second) {
        throw std::runtime_error("add_root_aggregate_node could not insert node");
    }
}

void Scene::add_root_instances_node(
    const std::string& name,
    SceneNode* scene_node)
{
    register_node(name, scene_node);
    if (!root_instances_nodes_.insert(std::make_pair(name, scene_node)).second) {
        throw std::runtime_error("add_root_instances_node could not insert node");
    }
}

bool Scene::root_node_scheduled_for_deletion(const std::string& name) const {
    std::lock_guard lock{ root_nodes_to_delete_mutex_ };
    if (root_nodes_.find(name) == root_nodes_.end()) {
        throw std::runtime_error("No root node with name \"" + name + "\" exists");
    }
    return root_nodes_to_delete_.contains(name);
}

void Scene::schedule_delete_root_node(const std::string& name) {
    std::lock_guard lock{ root_nodes_to_delete_mutex_ };
    if (root_nodes_.find(name) == root_nodes_.end()) {
        throw std::runtime_error("No root node with name \"" + name + "\" exists");
    }
    if (!root_nodes_to_delete_.insert(name).second) {
        throw std::runtime_error("Node \"" + name + "\" is already scheduled for deletion");
    }
}

void Scene::delete_scheduled_root_nodes() const {
    auto self = const_cast<Scene*>(this);
    std::lock_guard lock{ self->root_nodes_to_delete_mutex_ };
    while (!root_nodes_to_delete_.empty()) {
        // Temporary name variable to allow recursive calls to delete_root_node.
        auto name = *self->root_nodes_to_delete_.begin();
        self->root_nodes_to_delete_.erase(name);
        self->delete_root_node(name);
    }
}

void Scene::delete_root_node(const std::string& name) {
    LOG_FUNCTION("Scene::delete_root_node");
    // Temporary unique_ptr to allow recursive calls to delete_root_node.
    std::unique_ptr<SceneNode> to_delete;
    auto it = root_nodes_.find(name);
    if (it == root_nodes_.end()) {
        throw std::runtime_error("Could not find root node with name " + name);
    }
    to_delete = std::move(it->second);
    root_nodes_.erase(it);
    unregister_node(name);
}

void Scene::delete_root_nodes(const Mlib::regex& regex) {
    LOG_FUNCTION("Scene::delete_root_nodes");
    for (auto it = root_nodes_.begin(); it != root_nodes_.end(); ) {
        auto n = it++;
        if (Mlib::re::regex_match(n->first, regex)) {
            root_nodes_.erase(n->first);
        }
    }
    unregister_nodes(regex);
}

Scene::~Scene() {
    shutdown();
}

void Scene::shutdown() {
    shutting_down_ = true;
    root_instances_nodes_.clear();
    root_aggregate_nodes_.clear();
    static_root_nodes_.clear();
    root_nodes_.clear();
    root_nodes_to_delete_.clear();
    nodes_.clear();
}

bool Scene::contains_node(const std::string& name) const {
    return nodes_.contains(name);
}

void Scene::register_node(
    const std::string& name,
    SceneNode* scene_node)
{
    if (name.empty()) {
        throw std::runtime_error("register_node received empty name");
    }
    if (!nodes_.insert(std::make_pair(name, scene_node)).second) {
        throw std::runtime_error("Scene node with name \"" + name + "\" already exists");
    }
}

void Scene::unregister_node(const std::string& name) {
    if (nodes_.erase(name) != 1) {
        throw std::runtime_error("Could not find node with name \"" + name + '"');
    }
}

void Scene::unregister_nodes(const Mlib::regex& regex) {
    for (auto it = nodes_.begin(); it != nodes_.end(); ) {
        auto n = *it++;
        if (Mlib::re::regex_match(n.first, regex)) {
            if (nodes_.erase(n.first) != 1) {
                throw std::runtime_error("Could not find node with name \"" + n.first + '"');
            }
        }
    }
}

SceneNode* Scene::get_node(const std::string& name) const {
    if (root_nodes_to_delete_.contains(name)) {
        throw std::runtime_error("Node \"" + name + "\" is scheduled for deletion");
    }
    auto it = nodes_.find(name);
    if (it == nodes_.end()) {
        throw std::runtime_error("Could not find node with name \"" + name + '"');
    }
    return it->second;
}

void Scene::render(
    const FixedArray<float, 4, 4>& vp,
    const TransformationMatrix<float, 3>& iv,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    const ExternalRenderPass& external_render_pass,
    const std::function<std::function<void()>(std::function<void()>)>& run_in_background) const
{
    LOG_FUNCTION("Scene::render");
    std::list<std::pair<TransformationMatrix<float, 3>, Light*>> lights;
    std::list<Blended> blended;
    if ((external_render_pass.pass == ExternalRenderPassType::LIGHTMAP_TO_TEXTURE) && !external_render_pass.black_node_name.empty()) {
        auto it = root_nodes_.find(external_render_pass.black_node_name);
        if (it == root_nodes_.end()) {
            throw std::runtime_error("Could not find black node with name \"" + external_render_pass.black_node_name + '"');
        }
        it->second->render(vp, TransformationMatrix<float, 3>::identity(), iv, lights, blended, render_config, scene_graph_config, external_render_pass, style_.get());
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
        for (const auto& n : root_nodes_) {
            n.second->append_lights_to_queue(TransformationMatrix<float, 3>::identity(), lights);
        }
        for (const auto& n : static_root_nodes_) {
            n.second->append_lights_to_queue(TransformationMatrix<float, 3>::identity(), lights);
        }
        LOG_INFO("Scene::render non-blended");
        for (const auto& n : root_nodes_) {
            n.second->render(vp, TransformationMatrix<float, 3>::identity(), iv, lights, blended, render_config, scene_graph_config, external_render_pass, style_.get());
        }
        for (const auto& n : static_root_nodes_) {
            n.second->render(vp, TransformationMatrix<float, 3>::identity(), iv, lights, blended, render_config, scene_graph_config, external_render_pass, style_.get());
        }
        LOG_INFO("Scene::render large_aggregate_renderer");
        if (large_aggregate_renderer_ != nullptr) {
            if (!large_aggregate_renderer_initialized_) {
                std::list<std::shared_ptr<ColoredVertexArray>> aggregate_queue;
                for (const auto& n : static_root_nodes_) {
                    n.second->append_large_aggregates_to_queue(TransformationMatrix<float, 3>::identity(), aggregate_queue, scene_graph_config);
                }
                for (const auto& n : root_aggregate_nodes_) {
                    n.second->append_large_aggregates_to_queue(TransformationMatrix<float, 3>::identity(), aggregate_queue, scene_graph_config);
                }
                large_aggregate_renderer_->update_aggregates(aggregate_queue);
                large_aggregate_renderer_initialized_ = true;
            }
            large_aggregate_renderer_->render_aggregates(vp, iv, lights, scene_graph_config, render_config, external_render_pass);
        }
        LOG_INFO("Scene::render large_instances_renderer");
        if (large_instances_renderer_ != nullptr) {
            if (!large_instances_renderer_initialized_) {
                std::list<TransformedColoredVertexArray> instances_queue;
                for (const auto& n : static_root_nodes_) {
                    n.second->append_large_instances_to_queue(TransformationMatrix<float, 3>::identity(), fixed_zeros<float, 3>(), instances_queue, scene_graph_config);
                }
                for (const auto& n : root_aggregate_nodes_) {
                    n.second->append_large_instances_to_queue(TransformationMatrix<float, 3>::identity(), fixed_zeros<float, 3>(), instances_queue, scene_graph_config);
                }
                large_instances_renderer_->update_instances(instances_queue);
                large_instances_renderer_initialized_ = true;
            }
            large_instances_renderer_->render_instances(vp, iv, lights, scene_graph_config, render_config, external_render_pass);
        }
        {
            bool is_foreground_task =
                (external_render_pass.pass == ExternalRenderPassType::LIGHTMAP_TO_TEXTURE) ||
                (external_render_pass.pass == ExternalRenderPassType::DIRTMAP);
            std::shared_ptr<AggregateRenderer> small_sorted_aggregate_renderer = AggregateRenderer::small_sorted_aggregate_renderer();
            if (small_sorted_aggregate_renderer != nullptr) {
                // Contains continuous alpha and must therefore be rendered late.
                LOG_INFO("Scene::render small_sorted_aggregate_renderer");
                auto small_sorted_aggregate_renderer_update_func = [&](){
                    // copy "vp" and "scene_graph_config"
                    return run_in_background([this, vp, scene_graph_config, external_render_pass, small_sorted_aggregate_renderer](){
                        std::list<std::pair<float, std::shared_ptr<ColoredVertexArray>>> aggregate_queue;
                        for (const auto& n : static_root_nodes_) {
                            n.second->append_sorted_aggregates_to_queue(vp, TransformationMatrix<float, 3>::identity(), aggregate_queue, scene_graph_config, external_render_pass);
                        }
                        for (const auto& n : root_aggregate_nodes_) {
                            n.second->append_sorted_aggregates_to_queue(vp, TransformationMatrix<float, 3>::identity(), aggregate_queue, scene_graph_config, external_render_pass);
                        }
                        aggregate_queue.sort([](auto& a, auto& b){ return a.first < b.first; });
                        std::list<std::shared_ptr<ColoredVertexArray>> sorted_aggregate_queue;
                        for (auto& e : aggregate_queue) {
                            sorted_aggregate_queue.push_back(std::move(e.second));
                        }
                        small_sorted_aggregate_renderer->update_aggregates(sorted_aggregate_queue);
                    });
                };
                if (is_foreground_task) {
                    small_sorted_aggregate_renderer_update_func()();
                } else if (aggregation_bg_worker_.done()) {
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
            if (small_instances_renderer != nullptr) {
                auto small_instances_renderer_update_func = [&](){
                    // copy "vp" and "scene_graph_config"
                    return run_in_background([this, vp, scene_graph_config, external_render_pass, small_instances_renderer](){
                        std::list<std::pair<float, TransformedColoredVertexArray>> instances_queue;
                        for (const auto& n : static_root_nodes_) {
                            n.second->append_small_instances_to_queue(vp, TransformationMatrix<float, 3>::identity(), fixed_zeros<float, 3>(), instances_queue, scene_graph_config, external_render_pass);
                        }
                        for (const auto& n : root_instances_nodes_) {
                            n.second->append_small_instances_to_queue(vp, TransformationMatrix<float, 3>::identity(), fixed_zeros<float, 3>(), instances_queue, scene_graph_config, external_render_pass);
                        }
                        instances_queue.sort([](auto& a, auto& b){ return a.first < b.first; });
                        std::list<TransformedColoredVertexArray> sorted_instances_queue;
                        for (auto& e : instances_queue) {
                            sorted_instances_queue.push_back(std::move(e.second));
                        }
                        small_instances_renderer->update_instances(sorted_instances_queue);
                    });
                };
                if (is_foreground_task) {
                    small_instances_renderer_update_func()();
                } else if (instances_bg_worker_.done()) {
                    WorkerStatus status = instances_bg_worker_.tick(scene_graph_config.aggregate_update_interval);
                    if (status == WorkerStatus::IDLE) {
                        instances_bg_worker_.run(small_instances_renderer_update_func());
                    }
                }
                small_instances_renderer->render_instances(vp, iv, lights, scene_graph_config, render_config, external_render_pass);
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
            {external_render_pass, InternalRenderPass::BLENDED},
            style_.get());
    }
}

void Scene::move(float dt) {
    LOG_FUNCTION("Scene::move");
    if (!root_nodes_to_delete_.empty()) {
        throw std::runtime_error("Moving with root nodes scheduled for deletion");
    }
    for (const auto& n : root_nodes_) {
        n.second->move(TransformationMatrix<float, 3>::identity(), dt);
    }
}

size_t Scene::get_uuid() {
    std::lock_guard guard{uuid_mutex_};
    return uuid_++;
}

void Scene::print(std::ostream& ostr) const {
    ostr << "Dynamic nodes\n";
    for (const auto& n : root_nodes_) {
        ostr << " " << n.first << '\n';
        n.second->print(ostr, 2);
    }
    ostr << "Static nodes\n";
    for (const auto& n : static_root_nodes_) {
        ostr << " " << n.first << '\n';
        n.second->print(ostr, 2);
    }
    ostr << "Aggregate nodes\n";
    for (const auto& n : root_aggregate_nodes_) {
        ostr << " " << n.first << '\n';
        n.second->print(ostr, 2);
    }
    ostr << "Instances nodes\n";
    for (const auto& n : root_instances_nodes_) {
        ostr << " " << n.first << '\n';
        n.second->print(ostr, 2);
    }
}

bool Scene::shutting_down() const {
    return shutting_down_;
}

std::ostream& Mlib::operator << (std::ostream& ostr, const Scene& scene) {
    scene.print(ostr);
    return ostr;
}
