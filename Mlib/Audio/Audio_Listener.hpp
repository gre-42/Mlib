#pragma once
#include <al.h>
#include <cstddef>

namespace Mlib {

template <class TDir, class TPos, size_t n>
class TransformationMatrix;

class AudioListener {
public:
    AudioListener() = delete;
    AudioListener(const AudioListener&) = delete;
    AudioListener& operator = (const AudioListener&) = delete;
    static void set_transformation(const TransformationMatrix<float, float, 3>& trafo);
    static void set_gain(float f);
    static void mute();
    static void unmute();
private:
    static bool muted_;
    static float gain_;
};

}
