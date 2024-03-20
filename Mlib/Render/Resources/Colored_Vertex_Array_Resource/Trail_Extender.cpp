#include "Trail_Extender.hpp"
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Coordinates/Gl_Look_At.hpp>
#include <Mlib/Math/Orderable_Fixed_Array.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Render/Batch_Renderers/Trails_Instance.hpp>
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource/Trail_Sequence.hpp>

using namespace Mlib;

TrailExtender::TrailExtender(
    TrailsInstance& trails_instance,
    const TrailSequence& trail_sequence,
    const std::vector<FixedArray<ColoredVertex<float>, 3>>& segment,
    double minimum_length)
    : trails_instance_{ trails_instance }
    , trail_sequence_{ trail_sequence }
    , segment_{ segment }
    , minimum_length_squared_{ squared(minimum_length) }
{
    if (segment.empty()) {
        THROW_OR_ABORT("Trail segment is empty");
    }
}

void TrailExtender::append_location(const TransformationMatrix<float, double, 3>& location) {
    if (trails_instance_.time() == std::chrono::steady_clock::time_point()) {
        THROW_OR_ABORT("Trail instance move not called");
    }
    if (previous_center_.has_value()) {
        auto& prev = previous_center_.value();
        auto dz = (location.t() - prev.position).casted<float>();
        auto dz_l2 = sum(squared(dz));
        if (dz_l2 < minimum_length_squared_) {
            return;
        }
        auto lookat = gl_lookat_relative(dz / std::sqrt(dz_l2), location.R().column(1));
        if (!lookat.has_value()) {
            return;
        }
        const auto& R = lookat.value();
        auto RS = R;
        for (size_t r = 0; r < 3; ++r) {
            RS(r, 2) = dz(r);
        }
        TransformationMatrix<float, double, 3> loc{
            RS,
            prev.position };
        auto op = [this](const FixedArray<float, 2>& uv){
            return FixedArray<float, 2>{ trail_sequence_.u_offset + uv(0) * trail_sequence_.u_scale, uv(1) };
        };
        if (previous_vertices_.empty()) {
            for (const auto& t0 : segment_) {
                auto t = t0.applied<ColoredVertex<double>>([&loc, &R, &op](const auto& v){
                    return v.template casted<double>().transformed(loc, R).transformed_uv(op); });
                FixedArray<float, 3> time;
                for (size_t i = 0; i < 3; ++i) {
                    if (t0(i).position(2) == -1.f) {
                        previous_vertices_[OrderableFixedArray<float, 2>{ t0(i).position(0), t0(i).position(1) }] = t(i).position;
                        time(i) = 0.f;
                    } else if (t0(i).position(2) == 1.f) {
                        time(i) = std::chrono::duration<float>(trails_instance_.time() - prev.time).count() * s;
                    } else {
                        THROW_OR_ABORT("z-position of trail object is not 1 or -1");
                    }
                }
                trails_instance_.add_triangle(t, time, trail_sequence_);
            }
        } else {
            for (const auto& t0 : segment_) {
                FixedArray<ColoredVertex<double>, 3> t;
                FixedArray<float, 3> time;
                for (size_t i = 0; i < 3; ++i) {
                    OrderableFixedArray<float, 2> k{ t0(i).position(0), t0(i).position(1) };
                    if (t0(i).position(2) == -1.f) {
                        t(i) = t0(i).casted<double>().transformed(loc, R).transformed_uv(op);
                        current_vertices_[k] = t(i).position;
                        time(i) = 0.f;
                    } else if (t0(i).position(2) == 1.f) {
                        auto it = previous_vertices_.find(k);
                        if (it == previous_vertices_.end()) {
                            THROW_OR_ABORT("TrailExtender::append_location: Could not find previous vertex");
                        }
                        t(i) = t0(i).casted<double>().transformed_uv(op);
                        t(i).position = it->second;
                        time(i) = std::chrono::duration<float>(trails_instance_.time() - prev.time).count() * s;
                    } else {
                        THROW_OR_ABORT("z-position of trail object is not 1 or -1");
                    }
                }
                trails_instance_.add_triangle(t, time, trail_sequence_);
            }
            std::swap(previous_vertices_, current_vertices_);
        }
    }
    previous_center_ = PreviousCenter{ .position = location.t(), .time = trails_instance_.time() };
}
