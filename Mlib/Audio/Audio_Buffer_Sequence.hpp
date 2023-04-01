#pragma once
#include <memory>
#include <vector>

namespace Mlib {

class AudioBuffer;

struct AudioBufferAndFrequency {
    std::shared_ptr<AudioBuffer> buffer;
    float frequency;
};

class AudioBufferSequence {
public:
    explicit AudioBufferSequence(std::vector<AudioBufferAndFrequency> buffers);
    const AudioBufferAndFrequency& get_buffer_and_frequency(float frequency) const;
private:
    std::vector<AudioBufferAndFrequency> buffers_;
};

}
