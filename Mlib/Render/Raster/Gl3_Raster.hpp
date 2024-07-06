#pragma once
#include <Mlib/Geometry/Mesh/Load/IRaster.hpp>
#include <Mlib/Render/Any_Gl.hpp>

namespace Mlib {

namespace Dff {

struct MipLevelMetadata;

// used to emulate d3d and xbox textures
struct RasterLevels
{
	uint32_t format;
	struct Level {
		uint32_t width;
		uint32_t height;
		std::vector<uint8_t> data;
	};
	std::vector<Level> levels;
};

struct RasterConfig;

struct Image;

class Gl3Raster : public IRaster {
public:
	Gl3Raster(
		uint32_t width,
		uint32_t height,
		uint32_t depth,
		uint32_t format,
		uint32_t compression,
		uint32_t num_levels,
		bool has_alpha,
		const RasterConfig& cfg);
	virtual ~Gl3Raster() override;
	virtual void from_image(const Image& image) override;
	virtual const MipLevelMetadata& mip_level_meta_data(uint32_t level) const override;
	virtual uint32_t num_levels() const override;
	virtual uint8_t* lock(uint32_t level, uint32_t lock_mode) override;
	virtual void unlock() override;
private:
	void compute_mip_level_metadata();
	void allocate_dxt(const RasterConfig& cfg);
	uint32_t type() const;
	uint32_t flags() const;
	uint32_t format() const;
	uint32_t format_;
	uint32_t compression_;
	uint32_t num_levels_;
	bool has_alpha_;
	uint32_t private_flags_;
	uint32_t width_;
	uint32_t height_;
	uint32_t depth_;
	uint32_t stride_;
	uint32_t native_type_;
	bool native_has_alpha;
	uint32_t native_bpp_;
	uint32_t native_depth_;
	bool native_is_compressed_;
	uint32_t native_num_levels_;
	bool native_autogen_mipmap_;
	GLuint native_texture_id_;
	GLenum native_format_;
	uint32_t native_internal_format_;
	uint32_t custom_format_;
	std::vector<MipLevelMetadata> mip_levels_;
	RasterLevels native_backing_store_;
	std::vector<uint8_t> pixels_;
	std::optional<uint32_t> locked_level_;
};

}

}
