#include "Raster_Factory.hpp"
#include <Mlib/Geometry/Mesh/Load/Load_Dff.hpp>
#include <Mlib/Geometry/Mesh/Load/Raster_Config.hpp>
#include <Mlib/Render/Raster/D3d8_Raster.hpp>
#include <Mlib/Render/Raster/Gl3_Raster.hpp>

using namespace Mlib::Dff;

RasterFactory::RasterFactory(const RasterConfig& raster_config)
	: raster_config_{ raster_config }
{}

bool RasterFactory::is_p8_supported() const {
	return false;
}

std::unique_ptr<IRaster> RasterFactory::create_raster(const Image& img, uint32_t type) const
{
	// Almost the same as d3d9 and ps2 function
	int32_t width, height, depth, format;

	if ((type & 0xF) != Raster::TEXTURE) {
		THROW_OR_ABORT("Invalid raster type");
	}

	//	for(width = 1; width < img->width; width <<= 1);
	//	for(height = 1; height < img->height; height <<= 1);
	// Perhaps non-power-of-2 textures are acceptable?
	width = img.width;
	height = img.height;

	depth = img.depth;

	if (depth <= 8)
		depth = 32;

	switch (depth) {
	case 32:
		if (img.has_alpha())
			format = Raster::C8888;
		else{
			format = Raster::C888;
			depth = 24;
		}
		break;
	case 24:
		format = Raster::C888;
		break;
	case 16:
		format = Raster::C1555;
		break;

	case 8:
	case 4:
	default:
		THROW_OR_ABORT("Invalid raster depth");
	}

	format |= type;

	return std::make_unique<Gl3Raster>(
		width,
		height,
		depth,
		format,
		0, // compression
		1, // num_levels
		img.has_alpha(),
		raster_config_);
}

std::unique_ptr<IRaster> RasterFactory::create_raster(
	uint32_t width,
	uint32_t height,
	uint32_t depth,
	uint32_t format,
	uint32_t platform,
	uint32_t compression,
	uint32_t num_levels,
	bool has_alpha,
	const uint8_t* palette) const
{
	auto raster = [&]() -> std::unique_ptr<IRaster> {
		switch (platform) {
		case FOURCC_PS2:
			THROW_OR_ABORT("FOURCC_PS2 texture not yet implemented");
		case PLATFORM_D3D8:
			return std::make_unique<D3d8Raster>(width, height, depth, format, compression, num_levels, palette, has_alpha, raster_config_);
		case PLATFORM_D3D9:
			THROW_OR_ABORT("PLATFORM_D3D9 texture not yet implemented");
		case PLATFORM_XBOX:
			THROW_OR_ABORT("PLATFORM_XBOX texture not yet implemented");
		case PLATFORM_GL3:
			return std::make_unique<Gl3Raster>(width, height, depth, format, compression, num_levels, has_alpha, raster_config_);
		};
		THROW_OR_ABORT("Unknown platform");
		}();
	if (raster_config_.make_native) {
		return make_raster_native(std::move(raster));
	}
	return raster;
}

std::unique_ptr<IRaster> RasterFactory::make_raster_native(std::unique_ptr<IRaster>&& raster) const {
	if (dynamic_cast<Gl3Raster*>(raster.get()) != nullptr) {
		return raster;
	}
	auto img = raster->to_image();
	return create_raster(img, raster->type());
}
