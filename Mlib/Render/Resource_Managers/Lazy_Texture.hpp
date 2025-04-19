#pragma once
#include <Mlib/Geometry/Material/Colormap_With_Modifiers.hpp>
#include <Mlib/Geometry/Texture/ITexture_Handle.hpp>
#include <Mlib/Memory/Deallocation_Token.hpp>

namespace Mlib {

class RenderingResources;
enum class TextureRole;

class LazyTexture: public ITextureHandle {
public:
	explicit LazyTexture(
        const RenderingResources& rendering_resources,
        const ColormapWithModifiers& colormap,
        TextureRole role);
    virtual ~LazyTexture() override;
    virtual uint32_t handle32() const override;
    virtual uint64_t handle64() const override;
    virtual uint32_t& handle32() override;
    virtual uint64_t& handle64() override;
    virtual bool texture_is_loaded_and_try_preload() override;
    virtual ColorMode color_mode() const override;
    virtual MipmapMode mipmap_mode() const override;
    virtual WrapMode wrap_modes(size_t i) const override;
private:
    void deallocate();
    ITextureHandle& texture();
    const ITextureHandle& texture() const;
    const RenderingResources& rendering_resources_;
    mutable std::shared_ptr<ITextureHandle> texture_;
	ColormapWithModifiers colormap_;
    TextureRole role_;
    DeallocationToken deallocation_token_;
};

}
