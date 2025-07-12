#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Render/Instance_Handles/Render_Program.hpp>
#include <Mlib/Render/Render_Logics/Generic_Post_Processing_Logic.hpp>
#include <compare>
#include <memory>
#include <variant>

namespace Mlib {

class FrameBuffer;

struct LowpassFilterRenderProgram: public RenderProgram {
	GLint texture_color_location = -1;
	GLint lowpass_offset_location = -1;
};

enum class LowpassFlavor {
	NONE,
	MAX
};

struct NormalParameters {
	float stddev;
	float truncate = 3.5f;
	std::partial_ordering operator <=> (const NormalParameters&) const = default;
};

struct BoxParameters {
	std::partial_ordering operator <=> (const BoxParameters&) const = default;
};

class Lowpass: public GenericPostProcessingLogic {
public:
	using Params1 = std::variant<BoxParameters, NormalParameters>;
	using Params2 = FixedArray<std::variant<BoxParameters, NormalParameters>, 2>;
	explicit Lowpass(
		const Params2& params = {Params1{BoxParameters{}}, Params1{BoxParameters{}}},
		LowpassFlavor flavor = LowpassFlavor::NONE);
	~Lowpass();
	void render(
		int width,
		int height,
		const FixedArray<uint32_t, 2>& niterations,
		std::shared_ptr<FrameBuffer> fbs[2],
		size_t& target_id);
private:
	Params2 params_;
	LowpassFlavor flavor_;
	UFixedArray<LowpassFilterRenderProgram, 2> rp_;
};

}
