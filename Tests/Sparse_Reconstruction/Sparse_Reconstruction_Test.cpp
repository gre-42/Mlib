#include <Mlib/Geometry/Homogeneous.hpp>
#include <Mlib/Sfm/Components/Sparse_Reconstruction.hpp>
#include <Mlib/Sfm/Frames/Camera_Frame.hpp>
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
    cfg.append_with_bundle_adjustment = false;

    MarginalizedMap<std::map<std::chrono::milliseconds, CameraFrame>> camera_frames;
    std::map<std::chrono::milliseconds, FeaturePointFrame> particles;
    std::map<size_t, std::chrono::milliseconds> bad_points;
    SparseReconstruction recon = SparseReconstruction(sc.ki, camera_frames, particles, bad_points, "TestOut", cfg);
    Array<float> image = random_array2<float>(ArrayShape{3, 256, 512}, 1);
    ArrayShape patch_center{127, 130};
    ArrayShape patch_size{6, 6};
    TraceablePatch traceable_patch{image, patch_center, patch_size};
    // y.shape == frame, ID, coordinate
    auto insert_camera = [&](size_t itime) {
        if (!calculate_camera) {
            recon.debug_set_camera_frame(
                std::chrono::milliseconds(itime),
                CameraFrame{sc.dR(0, itime), sc.dt(0, itime), CameraFrame::undefined_kep});
        }
    };
    auto insert_particles = [&](size_t itime, size_t keep) {
        milliseconds time{itime + 1};
        FeaturePointFrame fr;
        //std::cerr << sc.y.shape() << std::endl;
        assert(keep < sc.y.shape(1));
        for(size_t pt_id = 0; pt_id < sc.y.shape(1) - keep; ++pt_id) {
            std::shared_ptr<FeaturePointSequence> seq;
            auto point = std::make_shared<FeaturePoint>(dehomogenized_2(sc.y[itime][pt_id]), traceable_patch);
            if ((particles.size() > 0) &&
                (particles.rbegin()->second.find(pt_id) != particles.rbegin()->second.end())) {
                seq = particles.rbegin()->second.find(pt_id)->second;
                // std::cerr << "exist" << std::endl;
            } else {
                seq = std::make_shared<FeaturePointSequence>();
                // std::cerr << "new" << std::endl;
            }
            seq->sequence[time] = point;
            fr[pt_id] = seq;
        }
        particles.insert(std::make_pair(time, fr));
    };
    std::cerr << "1. Insertion" << std::endl;
    // (cfg.nframes == 2) => nothing happens yet.
    insert_particles(0, 5);
    recon.reconstruct();
    std::cerr << "2. Insertion" << std::endl;
    insert_particles(1, 4);
    insert_camera(0);
    insert_camera(1);
    recon.reconstruct();
    // std::cerr << "sc.R\n" << sc.R(0, 1) << std::endl;
    // std::cerr << "recon\n" << recon.reconstructed_points() << std::endl;
    // std::cerr << "sc.x\n" << sc.x << std::endl;
    // std::cerr << recon.camera_frames().rbegin()->second.rotation << std::endl;
    // std::cerr << recon.camera_frames().rbegin()->second.position << std::endl;
    assert_allclose(
        camera_frames.begin()->second.rotation,
        identity_array<float>(3));
    assert_allclose(
        camera_frames.begin()->second.position,
        zeros<float>(ArrayShape{3}));
    assert_allclose(
        camera_frames.rbegin()->second.rotation,
        sc.dR(0, 1),
        calculate_camera ? 1e-4 : 1e-6);
    assert_allclose(
        camera_frames.rbegin()->second.position,
        calculate_camera ? sc.dt2(0, 1) : sc.dt(0, 1),
        calculate_camera ? 1e-4 : 1e-6);
    {
        Array<float> recon_pts = recon.reconstructed_points();
        Array<size_t> recon_ids = recon.reconstructed_point_ids();
        assert_allclose(
            recon_pts / recon_pts(0, 0),
            sc.x[recon_ids].col_range(0, 3) / sc.x(recon_ids(0), 0),
            calculate_camera ? 1e-1 : 1e-1);
    }
    // std::cerr << "initial x\n" << recon.reconstructed_points() << std::endl;
    std::cerr << "3. Insertion" << std::endl;
    insert_particles(2, 4);
    insert_camera(2);
    recon.reconstruct();
    assert_allclose(
        camera_frames.rbegin()->second.rotation,
        sc.dR(0, 2),
        calculate_camera ? 1e-3 : 1e-6);
    assert_allclose(
        normalized_l2(camera_frames.rbegin()->second.position),
        sc.dt2(0, 2),
        calculate_camera ? 1e-2 : 1e-6);
    {
        Array<float> recon_pts = recon.reconstructed_points();
        Array<size_t> recon_ids = recon.reconstructed_point_ids();
        assert_allclose(
            recon_pts / recon_pts(0, 0),
            sc.x[recon_ids].col_range(0, 3) / sc.x(recon_ids(0), 0),
            1e-2);
    }
    std::cerr << "4. Insertion" << std::endl;
    insert_particles(3, 5);
    insert_camera(3);
    recon.reconstruct();
    assert_allclose(
        camera_frames.rbegin()->second.rotation,
        sc.dR(0, 3),
        calculate_camera ? 1e-1 : 1e-6);
    assert_allclose(
        normalized_l2(camera_frames.rbegin()->second.position),
        sc.dt2(0, 3),
        calculate_camera ? 1e-3 : 1e-6);
    {
        Array<float> recon_pts = recon.reconstructed_points();
        Array<size_t> recon_ids = recon.reconstructed_point_ids();
        assert_allclose(
            recon_pts / recon_pts(0, 0),
            sc.x[recon_ids].col_range(0, 3) / sc.x(recon_ids(0), 0),
            5e-2);
    }
    // std::cerr << "Marginalizing some points" << std::endl;
    // camera_frames.at(std::chrono::milliseconds())
    // recon.debug_marginalize_points({0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12});
    std::cerr << "5. Insertion" << std::endl;
    insert_particles(4, 5);
    insert_camera(4);
    recon.reconstruct();
    assert_allclose(
        camera_frames.rbegin()->second.rotation,
        sc.dR(0, 4),
        calculate_camera ? 3e-3 : 1e-6);
    assert_allclose(
        normalized_l2(camera_frames.rbegin()->second.position),
        sc.dt2(0, 4),
        calculate_camera ? 5e-2 : 1e-6);
    {
        Array<float> recon_pts = recon.reconstructed_points();
        Array<size_t> recon_ids = recon.reconstructed_point_ids();
        assert_allclose(
            recon_pts / recon_pts(0, 0),
            sc.x[recon_ids].col_range(0, 3) / sc.x(recon_ids(0), 0),
            calculate_camera ? 9e-2 : 1e-3);
    }
    // std::cerr << "sc.x\n" << sc.x << std::endl;
    recon.reconstruct_pass2();
}

int main(int argc, char** argv) {
    test_reconstruction();
    return 0;
}
