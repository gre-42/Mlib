#include <Mlib/Geometry/Coordinates/Homogeneous.hpp>
#include <Mlib/Sfm/Components/Sparse_Reconstruction.hpp>
#include <Mlib/Sfm/Frames/Camera_Frame.hpp>
#include <Mlib/Sfm/Frames/Image_Frame.hpp>
#include <Mlib/Sfm/Rigid_Motion/Synthetic_Scene.hpp>

using namespace Mlib;
using namespace Mlib::Sfm;
using std::chrono::milliseconds;

void test_reconstruction() {
    bool calculate_camera = true;

    ReconstructionConfig cfg;
    cfg.nframes = 2;
    cfg.recompute_first_camera = false;
    cfg.recompute_second_camera = false;
    cfg.recompute_interval = 1;
    // cfg.projector_scale = 25.f;

    cfg.gm.nbundle_cameras = 3;
    cfg.gm.marginalization_target = MarginalizationTarget::POINTS;

    SyntheticScene sc(true); // true = zero_first_extrinsic

    cfg.fov_distances = FixedArray<float, 2>{0.1f, 200.f};

    cfg.ro_initial.nelems_small = sc.y.shape(1);
    cfg.ro_initial.ncalls = 1;
    cfg.ro_initial.inlier_distance_thresh = squared(INFINITY);
    cfg.ro_initial.inlier_count_thresh = 5;
    cfg.ro_initial.seed = 1;

    cfg.ro_append.nelems_small = sc.y.shape(1);
    cfg.ro_append.ncalls = 1;
    cfg.ro_append.inlier_distance_thresh = squared(INFINITY);
    cfg.ro_append.inlier_count_thresh = 5;
    cfg.ro_append.seed = 1;

    cfg.initialize_with_bundle_adjustment = false;
    cfg.interpolate_initial_cameras = false;
    cfg.append_mode = AppendMode::PROJECTION;

    std::map<std::chrono::milliseconds, ImageFrame> image_frames;
    MarginalizedMap<std::map<std::chrono::milliseconds, CameraFrame>> camera_frames;
    std::map<std::chrono::milliseconds, FeaturePointFrame> particles;
    std::map<size_t, std::chrono::milliseconds> bad_points;
    std::map<size_t, float> last_sq_residual;
    SparseReconstruction recon{ sc.ki, camera_frames, image_frames, particles, bad_points, last_sq_residual, "TestOut", cfg };
    Array<float> image = random_array2<float>(ArrayShape{3, 256, 512}, 1);
    FixedArray<size_t, 2> patch_center{127u, 130u};
    FixedArray<size_t, 2> patch_size{6u, 6u};
    TraceablePatch traceable_patch{image, patch_center, patch_size};
    // y.shape == frame, ID, coordinate
    auto insert_camera = [&](size_t itime) {
        if (!calculate_camera) {
            recon.set_camera_frame(
                std::chrono::milliseconds(itime),
                CameraFrame{ sc.delta_ke(0, itime) });
        }
    };
    auto insert_particles = [&](size_t itime, size_t keep) {
        milliseconds time{itime + 1};
        FeaturePointFrame fr;
        //lerr() << sc.y.shape();
        assert(keep < sc.y.shape(1));
        for (size_t pt_id = 0; pt_id < sc.y.shape(1) - keep; ++pt_id) {
            std::shared_ptr<FeaturePointSequence> seq;
            auto point = std::make_shared<FeaturePoint>(sc.y(itime, pt_id), traceable_patch, TraceableDescriptor{ Array<float>() });
            if ((particles.size() > 0) &&
                (particles.rbegin()->second.tracked_points.find(pt_id) != particles.rbegin()->second.tracked_points.end())) {
                seq = particles.rbegin()->second.tracked_points.find(pt_id)->second;
                // lerr() << "exist";
            } else {
                seq = std::make_shared<FeaturePointSequence>();
                // lerr() << "new";
            }
            seq->sequence[time] = point;
            fr.tracked_points[pt_id] = seq;
        }
        particles.insert(std::make_pair(time, fr));
    };
    lerr() << "1. Insertion";
    // (cfg.nframes == 2) => nothing happens yet.
    insert_particles(0, 5);
    recon.reconstruct();
    assert_isequal(camera_frames.size(), (size_t)0);
    lerr() << "2. Insertion";
    insert_particles(1, 4);
    insert_camera(0);
    insert_camera(1);
    recon.reconstruct();
    assert_isequal(camera_frames.size(), (size_t)2);
    // lerr() << "sc.R\n" << sc.R(0, 1);
    // lerr() << "recon\n" << recon.reconstructed_points();
    // lerr() << "sc.x\n" << sc.x;
    // lerr() << recon.camera_frames().rbegin()->second.rotation;
    // lerr() << recon.camera_frames().rbegin()->second.position;
    assert_allclose(
        camera_frames.begin()->second.pose.R.to_array(),
        identity_array<float>(3));
    assert_allclose(
        camera_frames.begin()->second.pose.t.to_array(),
        zeros<float>(ArrayShape{3}));
    assert_allclose(
        camera_frames.rbegin()->second.pose.R.to_array(),
        sc.dR(0, 1).to_array(),
        calculate_camera ? float{ 1e-4 } : float{ 1e-6 });
    assert_allclose(
        camera_frames.rbegin()->second.pose.t.to_array(),
        calculate_camera ? sc.dt2(0, 1).to_array() : sc.dt(0, 1).to_array(),
        calculate_camera ? float{ 1e-4 } : float { 1e-6 });
    {
        Array<FixedArray<float, 3>> recon_pts = recon.reconstructed_points();
        Array<size_t> recon_ids = recon.reconstructed_point_ids();
        assert_allclose(
            Array<float>{ recon_pts } / recon_pts(0)(0),
            Array<float>{ sc.x[recon_ids] } / sc.x(recon_ids(0))(0),
            calculate_camera ? float{ 1e-1 } : float{ 1e-1 });
    }
    // lerr() << "initial x\n" << recon.reconstructed_points();
    lerr() << "3. Insertion";
    insert_particles(2, 4);
    insert_camera(2);
    recon.reconstruct();
    assert_isequal(camera_frames.size(), (size_t)3);
    assert_allclose(
        camera_frames.rbegin()->second.pose.R.to_array(),
        sc.dR(0, 2).to_array(),
        calculate_camera ? float{ 1e-3 } : float{ 1e-6 });
    assert_allclose(
        normalized_l2(camera_frames.rbegin()->second.pose.t.to_array()),
        sc.dt2(0, 2).to_array(),
        calculate_camera ? float{ 1e-2 } : float{ 1e-6 });
    {
        Array<FixedArray<float, 3>> recon_pts = recon.reconstructed_points();
        Array<size_t> recon_ids = recon.reconstructed_point_ids();
        assert_allclose(
            Array<float>{ recon_pts } / recon_pts(0)(0),
            Array<float>{ sc.x[recon_ids] } / sc.x(recon_ids(0))(0),
            float{ 1e-2 });
    }
    lerr() << "4. Insertion";
    insert_particles(3, 5);
    insert_camera(3);
    recon.reconstruct();
    assert_isequal(camera_frames.size(), (size_t)4);
    assert_allclose(
        camera_frames.rbegin()->second.pose.R.to_array(),
        sc.dR(0, 3).to_array(),
        calculate_camera ? float{ 1e-1 } : float{ 1e-6 });
    assert_allclose(
        normalized_l2(camera_frames.rbegin()->second.pose.t.to_array()),
        sc.dt2(0, 3).to_array(),
        calculate_camera ? float{ 1e-3 } : float{ 1e-6 });
    {
        Array<FixedArray<float, 3>> recon_pts = recon.reconstructed_points();
        Array<size_t> recon_ids = recon.reconstructed_point_ids();
        assert_allclose(
            Array<float>{ recon_pts } / recon_pts(0)(0),
            Array<float>{ sc.x[recon_ids] } / sc.x(recon_ids(0))(0),
            float{ 5e-2 });
    }
    // lerr() << "Marginalizing some points";
    // camera_frames.at(std::chrono::milliseconds())
    // recon.debug_marginalize_points({0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12});
    lerr() << "5. Insertion";
    insert_particles(4, 5);
    insert_camera(4);
    recon.reconstruct();
    assert_isequal(camera_frames.size(), (size_t)5);
    assert_allclose(
        camera_frames.rbegin()->second.pose.R.to_array(),
        sc.dR(0, 4).to_array(),
        calculate_camera ? float{ 3e-3 } : float{ 1e-6 });
    assert_allclose(
        normalized_l2(camera_frames.rbegin()->second.pose.t.to_array()),
        sc.dt2(0, 4).to_array(),
        calculate_camera ? float{ 5e-2 } : float{ 1e-6 });
    {
        Array<FixedArray<float, 3>> recon_pts = recon.reconstructed_points();
        Array<size_t> recon_ids = recon.reconstructed_point_ids();
        assert_allclose(
            Array<float>{ recon_pts } / recon_pts(0)(0),
            Array<float>{ sc.x[recon_ids] } / sc.x(recon_ids(0))(0),
            calculate_camera ? float{ 9e-2 } : float{ 1e-3 });
    }
    // lerr() << "sc.x\n" << sc.x;
    recon.reconstruct_pass2();
}

int main(int argc, char** argv) {
    try {
        test_reconstruction();
    } catch (const std::runtime_error& e) {
        lerr() << e.what();
        return 1;
    }
    return 0;
}
