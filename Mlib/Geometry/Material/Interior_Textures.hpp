#pragma once
#include <iosfwd>
#include <string>

namespace Mlib {

static const size_t INTERIOR_LEFT = 0;
static const size_t INTERIOR_RIGHT = 1;
static const size_t INTERIOR_FLOOR = 2;
static const size_t INTERIOR_CEILING = 3;
static const size_t INTERIOR_BACK = 4;
static const size_t INTERIOR_COUNT = 5;

struct InteriorTextures {
    std::string left;
    std::string right;
    std::string floor;
    std::string ceiling;
    std::string back;
    std::strong_ordering operator <=> (const InteriorTextures&) const = default;
    static InteriorTextures parse(const std::string& text);
    bool empty() const;
    const std::string& operator [](size_t index) const;
    template <class Archive>
    void serialize(Archive& archive) {
        archive(left);
        archive(right);
        archive(floor);
        archive(ceiling);
        archive(back);
    }
};

std::ostream& operator << (std::ostream& ostr, const InteriorTextures& t);

}
