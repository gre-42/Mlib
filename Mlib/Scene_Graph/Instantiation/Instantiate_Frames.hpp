#pragma once
#include <list>
#include <set>

namespace Mlib {

class Scene;
struct InstanceInformation;
class SceneNodeResources;
class RenderingResources;

void instantiate(
	Scene& scene,
	const std::list<InstanceInformation>& infos,
	SceneNodeResources& scene_node_resources,
	RenderingResources& rendering_resources,
	const std::set<std::string>& exclude,
	std::set<std::string>* instantiated);

}
