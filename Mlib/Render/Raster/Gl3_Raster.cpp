#include "Gl3_Raster.hpp"
#include <Mlib/Geometry/Mesh/Load/Load_Dff.hpp>
#include <Mlib/Geometry/Mesh/Load/Mip_Level_Meta_Data.hpp>
#include <Mlib/Geometry/Mesh/Load/Raster_Config.hpp>
#include <Mlib/Render/Any_Gl.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Instance_Handles/Bind_Texture_Guard.hpp>
#include <Mlib/Render/Instance_Handles/Frame_Buffer_2D.hpp>

#define GL_COMPRESSED_RGB_S3TC_DXT1_EXT                   0x83F0
#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT                  0x83F1
#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT                  0x83F2
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT                  0x83F3

using namespace Mlib::Dff;

struct Gl3Caps
{
	int gles;
	int glversion;
	bool dxtSupported;
	bool astcSupported;	// not used yet
	float maxAnisotropy;
};
static Gl3Caps gl3Caps;

static void conv_RGBA8888_from_RGBA8888(uint8_t *out, const uint8_t *in)
{
	out[0] = in[0];
	out[1] = in[1];
	out[2] = in[2];
	out[3] = in[3];
}

static void conv_BGRA8888_from_RGBA8888(uint8_t *out, const uint8_t *in)
{
	out[2] = in[0];
	out[1] = in[1];
	out[0] = in[2];
	out[3] = in[3];
}

static void conv_RGBA8888_from_RGB888(uint8_t *out, const uint8_t *in)
{
	out[0] = in[0];
	out[1] = in[1];
	out[2] = in[2];
	out[3] = 0xFF;
}

static void conv_BGRA8888_from_RGB888(uint8_t *out, const uint8_t *in)
{
	out[2] = in[0];
	out[1] = in[1];
	out[0] = in[2];
	out[3] = 0xFF;
}

static void conv_RGB888_from_RGB888(uint8_t *out, const uint8_t *in)
{
	out[0] = in[0];
	out[1] = in[1];
	out[2] = in[2];
}

static void conv_BGR888_from_RGB888(uint8_t *out, const uint8_t *in)
{
	out[2] = in[0];
	out[1] = in[1];
	out[0] = in[2];
}

static void conv_ARGB1555_from_ARGB1555(uint8_t *out, const uint8_t *in)
{
	out[0] = in[0];
	out[1] = in[1];
}

static void conv_ARGB1555_from_RGB555(uint8_t *out, const uint8_t *in)
{
	out[0] = in[0];
	out[1] = in[1] | 0x80;
}

static void conv_RGBA5551_from_ARGB1555(uint8_t *out, const uint8_t *in)
{
	uint32_t r, g, b, a;
	a = (in[1]>>7) & 1;
	r = (in[1]>>2) & 0x1F;
	g = (in[1]&3)<<3 | ((in[0]>>5)&7);
	b = in[0] & 0x1F;
	out[0] = a | b<<1 | g<<6;
	out[1] = g>>2 | r<<3;
}

static void conv_ARGB1555_from_RGBA5551(uint8_t *out, const uint8_t *in)
{
	uint32_t r, g, b, a;
	a = in[0] & 1;
	b = (in[0]>>1) & 0x1F;
	g = (in[1]&7)<<2 | ((in[0]>>6)&3);
	r = (in[1]>>3) & 0x1F;
	out[0] = b | g<<5;
	out[1] = g>>3 | r<<2 | a<<7;
}

static void conv_RGBA8888_from_ARGB1555(uint8_t *out, const uint8_t *in)
{
	uint32_t r, g, b, a;
	a = (in[1]>>7) & 1;
	r = (in[1]>>2) & 0x1F;
	g = (in[1]&3)<<3 | ((in[0]>>5)&7);
	b = in[0] & 0x1F;
	out[0] = r*0xFF/0x1f;
	out[1] = g*0xFF/0x1f;
	out[2] = b*0xFF/0x1f;
	out[3] = a*0xFF;
}

static void conv_ABGR1555_from_ARGB1555(uint8_t *out, const uint8_t *in)
{
	uint32_t r, b;
	r = (in[1]>>2) & 0x1F;
	b = in[0] & 0x1F;
	out[1] = (in[1]&0x83) | b<<2;
	out[0] = (in[0]&0xE0) | r;
}

static void expandPal4(uint8_t *dst, uint32_t dststride, uint8_t *src, uint32_t srcstride, int32_t w, int32_t h)
{
	int32_t x, y;
	for(y = 0; y < h; y++)
		for(x = 0; x < w/2; x++){
			dst[y*dststride + x*2 + 0] = src[y*srcstride + x] & 0xF;
			dst[y*dststride + x*2 + 1] = src[y*srcstride + x] >> 4;
		}
}

