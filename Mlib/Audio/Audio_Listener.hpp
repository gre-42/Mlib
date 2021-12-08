#pragma once
#include <AL/al.h>
#include <cstddef>

namespace Mlib {

template <class TData, size_t n>
class TransformationMatrix;

class AudioListener {
public:
    AudioListener() = delete;
    AudioListener(const AudioListener&) = delete;
    AudioListener& operator = (const AudioListener&) = delete;
    static void set_transformation(const TransformationMatrix<float, 3>& trafo);
    static void set_gain(float f);
};

}
