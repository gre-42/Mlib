#include <Mlib/Arg_Parser.hpp>
#include <Mlib/Env.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Stats/Fixed_Random_Arrays.hpp>
#include <Mlib/Strings/To_Number.hpp>
#include <ostream>
#include <stdexcept>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace Mlib;

struct Vertex {
    FixedArray<float, 3> position;
    FixedArray<float, 3> normal;
    FixedArray<float, 2> uv;
};

void draw_arrays(
    std::ostream& file,
    const std::vector<Vertex>& vertices,
    const std::vector<FixedArray<size_t, 4>>& faces,
    size_t faceOffset)
{
    for (const Vertex& v : vertices) {
        std::print(file, "v {:3f} {:3f} {:3f}\n", v.position(0), v.position(1), v.position(2));
    }
    for (const Vertex& v : vertices) {
        std::print(file, "vn {:3f} {:3f} {:3f}\n", v.normal(0), v.normal(1), v.normal(2));
    }
    for (const Vertex& v : vertices) {
        std::print(file, "vt {:3f} {:3f}\n", v.uv(0), v.uv(1));
    }
    for (const FixedArray<size_t, 4>& f : faces) {
        std::print(
            file,
            "f {}/{}/{} {}/{}/{} {}/{}/{} {}/{}/{}\n",
            f(0) + 1 + faceOffset, f(0) + 1 + faceOffset, f(0) + 1 + faceOffset,
            f(1) + 1 + faceOffset, f(1) + 1 + faceOffset, f(1) + 1 + faceOffset,
            f(2) + 1 + faceOffset, f(2) + 1 + faceOffset, f(2) + 1 + faceOffset,
            f(3) + 1 + faceOffset, f(3) + 1 + faceOffset, f(3) + 1 + faceOffset);
    }
}

struct Bush {
    std::vector<Vertex> vertices;
    std::vector<FixedArray<size_t, 4>> faces;
};

Bush generate_bush(unsigned int nplanes, unsigned int seed) {
    Bush result;
    result.vertices.reserve(4 * nplanes);
    result.faces.reserve(nplanes);
    for (unsigned int i = 0; i < nplanes; ++i) {
        FixedArray<float, 3> n = fixed_random_uniform_array<float, 3>(seed + i);
        n(2) = 0;
        n /= std::sqrt(sum(squared(n)));
        float angle = UniformRandomNumberGenerator<float>{(unsigned int)(seed + i + 1), 0.f, 2.f * float(M_PI)}();
        FixedArray<float, 3, 3> r = rodrigues2(n, angle);
        auto face = FixedArray<float, 3, 4>::init(
            -1.f, +1.f, +1.f, -1.f,
            -1.f, -1.f, +1.f, +1.f,
             0.f,  0.f,  0.f,  0.f);
        FixedArray<float, 3, 4> tf = dot2d(r, face);
        for (unsigned int v = 0; v < 4; ++v) {
            result.vertices.push_back(Vertex{
                .position = {tf(0, v), tf(1, v), tf(2, v)},
                .normal = dot1d(r, FixedArray<float, 3>{0.f, 0.f, 1.f}),
                .uv = {(face(0, v) + 1.f) * 0.5f, (face(1, v) + 1.f) * 0.5f}});
        }
        result.faces.emplace_back(
            integral_cast<size_t>(i * 4 + 0),
            integral_cast<size_t>(i * 4 + 1),
            integral_cast<size_t>(i * 4 + 2),
            integral_cast<size_t>(i * 4 + 3));
    }
    return result;
}

int main(int argc, char **argv)
{
    const ArgParser parser(
        "Usage: proc_bush --nplanes <nplanes> --seed <seed>",
        {},
        {"--nplanes", "--seed"});

    const auto args = parser.parsed(argc, argv);

    Bush bush = generate_bush(
        safe_stou(args.named_value("--nplanes")),
        safe_stou(args.named_value("--seed")));
    {
        auto file = create_ofstream("bush.obj");
        if (file->fail()) {
            verbose_abort("Could not open bush.obj for write");
        }
        std::print(*file, "mtllib bush.mtl\n");
        std::print(*file, "g Tree\n");
        std::print(*file, "o Tree\n");
        std::print(*file, "usemtl twig\n");
        draw_arrays(*file, bush.vertices, bush.faces, 0);

        file->flush();
        if (file->fail()) {
            verbose_abort("Could not write to bush.obj");
        }
    }
    {
        auto file = create_ofstream("bush.mtl");
        if (file->fail()) {
            verbose_abort("Could not open bush.mtl for write");
        }
        std::print(*file, "newmtl twig\n");
        std::print(*file, "map_Kd twig.png\n");
        std::print(*file, "map_d twig.png\n");

        file->flush();
        if (file->fail()) {
            verbose_abort("Could not write to bush.mtl");
        }
    }
}
