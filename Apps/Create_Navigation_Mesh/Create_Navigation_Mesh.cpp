#include <Mlib/Arg_Parser.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Navigation/NavigationMeshBuilder.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <chrono>
#include <iostream>

using namespace Mlib;

int main(int argc, char** argv) {
    const ArgParser parser(
        "Usage: create_navigation_mesh mesh",
        {},
        {});
    try {
        const auto args = parser.parsed(argc, argv);
        args.assert_num_unnamed(1);

        NavigationMeshBuilder nmb{
            args.unnamed_value(0),
            NavigationMeshConfig{
                .cell_size = 1.f,
                .agent_radius = 0.6f}};
        // auto a = load_obj(args.unnamed_value(0), LoadMeshConfig{
        //     .triangle_tangent_error_behavior = TriangleTangentErrorBehavior::ZERO});
        // std::list<FixedArray<ColoredVertex, 3>> triangles;
        // for (const auto& cvas : a) {
        //     for (const auto& t : cvas->triangles) {
        //         triangles.push_back(t);
        //     }
        // }
        // IndexedFaceSet<float, size_t> indexed_face_set{ triangles };
        // NavigationMeshBuilder nmb{indexed_face_set};
        
        linfo() << "Point on navmesh";
        auto start = nmb.ssm.closest_point_on_navmesh(FixedArray<float, 3>{ -1534.788086f, 159.749268f, 756.568665f });
        auto end = nmb.ssm.closest_point_on_navmesh(FixedArray<float, 3>{ -1386.703369f, 164.245132f, 734.361694f });
        if (!start.has_value()) {
            THROW_OR_ABORT("Could not localize start");
        }
        if (!end.has_value()) {
            THROW_OR_ABORT("Could not localize end");
        }
        linfo() << "Start " << start->position;
        linfo() << "End " << end->position;
        linfo() << "Shortest path";
        for (const auto& p : nmb.ssm.shortest_path(*start, *end, 2.f)) {
            linfo() << p;
        }
    } catch (const std::runtime_error& e) {
        linfo() << e.what();
        return 1;
    }
    return 0;
}
