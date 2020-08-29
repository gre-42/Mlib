#pragma once
#include <Mlib/Array/Sparse_Array.hpp>
#include <Mlib/Sfm/Marginalization/UUID.hpp>
#include <chrono>

namespace Mlib { namespace Sfm { namespace SynthMarg {

class CameraVariable {
public:
    explicit CameraVariable(const std::chrono::milliseconds& time);
    bool operator < (const CameraVariable& ci) const;
    std::chrono::milliseconds time_;
};

class FeaturePointVariable {
public:
    explicit FeaturePointVariable(size_t feature_point_id);
    bool operator < (const FeaturePointVariable& fi) const;
    size_t feature_point_id_;
};

typedef UUIDGen<CameraVariable, FeaturePointVariable> XUUIDGen;

class TestScene1d {
public:
    explicit TestScene1d(XUUIDGen& uuid_gen);
    void add_feature_point(float position);
    void add_camera(float position);
    void add_observation(size_t feature_point_index, float error);
    void delete_camera(const std::chrono::milliseconds& time);
    void delete_feature_point(size_t index);
    XUUIDGen& uuid_gen_;
    std::map<size_t, float> feature_points_;
    std::map<std::chrono::milliseconds, float> cameras_;
    std::map<std::chrono::milliseconds, std::map<size_t, float>> observations_;
};

class Scene1dMatrix {
public:
    explicit Scene1dMatrix(const TestScene1d& scene);
    size_t row_id_observation(const std::chrono::milliseconds& observation_time, size_t feature_point_index) const;
    size_t column_id_camera(const CameraVariable& camera_index) const;
    size_t column_id_feature_point(const FeaturePointVariable& feature_point_index) const;
    size_t nrows() const;
    size_t ncols() const;
    SparseArrayCcs<float> jacobian() const;
    Array<float> rhs() const;
    std::map<UUID, size_t> predictor_uuids() const;
    void print_x(const Array<float>& x, bool correct=true) const;
    void print_uuids() const;
private:
    std::map<std::pair<std::chrono::milliseconds, size_t>, size_t> observation_row_id_;
    std::map<CameraVariable, size_t> cameras_;
    std::map<FeaturePointVariable, size_t> feature_points_;
    const std::map<std::chrono::milliseconds, std::map<size_t, float>>& observations_;
    const XUUIDGen& uuid_gen_;
};

void gen_scene0(TestScene1d& tc);
void gen_scene1(TestScene1d& tc);

}}}
