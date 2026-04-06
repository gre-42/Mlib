
#include "Lazy_Texture.hpp"
#include <Mlib/OpenGL/Deallocate/Render_Deallocator.hpp>
#include <Mlib/OpenGL/Resource_Managers/Rendering_Resources.hpp>

using namespace Mlib;

LazyTexture::LazyTexture(
	const RenderingResources& rendering_resources,
	const ColormapWithModifiers& colormap,
	TextureRole role)
	: rendering_resources_{ rendering_resources }
	, colormap_{ role == TextureRole::COLOR_FROM_DB
		? rendering_resources.colormap(colormap)
		: colormap }
	, role_{ (role == TextureRole::COLOR_FROM_DB)
		? TextureRole::COLOR
		: role }
	, deallocation_token_{ render_deallocator.insert([this]() { deallocate(); }) }
{
	if (!colormap.hash.has_value()) {
		throw std::runtime_error("LazyTexture: Colormap hash not computed");
	}
}

LazyTexture::~LazyTexture() = default;

void LazyTexture::deallocate() {
	texture_ = nullptr;
}

uint32_t LazyTexture::handle32() const {
	return texture().handle32();
}

uint64_t LazyTexture::handle64() const {
	return texture().handle32();
}

uint32_t& LazyTexture::handle32() {
	return texture().handle32();
}

uint64_t& LazyTexture::handle64() {
	return texture().handle64();
}

bool LazyTexture::texture_is_loaded_and_try_preload() {
	// Note that this function does not call "texture_->texture_is_loaded_and_try_preload()".
	// => The child texture need not implement this method.
	return
		(texture_ != nullptr) ||
		rendering_resources_.texture_is_loaded_and_try_preload(colormap_, role_);
}

void LazyTexture::load_gpu() {
	texture().load_gpu();
}

TextureTarget LazyTexture::target() const {
	return texture().target();
}

ColorMode LazyTexture::color_mode() const {
	return colormap_.color_mode;
}

MipmapMode LazyTexture::mipmap_mode() const {
	return colormap_.mipmap_mode;
}

InterpolationMode LazyTexture::magnifying_interpolation_mode() const {
	return colormap_.magnifying_interpolation_mode;
}

WrapMode LazyTexture::wrap_modes(size_t i) const {
	assert_true(i < 2);
	return colormap_.wrap_modes(i);
}

FixedArray<float, 4> LazyTexture::border_color() const {
	return colormap_.border_color;
}

uint32_t LazyTexture::layers() const {
	return texture().layers();
}

ITextureHandle& LazyTexture::texture() {
	if (texture_ == nullptr) {
		texture_ = rendering_resources_.get_texture(colormap_, role_, CallerType::RENDER);
	}
	return *texture_;
}

const ITextureHandle& LazyTexture::texture() const {
	return const_cast<LazyTexture*>(this)->texture();
}
