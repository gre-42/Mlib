#pragma once
#include <Mlib/Images/Flip_Mode.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <stb/stb_image.h>
#include <string>
#include <variant>
#include <vector>

#if defined(STBI_MALLOC) || defined(STBI_FREE) || defined(STBI_REALLOC) || defined(STBI_REALLOC_SIZED)
#error Do not define STBI_MALLOC etc.
#endif

template <class TData>
class StbInfo {
public:
    inline StbInfo()
        : width{ 0 }
        , height{ 0 }
        , nrChannels{ 0 }
        , data_{ nullptr, &stbi_image_free }
    {}
    inline StbInfo(int width, int height, int nrChannels, TData* data)
        : width{ width }
        , height{ height }
        , nrChannels{ nrChannels }
        , data_{ data, &stbi_image_free }
    {}
    inline StbInfo(int width, int height, int nrChannels)
        : width{ width }
        , height{ height }
        , nrChannels{ nrChannels }
        , data_{ nullptr, &stbi_image_free }
    {
        if ((width < 0) || (width > 16'000)) {
            THROW_OR_ABORT("StbInfo: width negative or too large");
        }
        if ((height < 0) || (height > 16'000)) {
            THROW_OR_ABORT("StbInfo: height negative or too large");
        }
        if ((nrChannels < 0) || (nrChannels > 4)) {
            THROW_OR_ABORT("StbInfo: nrChannels negative or too large");
        }
        data_.reset((TData*)::malloc(size_t(width * height * nrChannels) * sizeof(TData)));
        if (data_ == nullptr) {
            THROW_OR_ABORT("StbInfo: Cannot allocate image");
        }
    }
    StbInfo(StbInfo&& other) = default;
    StbInfo& operator = (StbInfo&& other) = default;
    int width;
    int height;
    int nrChannels;
    inline TData* data() {
        return data_.get();
    }
    inline const TData* data() const {
        return data_.get();
    }
    inline TData& operator [] (size_t i) {
        return data_.get()[i];
    }
    inline const TData& operator [] (size_t i) const {
        return data_.get()[i];
    }
    inline TData& operator () (int x, int y, int channel) {
        return data_.get()[(size_t)(channel + (x + y * width) * nrChannels)];
    }
    inline const TData& operator () (int x, int y, int channel) const {
        return const_cast<StbInfo&>(*this)(x, y, channel);
    }
private:
    std::unique_ptr<TData, decltype(&stbi_image_free)> data_;
};

enum class IncorrectDatasizeBehavior {
    THROW,
    CONVERT
};

template <class TData>
void stb_image_flip_horizontally(StbInfo<TData>& image);

std::variant<StbInfo<uint8_t>, StbInfo<uint16_t>> stb_load(
    const std::string& filename,
    Mlib::FlipMode flip_mode,
    const std::vector<std::byte>* data = nullptr);
StbInfo<uint8_t> stb_load8(
    const std::string& filename,
    Mlib::FlipMode flip_mode,
    const std::vector<std::byte>* data = nullptr,
    IncorrectDatasizeBehavior datasize_behavior = IncorrectDatasizeBehavior::THROW);
