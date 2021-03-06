#include "Heterogeneous_Resource_Instantiator.hpp"
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource.hpp>
#include <Mlib/Scene_Graph/Batch_Resource_Instantiator.hpp>
#include <Mlib/Scene_Graph/Parsed_Resource_Name.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>

using namespace Mlib;

HeterogeneousResourceInstantiator::HeterogeneousResourceInstantiator(
    const SceneNodeResources& scene_node_resources)
: bri{ std::make_unique<BatchResourceInstantiator>() },
  acvas{ std::make_shared<AnimatedColoredVertexArrays>() },
  scene_node_resources_{ scene_node_resources }
{}

HeterogeneousResourceInstantiator::~HeterogeneousResourceInstantiator()
{}

void HeterogeneousResourceInstantiator::instantiate_renderable(
    const InstantiationOptions& options,
    const FixedArray<float, 3>& rotation,
    float scale) const
{
    bri->instantiate_renderables(
        scene_node_resources_,
        options,
        rotation,
        scale);

    if (rcva_ == nullptr) {
        std::lock_guard lock{ rcva_mutex_ };
        if (rcva_ == nullptr) {
            rcva_ = std::make_shared<ColoredVertexArrayResource>(acvas);
        }
    }
    rcva_->instantiate_renderable(options);
}

std::shared_ptr<AnimatedColoredVertexArrays> HeterogeneousResourceInstantiator::get_animated_arrays(
    float scale) const
{
    if (acvas_ == nullptr) {
        std::lock_guard lock{ acvas_mutex_ };
        if (acvas_ == nullptr) {
            // Start with "normal" arrays.
            auto res = std::make_shared<AnimatedColoredVertexArrays>(*acvas);
            // Append hitboxes.
            bri->instantiate_hitboxes(res->dcvas, scene_node_resources_, scale);
            acvas_ = res;
        }
    }
    return acvas_;
}

void HeterogeneousResourceInstantiator::generate_instances() {
    static const DECLARE_REGEX(re, "^(\\w+)(?:\\.(\\d+))?\\.billboard(?:\\b|_)");
    acvas->scvas.remove_if([this](const std::shared_ptr<ColoredVertexArray<float>>& cva){
        Mlib::re::smatch match;
        if (Mlib::re::regex_search(cva->name, match, re)) {
            if (cva->triangles.empty()) {
                throw std::runtime_error("Mesh \"" + cva->name + "\" is empty");
            }
            const auto& tri = *cva->triangles.begin();
            bri->add_parsed_resource_name(
                tri(0).position.casted<double>(),
                ParsedResourceName{
                    .name = match[1].str(),
                    .billboard_id = match[2].matched ? safe_stou(match[2].str()) : UINT32_MAX,
                    .probability = NAN,
                    .aggregate_mode = scene_node_resources_.aggregate_mode(match[1].str()),
                    .hitbox = ""},
                0.f,   // yangle
                1.f);  // scale
            return true;
        } else {
            return false;
        }
    });
    for (const auto& cva : acvas->dcvas) {
        if (Mlib::re::regex_search(cva->name, re)) {
            throw std::runtime_error("Instances require single precision meshes");
        }
    }
}
