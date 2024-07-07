#include "Instantiate_Frames.hpp"
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Instantiation/Instance_Information.hpp>
#include <Mlib/Scene_Graph/Instantiation_Options.hpp>
#include <Mlib/Scene_Graph/Resources/Renderable_Resource_Filter.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>

using namespace Mlib;

void Mlib::instantiate(
	Scene& scene,
	const std::list<InstanceInformation>& infos,
	SceneNodeResources& scene_node_resources,
	RenderingResources& rendering_resources,
	const std::set<std::string>& exclude)
{
	for (const auto& info : infos) {
		if (exclude.contains(info.resource_name)) {
			continue;
		}
		auto name = info.resource_name + "_inst_" + std::to_string(scene.get_uuid());
		auto node = make_dunique<SceneNode>(
			info.trafo.t(),
			matrix_2_tait_bryan_angles(info.trafo.R()),
			1.f);
		scene_node_resources.instantiate_renderable(
			info.resource_name + ".dff",
			InstantiationOptions{
				.rendering_resources = &rendering_resources,
				.imposters = nullptr,
				.supply_depots = nullptr,
				.instance_name = name,
				.scene_node = node.ref(DP_LOC),
				.renderable_resource_filter = RenderableResourceFilter{}},
			PreloadBehavior::NO_PRELOAD);
		scene.add_static_root_node(name, std::move(node));
	}
}
