#pragma once
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>

namespace Mlib {

class AutoShutdownSceneNode {
public:
	inline AutoShutdownSceneNode(DanglingUniquePtr<SceneNode>&& node)
		: node_{ std::move(node) }
	{}
	inline ~AutoShutdownSceneNode() {
		if (node_ != nullptr) {
			node_->shutdown();
		}
	}
	DanglingPtr<SceneNode> get(SourceLocation loc) const {
		return node_.get(loc);
	}
	DanglingRef<SceneNode> ref(SourceLocation loc) const {
		return node_.ref(loc);
	}
	SceneNode* operator -> () {
		return node_.operator->();
	}
	const SceneNode* operator -> () const {
		return node_.operator->();
	}
	operator DanglingUniquePtr<SceneNode>&& () {
		return std::move(node_);
	}
private:
	DanglingUniquePtr<SceneNode> node_;
};

template <class... Args>
AutoShutdownSceneNode make_unique_scene_node(Args&&... args) {
	return { make_dunique<SceneNode>(std::forward<Args>(args)...) };
}

}
