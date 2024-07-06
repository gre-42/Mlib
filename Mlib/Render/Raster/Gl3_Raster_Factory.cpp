#include "Gl3_Raster_Factory.hpp"
#include <Mlib/Geometry/Mesh/Load/Load_Dff.hpp>
#include <Mlib/Render/Raster/Gl3_Raster.hpp>

using namespace Mlib::Dff;

Gl3RasterFactory::Gl3RasterFactory(const RasterConfig& raster_config)
	: raster_config_{ raster_config }
{}

bool Gl3RasterFactory::is_p8_supported() const {
	return true;
}

std::unique_ptr<IRaster> Gl3RasterFactory::create_raster(const Image& img, uint32_t type) const
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

	if(depth <= 8)
		depth = 32;

	switch(depth){
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

std::unique_ptr<IRaster> Gl3RasterFactory::create_raster(
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
	return std::make_unique<Gl3Raster>(width, height, depth, format, compression, num_levels, has_alpha, raster_config_);
}
