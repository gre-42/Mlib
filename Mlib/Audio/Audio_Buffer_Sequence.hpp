#pragma once
#include <memory>
#include <string>
#include <vector>

namespace Mlib {

class AudioBuffer;

struct AudioBufferAndFrequency {
    std::shared_ptr<AudioBuffer> buffer;
    float frequency;
};

enum class PitchAdjustmentStrategy {
    ROUNDING,
    UP_SAMPLING,
    DOWN_SAMPLING
};

PitchAdjustmentStrategy pitch_adjustment_strategy_from_string(const std::string& s);

class AudioBufferSequence {
public:
    explicit AudioBufferSequence(std::vector<AudioBufferAndFrequency> buffers);
    const AudioBufferAndFrequency& get_buffer_and_frequency(
        float frequency,
        PitchAdjustmentStrategy strategy = PitchAdjustmentStrategy::ROUNDING) const;
private:
    std::vector<AudioBufferAndFrequency> buffers_;
};

}