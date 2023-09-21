#pragma once
#include <Mlib/Audio/OpenAL_al.h>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <cstddef>
#include <optional>

namespace Mlib {

template <class TDir, class TPos, size_t n>
class TransformationMatrix;

class AudioListener {
    AudioListener() = delete;
    AudioListener(const AudioListener &) = delete;
    AudioListener &operator=(const AudioListener &) = delete;

public:
    static void set_transformation(const TransformationMatrix<float, double, 3> &trafo);
    static std::optional<FixedArray<float, 3>>
    get_relative_position(const FixedArray<double, 3> &position);
    static void set_gain(float f);
    static void mute();
    static void unmute();

private:
    static bool muted_;
    static float gain_;
    static std::optional<TransformationMatrix<float, double, 3>> view_matrix_;
};

}
