#pragma once
#include <cstddef>
#include <string>
#include <vector>

namespace Mlib {

struct KeyDescription;

class KeyDescriptions {
public:
    KeyDescriptions();
    ~KeyDescriptions();
    void load(const std::string& filename);
    void append(KeyDescription key_description);
    const KeyDescription& operator [] (size_t i) const;
    size_t size() const;
    std::vector<KeyDescription>::const_iterator begin() const;
    std::vector<KeyDescription>::const_iterator end() const;
private:
    std::vector<KeyDescription> key_descriptions_;
};

}
