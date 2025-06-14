#pragma once
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Math.hpp>
#include <cmath>
#include <ostream>

namespace Mlib {

static const std::array<FixedArray<int, 2>, 9> discrete_velocity_directions_d2q9 = {
    FixedArray<int, 2>{1, -1}, FixedArray<int, 2>{1, 0}, FixedArray<int, 2>{1, 1},
    FixedArray<int, 2>{0, -1}, FixedArray<int, 2>{0, 0}, FixedArray<int, 2>{0, 1},
    FixedArray<int, 2>{-1, -1}, FixedArray<int, 2>{-1, 0}, FixedArray<int, 2>{-1, 1}};
    
template <class T>
struct LbmModelD2Q9 {
    using type = T;

    static const size_t ndirections = 9;

    constexpr static const T weights[ndirections] = {
        (T)1 / 36, (T)1 / 9, (T)1 / 36,
        (T)1 / 9, (T)4 / 9, (T)1 / 9,
        (T)1 / 36, (T)1 / 9, (T)1 / 36};

    constexpr static const std::array<FixedArray<int, 2>, ndirections>& discrete_velocity_directions =
        discrete_velocity_directions_d2q9;
};

template <class TModel>
class FluidSubdomain {
    using T = TModel::type;
    static const size_t ndirections = TModel::ndirections;
    constexpr static const T speed_of_sound = 1 / std::sqrt((T)3);
    constexpr static const T speed_of_sound2 = squared(speed_of_sound);
    constexpr static const T speed_of_sound4 = squared(speed_of_sound2);
    constexpr static const T time_relaxation_constant = (T)0.55;
    constexpr static const size_t temp_field_id_ = 0;
    constexpr static const size_t good_field_id_ = 1;
public:
    explicit FluidSubdomain(const FixedArray<size_t, 2>& subdomain_size)
        : subdomain_size_{ subdomain_size }
        , velocity_magnitudes_fields_{ArrayShape{2, ndirections, subdomain_size(0), subdomain_size(1)}}
        , velocity_field_{ArrayShape{2, subdomain_size(0), subdomain_size(1)}}
        , density_field_{ArrayShape{subdomain_size(0), subdomain_size(1)}}
    {
        if (any(subdomain_size_ == 0uz)) {
            THROW_OR_ABORT("Subdomain size cannot be zero");
        }
        for (size_t dir = 0; dir < ndirections; ++dir) {
            // This is only necessary for the boundary values
            velocity_magnitudes_fields_[temp_field_id_][dir] = TModel::weights[dir];
            velocity_magnitudes_fields_[good_field_id_][dir] = TModel::weights[dir];
        }
        calculate_macroscopic_variables();
    }
    T density(const FixedArray<size_t, 2>& coords) const {
        T res = (T)0;
        for (size_t v = 0; v < ndirections; ++v) {
            res += velocity_magnitudes_fields_(good_field_id_, v, coords(0), coords(1));
        }
        return res;
    }
    FixedArray<T, 2> velocity(const FixedArray<size_t, 2>& coords) const {
        const auto& dirs = TModel::discrete_velocity_directions;
        auto result = fixed_zeros<T, 2>();
        for (size_t v = 0; v < ndirections; ++v) {
            result += (dirs[v].template casted<T>()) * velocity_magnitudes_fields_(good_field_id_, v, coords(0), coords(1));
        }
        return result;
    }
    FixedArray<T, 2> velocity_field(const FixedArray<size_t, 2>& coords) const {
        return FixedArray<T, 2>{
            velocity_field_(0, coords(0), coords(1)),
            velocity_field_(1, coords(0), coords(1))};
    }
    T density_field(const FixedArray<size_t, 2>& coords) const {
        return density_field_(coords);
    }
    FixedArray<T, 2> momentum_field(const FixedArray<size_t, 2>& coords) const {
        return velocity_field(coords) * density_field(coords);
    }
    void set_velocity_field(
        const FixedArray<size_t, 2>& coords,
        const FixedArray<T, 2>& value)
    {
        velocity_field_(0, coords(0), coords(1)) = value(0);
        velocity_field_(1, coords(0), coords(1)) = value(1);
    }
    void iterate() {
        collide();
        stream();
        calculate_macroscopic_variables();
    }
    void print(std::ostream& ostr) const {
        T offset = 127;
        T scale = 127;
        for (size_t y = 0; y < subdomain_size_(1); ++y) {
            for (size_t x = 0; x < subdomain_size_(0); ++x) {
                auto flow_momentum = momentum_field({x, y});
                ostr << "\033[38;2;" <<
                    (int)std::round(std::clamp<T>(offset + scale * flow_momentum(1), 0, 255)) << ';' <<
                    (int)std::round(std::clamp<T>(offset + scale * flow_momentum(0), 0, 255)) <<
                    ";0m██";
            }
            ostr << '\n';
        }
    }
private:
    void collide() {
        const auto& dirs = TModel::discrete_velocity_directions;
        const auto& weights = TModel::weights;
        for (size_t x = 0; x < subdomain_size_(0); ++x) {
            for (size_t y = 0; y < subdomain_size_(1); ++y) {
                auto flow_velocity = velocity_field({x, y});
                auto dens = density_field({x, y});
                auto vel2 = sum(squared(flow_velocity));
                for (size_t v = 0; v < ndirections; ++v) {
                    auto velocity_v = velocity_magnitudes_fields_(good_field_id_, v, x, y);
                    auto first_term = velocity_v;
                    // the flow velocity
                    auto dotted = dot0d(flow_velocity, dirs[v].template casted<T>());
                    // the taylor expainsion of equilibrium term
                    auto taylor = 1 + (dotted / speed_of_sound2) + (squared(dotted) / (2 * speed_of_sound4)) -
                        (vel2 / (2 * speed_of_sound2));
                    auto equilibrium = dens * taylor * weights[v];
                    if ((x == 0) || (x == subdomain_size_(0) - 1) ||
                        (y == 0) || (y == subdomain_size_(1) - 1))
                    {
                        velocity_magnitudes_fields_(temp_field_id_, v, x, y) = equilibrium;
                    } else {
                        auto second_term = (equilibrium - velocity_v) / time_relaxation_constant;
                        velocity_magnitudes_fields_(temp_field_id_, v, x, y) = first_term + second_term;
                    }
                }
            }
        }
    }
    void stream() {
        const auto& dirs = TModel::discrete_velocity_directions;
        for (size_t v = 0; v < ndirections; ++v) {
            const auto& dir = dirs[v];
            for (size_t x = 1; x < subdomain_size_(0) - 1; ++x) {
                for (size_t y = 1; y < subdomain_size_(1) - 1; ++y) {
                    size_t source_x = x - dir(0);
                    size_t source_y = y - dir(1);
                    velocity_magnitudes_fields_(good_field_id_, v, x, y) = velocity_magnitudes_fields_(temp_field_id_, v, source_x, source_y);
                }
            }
        }
    }
    void calculate_macroscopic_variables() {
        for (size_t x = 0; x < subdomain_size_(0); ++x) {
            for (size_t y = 0; y < subdomain_size_(1); ++y) {
                auto dens = density({x, y});
                auto vel = velocity({x, y});
                density_field_(x, y) = dens;
                velocity_field_(0, x, y) = vel(0) / dens;
                velocity_field_(1, x, y) = vel(1) / dens;
            }
        }
    }
    FixedArray<size_t, 2> subdomain_size_;
    Array<T> velocity_magnitudes_fields_;
    Array<T> velocity_field_;
    Array<T> density_field_;
};

}