static void compressPal4(uint8_t *dst, uint32_t dststride, uint8_t *src, uint32_t srcstride, int32_t w, int32_t h)
{
	int32_t x, y;
	for(y = 0; y < h; y++)
		for(x = 0; x < w/2; x++)
			dst[y*dststride + x] = src[y*srcstride + x*2 + 0] | src[y*srcstride + x*2 + 1] << 4;
}

static void expandPal4_BE(uint8_t *dst, uint32_t dststride, uint8_t *src, uint32_t srcstride, int32_t w, int32_t h)
{
	int32_t x, y;
	for(y = 0; y < h; y++)
		for(x = 0; x < w/2; x++){
			dst[y*dststride + x*2 + 1] = src[y*srcstride + x] & 0xF;
			dst[y*dststride + x*2 + 0] = src[y*srcstride + x] >> 4;
		}
}

static void compressPal4_BE(uint8_t *dst, uint32_t dststride, uint8_t *src, uint32_t srcstride, int32_t w, int32_t h)
{
	int32_t x, y;
	for(y = 0; y < h; y++)
		for(x = 0; x < w/2; x++)
			dst[y*dststride + x] = src[y*srcstride + x*2 + 1] | src[y*srcstride + x*2 + 0] << 4;
}

static void copyPal8(uint8_t *dst, uint32_t dststride, uint8_t *src, uint32_t srcstride, int32_t w, int32_t h)
{
	int32_t x, y;
	for(y = 0; y < h; y++)
		for(x = 0; x < w; x++)
			dst[y*dststride + x] = src[y*srcstride + x];
}

void Gl3Raster::compute_mip_level_metadata() {
	uint32_t w = width_;
	uint32_t h = height_;
	uint32_t s = stride_;
	uint32_t minDim = 1;

	switch (native_internal_format_){
	case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
	case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
	case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
	case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
		minDim = 4;
		break;
	}

	std::list<MipLevelMetadata> levels;
	levels.push_back(MipLevelMetadata{.width = w, .height = h, .stride = s});
	for (uint32_t i = 0; (w > minDim) || (h > minDim); i++) {
		if (w > minDim) {
			w /= 2;
			s /= 2;
		}
		if (h > minDim) {
			h /= 2;
		}
		levels.push_back(MipLevelMetadata{.width = w, .height = h, .stride = s});
	}
	mip_levels_ = { levels.begin(), levels.end() };
}

const MipLevelMetadata& Gl3Raster::mip_level_meta_data(uint32_t level) const
{
	if (level > mip_levels_.size()) {
		THROW_OR_ABORT("Mip-level out of bounds");
	}
	return mip_levels_[level];
}

uint32_t Gl3Raster::num_levels() const {
	return integral_cast<uint32_t>(mip_levels_.size());
}

void Gl3Raster::allocate_dxt(const RasterConfig& cfg) {
	if (type() != Raster::TEXTURE) {
		THROW_OR_ABORT("allocate_dxt requires \"type == texture\"");
	}
	switch (compression_) {
	case 1:
		if (has_alpha_) {
			native_internal_format_ = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
			format_ = GL_RGBA;
		}
		else {
			native_internal_format_ = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
			format_ = GL_RGB;
		}
		// bogus, but stride*height should be the size of the image
		// 4x4 in 8 bytes
		stride_ = width_ / 2;
		break;
	case 3:
		native_internal_format_ = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
		format_ = GL_RGBA;
		// 4x4 in 16 bytes
		stride_ = width_;
		break;
	case 5:
		native_internal_format_ = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
		format_ = GL_RGBA;
		// 4x4 in 16 bytes
		stride_ = width_;
		break;
	default:
		THROW_OR_ABORT("Invalid DXT format");
	}
	native_type_ = GL_UNSIGNED_BYTE;
	native_has_alpha = has_alpha_;
	native_bpp_ = 2;
	native_depth_ = 16;

	native_is_compressed_ = true;
	if (format_ & Raster::MIPMAP)
		native_num_levels_ = num_levels_;
	native_autogen_mipmap_ = (format_ & (Raster::MIPMAP | Raster::AUTOMIPMAP)) == (Raster::MIPMAP | Raster::AUTOMIPMAP);
	if (native_autogen_mipmap_)
		native_num_levels_ = 1;

	CHK(glGenTextures(1, &native_texture_id_));
	{
		BindTextureGuard btg{ GL_TEXTURE_2D, native_texture_id_ };
		CHK(glTexImage2D(GL_TEXTURE_2D, 0, native_internal_format_,
			width_, height_,
			0, native_format_, native_type_, nullptr));
		// TODO: allocate other levels...probably
		CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, native_num_levels_ - 1));
		// natras->filterMode = 0;
		// natras->addressU = 0;
		// natras->addressV = 0;
		// natras->maxAnisotropy = 1;
	}

	if (gl3Caps.gles && cfg.need_to_read_back_textures) {
		// Uh oh, need to keep a copy in cpu memory
		uint32_t size = 0;
		for (uint32_t i = 0; i < native_num_levels_; i++)
			size += mip_level_meta_data(i).size();
		native_backing_store_.levels.resize(native_num_levels_);
		native_backing_store_.format = 0;	// not needed
		for (uint32_t i = 0; i < native_num_levels_; i++) {
			auto size = mip_level_meta_data(i).size();
			native_backing_store_.levels[i].data.resize(size);
			native_backing_store_.levels[i].width = 0;	// we don't need those here, maybe later...
			native_backing_store_.levels[i].height = 0;
		}
	}

	format_ &= ~Raster::DONTALLOCATE;
}

