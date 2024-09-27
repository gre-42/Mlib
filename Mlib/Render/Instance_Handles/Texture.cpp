#include "Texture.hpp"
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Deallocate/Render_Deallocator.hpp>
#include <Mlib/Render/Deallocate/Render_Try_Delete.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

static inline GLuint check_handle(GLuint handle) {
	if (handle == (GLuint)-1) {
		THROW_OR_ABORT("Unsupported texture handle");
	}
	return handle;
}

Texture::Texture(GenerateTexture)
	: handle_{ (GLuint)-1 }
	, deallocation_token_ { render_deallocator.insert([this]() { deallocate(); })
}
{
	CHK(glGenTextures(1, &handle_));
}

Texture::Texture(GLuint handle)
	: handle_{ check_handle(handle) }
	, deallocation_token_{ render_deallocator.insert([this]() { deallocate(); }) }
{}

Texture::Texture(Texture&& other) noexcept
	: handle_{ other.handle_ }
	, deallocation_token_{ std::move(other.deallocation_token_) }
{
	other.handle_ = (GLuint)-1;
}

Texture::~Texture() {
	deallocate();
}

Texture& Texture::operator = (GLuint handle) {
	deallocate();
	handle_ = handle;
	return *this;
}

void Texture::deallocate() {
	if (handle_ != (GLuint)-1) {
		try_delete_texture(handle_);
	}
}

uint32_t Texture::handle32() const {
	return handle_;
}

uint64_t Texture::handle64() const {
	THROW_OR_ABORT("Unsupported texture handle type");
}

uint32_t& Texture::handle32() {
	return handle_;
}

uint64_t& Texture::handle64() {
	THROW_OR_ABORT("Unsupported texture handle type");
}
