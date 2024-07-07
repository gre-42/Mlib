#pragma once
#include <list>

namespace Mlib {

class Scene;
struct InstanceInformation;
class SceneNodeResources;
class RenderingResources;

void instantiate(
	Scene& scene,
	const std::list<InstanceInformation>& infos,
	SceneNodeResources& scene_node_resources,
	RenderingResources& rendering_resources);

}
