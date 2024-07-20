#include "Gl3_Texture_Handle.hpp"

using namespace Mlib;

Gl3TextureHandle::Gl3TextureHandle()
	: handle_{ (GLuint)-1 }
{}

Gl3TextureHandle::~Gl3TextureHandle() = default;

Gl3TextureHandle::~Gl3TextureHandle() = default;

uint32_t Gl3TextureHandle::handle32() const {
	return handle_;
}

uint64_t Gl3TextureHandle::handle64() const {
	THROW_OR_ABORT("Unsupported texture handle type");
}

uint32_t& Gl3TextureHandle::handle32() {
	return handle_;
}

uint64_t& Gl3TextureHandle::handle64() {
	THROW_OR_ABORT("Unsupported texture handle type");
}
