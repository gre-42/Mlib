#pragma once
#include <Mlib/Math/Orderable_Fixed_Array.hpp>
#include <Mlib/Stats/Min_Max.hpp>

namespace Mlib {

class AddableStepDistances;

class SquaredStepDistances {
    friend struct std::hash<Mlib::SquaredStepDistances>;
public:
    static inline SquaredStepDistances from_distances(float near, float far);
    static inline SquaredStepDistances from_distances(const FixedArray<float, 2>& d1);
    inline AddableStepDistances sqrt() const;
    inline SquaredStepDistances operator + (float radius) const;
    inline SquaredStepDistances& operator += (float radius);
    inline float operator () (size_t i) const;
    inline bool operator < (const SquaredStepDistances& rhs) const {
        return OrderableFixedArray{distances2_} < OrderableFixedArray{rhs.distances2_};
    }
    inline bool operator == (const SquaredStepDistances& rhs) const {
        return OrderableFixedArray{distances2_} == OrderableFixedArray{rhs.distances2_};
    }
    inline bool operator != (const SquaredStepDistances& rhs) const {
        return OrderableFixedArray{distances2_} != OrderableFixedArray{rhs.distances2_};
    }
    inline bool operator > (const SquaredStepDistances& rhs) const {
        return OrderableFixedArray{distances2_} > OrderableFixedArray{rhs.distances2_};
    }
    inline std::strong_ordering operator <=> (const SquaredStepDistances& rhs) const {
        return OrderableFixedArray{distances2_} <=> OrderableFixedArray{rhs.distances2_};
    }
    template <class Archive>
    void serialize(Archive& archive) {
        archive(distances2_);
    }
private:
    inline explicit SquaredStepDistances(const FixedArray<float, 2>& d1);
    FixedArray<float, 2> distances2_;
};

class AddableStepDistances {
public:
    inline AddableStepDistances(float near, float far)
        : distances_{ near, far }
    {}
    inline SquaredStepDistances squared() const {
        return SquaredStepDistances::from_distances(
            maximum(distances_, 0.f));
    }
    inline AddableStepDistances operator + (float radius) const {
        return AddableStepDistances{ distances_ + radius };
    }
    inline AddableStepDistances& operator += (float radius) {
        *this = *this + radius;
        return *this;
    }
private:
    inline AddableStepDistances(const FixedArray<float, 2>& distances)
        : distances_{ distances }
    {}
    FixedArray<float, 2> distances_;
};

SquaredStepDistances SquaredStepDistances::from_distances(float near, float far) {
    return SquaredStepDistances{{ near, far }};
}

SquaredStepDistances SquaredStepDistances::from_distances(const FixedArray<float, 2>& d1) {
    return SquaredStepDistances{d1};
}

SquaredStepDistances::SquaredStepDistances(const FixedArray<float, 2>& d1)
    : distances2_{ uninitialized }
{
    if (!all(d1 >= 0.f)) {
        THROW_OR_ABORT("SquaredStepDistances received negative values");
    }
    distances2_ = squared(d1);
}

AddableStepDistances SquaredStepDistances::sqrt() const {
    return AddableStepDistances{
        distances2_(0) == 0.f ? -INFINITY : std::sqrt(distances2_(0)),
        std::sqrt(distances2_(1))
    };
}

SquaredStepDistances SquaredStepDistances::operator + (float radius) const {
    return (sqrt() + radius).squared();
}

SquaredStepDistances& SquaredStepDistances::operator += (float radius) {
    *this = *this + radius;
    return *this;
}

float SquaredStepDistances::operator () (size_t i) const {
    return distances2_(i);
}

// Default: No fade-in (0, 0), no fade-out (INFINITY, INFINITY)
static const OrderableFixedArray<float, 4> default_linear_distances{ 0.f, 0.f, INFINITY, INFINITY };
static const OrderableFixedArray<float, 4> default_linear_cosines{ -1.f, -1.f, 1.f, 1.f };
static const OrderableFixedArray<float, 2> default_step_distances{ 0.f, INFINITY };
static auto default_step_distances2 = SquaredStepDistances::from_distances(default_step_distances);

}

template <>
struct std::hash<Mlib::SquaredStepDistances>
{
    std::size_t operator() (const Mlib::SquaredStepDistances& a) const {
        Mlib::Hasher hasher{ 0xc0febabe };
        hasher.combine(Mlib::OrderableFixedArray{a.distances2_});
        return hasher;
    }
};
