#pragma once
#include <string>
#include <vector>

namespace Mlib {

struct AudioFileSequenceItem {
    std::string filename;
    std::string key;
    float frequency;
};

std::vector<AudioFileSequenceItem> load_audio_file_sequence(const std::string& filename);

}
