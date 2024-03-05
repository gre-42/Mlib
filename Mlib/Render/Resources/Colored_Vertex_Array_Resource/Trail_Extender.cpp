#include "Trail_Extender.hpp"
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Math/Orderable_Fixed_Array.hpp>
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
    , minimum_length_{ minimum_length }
{
    if (segment.empty()) {
        THROW_OR_ABORT("Trail segment is empty");
    }
}

void TrailExtender::append_location(const TransformationMatrix<float, double, 3>& location) {
    if (!previous_center_.has_value()) {
        previous_center_ = PreviousCenter{ .position = location.t(), .time = trails_instance_.time() };
        return;
    }
    auto& prev = previous_center_.value();
    auto dz = (location.t() - prev.position).casted<float>();
    auto dz_l2 = sum(squared(dz));
    if (dz_l2 < squared(minimum_length_)) {
        return;
    }
    dz /= std::sqrt(dz_l2);
    auto dy = cross(dz, location.R().column(0));
    auto dy_l2 = sum(squared(dy));
    if (dy_l2 < 1e-12) {
        return;
    }
    dy /= std::sqrt(dy_l2);
    auto dx = cross(dy, dz);
    TransformationMatrix<float, double, 3> loc{
        FixedArray<float, 3, 3>::init(
            dx(0), dy(0), dz(0),
            dx(1), dy(1), dz(1),
            dx(2), dy(2), dz(2)),
        location.t() };
    auto op = [this](const FixedArray<float, 2>& uv){
        return FixedArray<float, 2>{ trail_sequence_.u_offset + uv(0) * trail_sequence_.u_scale, uv(1) };
    };
    if (previous_vertices_.empty()) {
        for (const auto& t0 : segment_) {
            auto t = t0.applied<ColoredVertex<double>>([&loc, &op](const auto& v){
                return v.template casted<double>().transformed(loc, loc.R()).transformed_uv(op); });
            FixedArray<float, 3> time;
            for (size_t i = 0; i < 3; ++i) {
                if (t0(i).position(2) == -1.f) {
                    previous_vertices_[OrderableFixedArray<float, 2>{ t0(i).position(0), t0(i).position(1) }] = t(i).position;
                    time(i) = float(trails_instance_.time() - prev.time);
                } else if (t0(i).position(2) == 1.f) {
                    time(i) = 0.f;
                } else {
                    THROW_OR_ABORT("z-position of trail object is not 1 or -1");
                }
            }
            trails_instance_.add_triangle(t, time, trail_sequence_);
        }
    }
    for (const auto& t0 : segment_) {
        FixedArray<ColoredVertex<double>, 3> t;
        FixedArray<float, 3> time;
        for (size_t i = 0; i < 3; ++i) {
            OrderableFixedArray<float, 2> k{ t0(i).position(0), t0(i).position(1) };
            if (t0(i).position(2) == -1.f) {
                t(i) = t0(i).casted<double>().transformed(loc, loc.R()).transformed_uv(op);
                previous_vertices_[k] = t(i).position;
                time(i) = float(trails_instance_.time() - prev.time);
            } else if (t0(i).position(2) == 1.f) {
                auto it = previous_vertices_.find(k);
                if (it == previous_vertices_.end()) {
                    THROW_OR_ABORT("TrailExtender::append_location: Could not find previous vertex");
                }
                t(i) = t0(i).casted<double>().transformed_uv(op);
                t(i).position = it->second;
                time(i) = 0.f;
            } else {
                THROW_OR_ABORT("z-position of trail object is not 1 or -1");
            }
        }
        trails_instance_.add_triangle(t, time, trail_sequence_);
    }
}
