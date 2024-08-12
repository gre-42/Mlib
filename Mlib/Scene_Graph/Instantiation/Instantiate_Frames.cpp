#include "Instantiate_Frames.hpp"
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Rendering_Strategies.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Instantiation/Child_Instantiation_Options.hpp>
#include <Mlib/Scene_Graph/Instantiation/Instance_Information.hpp>
#include <Mlib/Scene_Graph/Resources/Renderable_Resource_Filter.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>

using namespace Mlib;

// From: https://gtamods.com/wiki/Map_system
// X: east/west direction
// Y: north/south direction
// Z: up/down direction
static const auto r_to_world = FixedArray<float, 3, 3>::init(
	1.f, 0.f, 0.f,
	0.f, 0.f, 1.f,
	0.f, -1.f, 0.f);
static const auto t_to_world = fixed_zeros<ScenePos, 3>();
static const TransformationMatrix<float, ScenePos, 3> trafo_to_world{ r_to_world, t_to_world };

void Mlib::instantiate(
	Scene& scene,
	const std::list<InstanceInformation>& infos,
	SceneNodeResources& scene_node_resources,
	RenderingResources& rendering_resources,
	const std::set<std::string>& exclude,
	std::set<std::string>* instantiated)
{
	for (const auto& info : infos) {
		if (exclude.contains(info.resource_name)) {
			continue;
		}
		auto name = info.resource_name + "_inst_" + std::to_string(scene.get_uuid());
		auto trafo = trafo_to_world * info.trafo;
		auto node = make_dunique<SceneNode>(
			trafo.t(),
			matrix_2_tait_bryan_angles(trafo.R()),
			info.scale);
		auto resource_name = info.resource_name + ".dff";
		scene_node_resources.instantiate_child_renderable(
			resource_name,
			ChildInstantiationOptions{
				.rendering_resources = &rendering_resources,
				.instance_name = name,
				.scene_node = node.ref(DP_LOC),
				.renderable_resource_filter = RenderableResourceFilter{}},
			PreloadBehavior::NO_PRELOAD);
		if (!any(node->rendering_strategies())) {
			lwarn() << "Skipping invisible instance \"" << name << '"';
		} else {
			scene.auto_add_root_node(name, std::move(node), info.rendering_dynamics);
			if (instantiated != nullptr) {
				instantiated->insert(resource_name);
			}
		}
	}
}
