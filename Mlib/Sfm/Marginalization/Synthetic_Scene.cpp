#include "Synthetic_Scene.hpp"
#include <Mlib/Array/Sparse_Array.hpp>
#include <Mlib/Sfm/Marginalization/UUID.hpp>
#include <Mlib/Stats/Random_Number_Generators.hpp>
#include <chrono>

using namespace Mlib;
using namespace Mlib::Sfm;
using namespace Mlib::Sfm::SynthMarg;

static float intrinsic_matrix = 2;
static float extrinsic_matrix = 3;
static float error = 1e-1f;

CameraVariable::CameraVariable(const std::chrono::milliseconds& time)
: time_(time)
{}
bool CameraVariable::operator < (const CameraVariable& ci) const {
    return time_ < ci.time_;
}

FeaturePointVariable::FeaturePointVariable(size_t feature_point_id)
: feature_point_id_(feature_point_id)
{}

bool FeaturePointVariable::operator < (const FeaturePointVariable& fi) const {
    return feature_point_id_ < fi.feature_point_id_;
}

TestScene1d::TestScene1d(XUUIDGen& uuid_gen)
: uuid_gen_(uuid_gen) {}
void TestScene1d::add_feature_point(float position) {
    size_t feature_point_id = feature_points_.size();
    feature_points_[feature_point_id] = position;
    uuid_gen_.generate(FeaturePointVariable{feature_point_id});
}
void TestScene1d::add_camera(float position) {
    std::chrono::milliseconds time(cameras_.size());
    cameras_[time] = position;
    observations_[time];
    uuid_gen_.generate(CameraVariable{time});
}
void TestScene1d::add_observation(size_t feature_point_index, float error) {
    auto& o = *observations_.rbegin();
    auto& p = feature_points_.at(feature_point_index);
    o.second[feature_point_index] = error + intrinsic_matrix * (extrinsic_matrix + p - cameras_.rbegin()->second);
}
void TestScene1d::delete_camera(const std::chrono::milliseconds& time) {
    cameras_.erase(time);
}
void TestScene1d::delete_feature_point(size_t index) {
    feature_points_.erase(index);
}

Scene1dMatrix::Scene1dMatrix(const TestScene1d& scene)
: observations_(scene.observations_),
    uuid_gen_(scene.uuid_gen_)
{
    for (const auto& c : scene.cameras_) {
        cameras_.insert(std::make_pair(CameraVariable(c.first), cameras_.size()));
    }
    for (const auto& f : scene.feature_points_) {
        feature_points_.insert(std::make_pair(FeaturePointVariable(f.first), feature_points_.size()));
    }
    for (const auto& o : scene.observations_) {
        for (const auto& oo : o.second) {
            observation_row_id_.insert(std::make_pair(PointObservation{ o.first, oo.first }, observation_row_id_.size()));
        }
    }
}
size_t Scene1dMatrix::row_id_observation(const std::chrono::milliseconds& observation_time, size_t feature_point_index) const {
    return observation_row_id_.at(PointObservation{ observation_time, feature_point_index });
}
size_t Scene1dMatrix::column_id_camera(const CameraVariable& camera_index) const {
    return cameras_.at(camera_index);
}
size_t Scene1dMatrix::column_id_feature_point(const FeaturePointVariable& feature_point_index) const {
    return cameras_.size() + feature_points_.at(feature_point_index);
}
size_t Scene1dMatrix::nrows() const {
    return observation_row_id_.size();
}
size_t Scene1dMatrix::ncols() const {
    return cameras_.size() + feature_points_.size();
}
SparseArrayCcs<float> Scene1dMatrix::jacobian() const {
    SparseArrayCcs<float> res{ArrayShape{nrows(), ncols()}};
    for (const auto& o : observations_) {
        for (const auto& p : o.second) {
            res(row_id_observation(o.first, p.first), column_id_camera(CameraVariable(o.first))) = -intrinsic_matrix;
            if (feature_points_.find(FeaturePointVariable(p.first)) != feature_points_.end()) {
                res(row_id_observation(o.first, p.first), column_id_feature_point(FeaturePointVariable(p.first))) = intrinsic_matrix;
            }
        }
    }
    return res;
}
Array<float> Scene1dMatrix::rhs() const {
    Array<float> res{ArrayShape{nrows()}};
    for (const auto& o : observations_) {
        for (const auto& p : o.second) {
            res(row_id_observation(o.first, p.first)) = p.second;
        }
    }
    return res;
}
std::map<UUID, size_t> Scene1dMatrix::predictor_uuids() const {
    std::map<UUID, size_t> res;
    for (const auto& c : cameras_) {
        res.insert(std::make_pair(uuid_gen_.get(c.first), column_id_camera(c.first)));
    }
    for (const auto& f : feature_points_) {
        res.insert(std::make_pair(uuid_gen_.get(f.first), column_id_feature_point(f.first)));
    }
    return res;
}
void Scene1dMatrix::print_x(const Array<float>& x, bool correct) const {
    Array<float> xc = correct ? x - x(0) : x;
    for (const auto& c : cameras_) {
        lerr() << "c: " << c.first.time_.count() << " ms, translation: " << xc(column_id_camera(c.first));
    }
    for (const auto& p : feature_points_) {
        lerr() << "p: " << p.first.feature_point_id_ << ", " << xc(column_id_feature_point(p.first));
    }
}
void Scene1dMatrix::print_uuids() const {
    //for (const auto& v : predictor_uuids()) {
    //    lerr() << v.first << ": " << v.second;
    //}
    for (const auto& c : cameras_) {
        lerr() << "c: " << c.first.time_.count() << " ms, uuid: " << uuid_gen_.get(c.first) << ", column " << column_id_camera(c.first);
    }
    for (const auto& p : feature_points_) {
        lerr() << "p: " << p.first.feature_point_id_ << ",    uuid: " << uuid_gen_.get(p.first) << ", column " << column_id_feature_point(p.first);
    }
}

void Mlib::Sfm::SynthMarg::gen_scene0(TestScene1d& tc) {
    NormalRandomNumberGenerator<float> e{0};
    tc.add_feature_point(1); // 0
    tc.add_feature_point(3); // 1

    tc.add_camera(2); // 0 ms
    tc.add_observation(0, e() * error);

    tc.add_camera(4); // 1 ms
    tc.add_observation(0, e() * error);
    tc.add_observation(1, e() * error);
}

void Mlib::Sfm::SynthMarg::gen_scene1(TestScene1d& tc) {
    NormalRandomNumberGenerator<float> e{0};
    tc.add_feature_point(2); // 0
    tc.add_feature_point(3); // 1
    tc.add_feature_point(1); // 2
    tc.add_feature_point(4); // 3

    tc.add_camera(2); // 0 ms
    tc.add_observation(0, e() * error);
    tc.add_observation(1, e() * error);

    tc.add_camera(4); // 1 ms
    tc.add_observation(0, e() * error);
    tc.add_observation(1, e() * error);

    tc.add_camera(3); // 2 ms
    tc.add_observation(1, e() * error);
    tc.add_observation(2, e() * error);

    tc.add_camera(5); // 3 ms
    tc.add_observation(2, e() * error);
    tc.add_observation(3, e() * error);

    tc.add_camera(6); // 4 ms
    tc.add_observation(2, e() * error);
    tc.add_observation(3, e() * error);
    tc.add_observation(1, e() * error);
}