uint8_t* Gl3Raster::lock(uint32_t level, uint32_t lock_mode)
{
	if (private_flags_ != 0) {
		THROW_OR_ABORT("Raster private flags not zero");
	}

	switch (type()) {
	case Raster::NORMAL:
	case Raster::TEXTURE:
	case Raster::CAMERATEXTURE: {
		const auto& level_meta_data = mip_level_meta_data(level);
		auto alloc_sz = level_meta_data.size();
		pixels_.resize(alloc_sz);

		if (lock_mode & Raster::LOCKREAD || !(lock_mode & Raster::LOCKNOFETCH)) {
			if (native_is_compressed_) {
				if (!native_backing_store_.levels.empty()) {
					if (level > native_backing_store_.levels.size()) {
						THROW_OR_ABORT("Mipmap level too large");
					}
					if (alloc_sz != native_backing_store_.levels[level].data.size()) {
						THROW_OR_ABORT("Unexpected level data size");
					}
					std::memcpy(pixels_.data(), native_backing_store_.levels[level].data.data(), alloc_sz);
				} else {
					// GLES is losing here
					BindTextureGuard btg{ GL_TEXTURE_2D, native_texture_id_ };
					CHK(glGetCompressedTexImage(GL_TEXTURE_2D, level, pixels_.data()));
				}
			} else if (gl3Caps.gles) {
				if (native_format_ != GL_RGBA) {
					THROW_OR_ABORT("Unexpected native format");
				}
				FrameBufferStorage2D fbs{ native_texture_id_, 0 };
				CHK(glReadPixels(0, 0, level_meta_data.width, level_meta_data.height, native_format_, native_type_, pixels_.data()));
				//e = glGetError(); printf("GL err4 %x (%x)\n", e, natras->format);
			} else {
				BindTextureGuard btg{ GL_TEXTURE_2D, native_texture_id_ };
				CHK(glPixelStorei(GL_PACK_ALIGNMENT, 1));
				CHK(glGetTexImage(GL_TEXTURE_2D, level, native_format_, native_type_, pixels_.data()));
			}
		}

		private_flags_ = lock_mode;
		break;
	}

	case Raster::CAMERA: {
		if (lock_mode & Raster::PRIVATELOCK_WRITE) {
			THROW_OR_ABORT("Can't lock framebuffer for writing");
		}
		if (native_bpp_ != 3) {
			THROW_OR_ABORT("Camera raster is not RGB");
		}
		auto alloc_sz = height_ * stride_;
		pixels_.resize(alloc_sz);
		CHK(glReadBuffer(GL_BACK));
		CHK(glReadPixels(0, 0, width_, height_, GL_RGB, GL_UNSIGNED_BYTE, pixels_.data()));

		private_flags_ = lock_mode;
		break;
	}

	default:
		THROW_OR_ABORT("Cannot lock this type of raster yet");
	}
	locked_level_ = level;
	return pixels_.data();
}

void Gl3Raster::unlock()
{
	if (pixels_.empty()) {
		THROW_OR_ABORT("No raster pixels allocated");
	}
	if (!locked_level_.has_value()) {
		THROW_OR_ABORT("Raster unlock without previous lock");
	}
	auto level = locked_level_.value();

	switch (type()) {
	case Raster::NORMAL:
	case Raster::TEXTURE:
	case Raster::CAMERATEXTURE:
		if (private_flags_ & Raster::LOCKWRITE) {
			const auto& level_meta_data = mip_level_meta_data(level);
			BindTextureGuard btg{ GL_TEXTURE_2D, native_texture_id_ };
			if (native_is_compressed_) {
				glCompressedTexImage2D(GL_TEXTURE_2D, level, native_internal_format_,
					level_meta_data.width, level_meta_data.height, 0,
					level_meta_data.size(),
					pixels_.data());
				if (!native_backing_store_.levels.empty()) {
					if (level >= native_backing_store_.levels.size()) {
						THROW_OR_ABORT("Level index too large for backing-store");
					}
					memcpy(native_backing_store_.levels[level].data.data(), pixels_.data(),
						native_backing_store_.levels[level].data.size());
				}
			} else {
				glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
				glTexImage2D(GL_TEXTURE_2D, level, native_internal_format_,
					level_meta_data.width, level_meta_data.height,
					0, native_format_, native_type_, pixels_.data());
			}
			if (level == 0 && native_autogen_mipmap_)
				glGenerateMipmap(GL_TEXTURE_2D);
		}
		break;

	case Raster::CAMERA:
		// TODO: write?
		break;
	}

	pixels_.clear();
	private_flags_ = 0;
	locked_level_.reset();
}

