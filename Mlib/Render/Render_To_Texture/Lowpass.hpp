#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Render/Instance_Handles/Render_Program.hpp>
#include <Mlib/Render/Render_Logics/Generic_Post_Processing_Logic.hpp>
#include <memory>

namespace Mlib {

class FrameBuffer;

struct LowpassFilterRenderProgram: public RenderProgram {
	GLint texture_color_location = -1;
	GLint lowpass_offset_location = -1;
};

class Lowpass: public GenericPostProcessingLogic {
public:
	Lowpass();
	~Lowpass();
	void render(
		int width,
		int height,
		const FixedArray<uint32_t, 2>& niterations,
		std::shared_ptr<FrameBuffer> fbs[2],
		size_t& target_id);
private:
	UFixedArray<LowpassFilterRenderProgram, 2> rp_;
};

}