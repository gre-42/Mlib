#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Map/Verbose_Unordered_Map.hpp>
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <Mlib/Memory/Destruction_Functions.hpp>
#include <Mlib/Scene_Precision.hpp>
#include <Mlib/Signal/Exponential_Smoother.hpp>
#include <Mlib/Threads/Fast_Mutex.hpp>
#include <chrono>
#include <iosfwd>
#include <optional>
#include <unordered_map>

namespace Mlib {

class AudioSource;
template <class TPosition>
struct AudioSourceState;
class SceneNode;

struct AudioSourceNode {
	ExponentialSmoother<FixedArray<float, 3>, float> relative_position;
	ExponentialSmoother<FixedArray<float, 3>, float> relative_velocity;
};

class AudioScene {
	AudioScene() = delete;
	AudioScene(const AudioScene&) = delete;
	AudioScene &operator=(const AudioScene&) = delete;
public:
	static void set_default_alpha(float alpha);
	static void add_source(AudioSource& source, float alpha);
	static void remove_source(AudioSource& source);
	static void set_listener(
		const DanglingRef<SceneNode>& node,
		std::chrono::steady_clock::time_point time,
		std::chrono::steady_clock::duration velocity_dt);
	static void set_source_transformation(
		AudioSource& source,
		const AudioSourceState<ScenePos>& state);
	static void print(std::ostream& ostr);
private:
	static FastMutex mutex_;
	static float default_alpha_;
	static VerboseUnorderedMap<const AudioSource*, AudioSourceNode> source_nodes_;
	static DanglingPtr<SceneNode> listener_node_;
	static std::optional<DestructionFunctionsRemovalTokens> on_destroy_;
};

}