Gl3Raster::Gl3Raster(
	uint32_t width,
	uint32_t height,
	uint32_t depth,
	uint32_t format,
	uint32_t compression,
	uint32_t num_levels,
	bool has_alpha,
	const RasterConfig& cfg)
	: width_{ width }
	, height_{ height }
	, depth_{ depth }
	, format_{ format }
	, compression_{ compression }
	, num_levels_{ num_levels }
	, has_alpha_{ has_alpha }
{
	if ((format & 0xF) != Raster::TEXTURE) {
		THROW_OR_ABORT("Invalid raster type");
	}
	if (compression != 0) {
		allocate_dxt(cfg);
		custom_format_ = 1;
	}
}

Gl3Raster::~Gl3Raster() = default;

void Gl3Raster::from_image(const Image& image)
{
	void (*conv)(uint8_t *out, const uint8_t *in) = nullptr;

	// Unpalettize image if necessary but don't change original
	Image truecolimg;
	const Image* pimage;
	if (image.depth <= 8) {
		truecolimg.width = image.width;
		truecolimg.height = image.height;
		truecolimg.depth = image.depth;
		truecolimg.pixels = image.pixels;
		truecolimg.stride = image.stride;
		truecolimg.palette = image.palette;
		truecolimg.unpalletize();
		pimage = &truecolimg;
	} else {
		pimage = &image;
	}

	uint32_t fmt = format_ & 0xF00;
	if (compression_ == 0) {
		THROW_OR_ABORT("Texture is not compressed");
	}
	switch(image.depth){
	case 32:
		if (gl3Caps.gles)
			conv = conv_RGBA8888_from_RGBA8888;
		else if (fmt == Raster::C8888)
			conv = conv_RGBA8888_from_RGBA8888;
		else if(fmt == Raster::C888)
			conv = conv_RGB888_from_RGB888;
		else
			goto err;
		break;
	case 24:
		if (gl3Caps.gles)
			conv = conv_RGBA8888_from_RGB888;
		else if(fmt == Raster::C8888)
			conv = conv_RGBA8888_from_RGB888;
		else if(fmt == Raster::C888)
			conv = conv_RGB888_from_RGB888;
		else
			goto err;
		break;
	case 16:
		if (gl3Caps.gles)
			conv = conv_RGBA8888_from_ARGB1555;
		else if(fmt == Raster::C1555)
			conv = conv_RGBA5551_from_ARGB1555;
		else
			goto err;
		break;

	case 8:
	case 4:
	default:
	err:
		THROW_OR_ABORT("Unsupported image depth");
	}

	if (has_alpha_ != image.has_alpha()) {
		THROW_OR_ABORT("Conflicting alpha attribute");
	}

	bool unlock_required = false;
	if (pixels_.empty()) {
		lock(0, Raster::LOCKWRITE | Raster::LOCKNOFETCH);
		unlock_required = true;
	}

	if (pixels_.size() != stride_ * height_) {
		THROW_OR_ABORT("Unexpected number of allocated pixels");
	}
	const uint8_t *imgpixels = image.pixels.data() + (image.height - 1) * image.stride;

	if (image.width != width_) {
		THROW_OR_ABORT("Unexpected image width");
	}
	if (image.height == height_) {
		THROW_OR_ABORT("Unexpected image height");
	}
	uint8_t* pixels = pixels_.data();
	for (uint32_t y = 0; y < image.height; y++) {
		const uint8_t *imgrow = imgpixels;
		uint8_t *rasrow = pixels;
		for(uint32_t x = 0; x < image.width; x++){
			conv(rasrow, imgrow);
			imgrow += image.bpp;
			rasrow += native_bpp_;
		}
		imgpixels -= image.stride;
		pixels += stride_;
	}
	if (unlock_required)
		unlock();
}

uint32_t Gl3Raster::type() const {
	return format_ & 0x7;
}

uint32_t Gl3Raster::flags() const {
	return format_ & 0xF8;
}

uint32_t Gl3Raster::format() const {
	return format_ & 0xFF00;
}
