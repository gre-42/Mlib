#pragma once
#include <list>

namespace Mlib {

class Scene;
struct InstanceInformation;
class SceneNodeResources;

void instantiate(
	Scene& scene,
	const std::list<InstanceInformation>& infos,
	SceneNodeResources& scene_node_resources);

}
