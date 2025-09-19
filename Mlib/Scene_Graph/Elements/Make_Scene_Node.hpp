#pragma once
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <memory>

namespace Mlib {

class AutoShutdownSceneNode {
public:
	inline AutoShutdownSceneNode(std::unique_ptr<SceneNode>&& node)
		: node_{ std::move(node) }
	{}
	inline ~AutoShutdownSceneNode() {
		if (node_ != nullptr) {
			node_->shutdown();
		}
	}
	DanglingBaseClassPtr<SceneNode> get(SourceLocation loc) const {
		return { node_.get(), loc };
	}
	DanglingBaseClassRef<SceneNode> ref(SourceLocation loc) const {
		return { *node_.get(), loc };
	}
	SceneNode* operator -> () {
		return node_.operator->();
	}
	const SceneNode* operator -> () const {
		return node_.operator->();
	}
	operator std::unique_ptr<SceneNode>&& () {
		return std::move(node_);
	}
private:
	std::unique_ptr<SceneNode> node_;
};

template <class... Args>
AutoShutdownSceneNode make_unique_scene_node(Args&&... args) {
	return { std::make_unique<SceneNode>(std::forward<Args>(args)...) };
}

}
