#include <Mlib/Arg_Parser.hpp>
#include <Mlib/Env.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/String.cpp>
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
    FILE * pFile,
    const std::vector<Vertex>& vertices,
    const std::vector<FixedArray<size_t, 4>>& faces,
    int faceOffset)
{
    for (const Vertex& v : vertices) {
        int res = fprintf(pFile, "v %+3.3f %+3.3f %+3.3f\n", v.position(0), v.position(1), v.position(2));
        if (res < 0) {
            std::runtime_error(strerror(errno));
        }
    }
    for (const Vertex& v : vertices) {
        int res = fprintf(pFile, "vn %+3.3f %+3.3f %+3.3f\n", v.normal(0), v.normal(1), v.normal(2));
        if (res < 0) {
            std::runtime_error(strerror(errno));
        }
    }
    for (const Vertex& v : vertices) {
        int res = fprintf(pFile, "vt %+3.3f %+3.3f\n", v.uv(0), v.uv(1));
        if (res < 0) {
            std::runtime_error(strerror(errno));
        }
    }
    for(const FixedArray<size_t, 4>& f : faces) {
        int res = fprintf(
			pFile,
			"f %zu/%zu/%zu %zu/%zu/%zu %zu/%zu/%zu %zu/%zu/%zu\n",
			f(0) + 1 + faceOffset, f(0) + 1 + faceOffset, f(0) + 1 + faceOffset,
			f(1) + 1 + faceOffset, f(1) + 1 + faceOffset, f(1) + 1 + faceOffset,
			f(2) + 1 + faceOffset, f(2) + 1 + faceOffset, f(2) + 1 + faceOffset,
            f(3) + 1 + faceOffset, f(3) + 1 + faceOffset, f(3) + 1 + faceOffset);
        if (res < 0) {
            std::runtime_error(strerror(errno));
        }
    }
}

struct Bush {
    std::vector<Vertex> vertices;
    std::vector<FixedArray<size_t, 4>> faces;
};

Bush generate_bush(size_t nplanes, unsigned int seed) {
    Bush result;
    result.vertices.reserve(4 * nplanes);
    result.faces.reserve(nplanes);
    for(size_t i = 0; i < nplanes; ++i) {
        FixedArray<float, 3> n = fixed_random_array4<float, 3>(seed + i);
        n /= std::sqrt(sum(squared(n)));
        float angle = UniformRandomNumberGenerator<float>{(unsigned int)(seed + i + 1), 0, 2 * M_PI}();
        FixedArray<float, 3, 3> r = rodrigues(n, angle);
        FixedArray<float, 3, 4> face{
            -1, +1, +1, -1,
            -1, -1, +1, +1,
            0, 0, 0, 0};
        FixedArray<float, 3, 4> tf = dot2d(r, face);
        for(size_t v = 0; v < 4; ++v) {
            result.vertices.push_back(Vertex{
                position: {tf(0, v), tf(1, v), tf(2, v)},
                normal: dot1d(r, FixedArray<float, 3>{0, 0, 1}),
                uv: {(face(0, v) + 1.f) * 0.5f, (face(1, v) + 1.f) * 0.5f}});
        }
        result.faces.push_back({
            i * 4 + 0,
            i * 4 + 1,
            i * 4 + 2,
            i * 4 + 3});
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
        safe_stoi(args.named_value("--nplanes")),
        safe_stoi(args.named_value("--seed")));
    {
        FILE * pFile = fopen("bush.obj", "w");
        if (pFile == nullptr) {
            throw std::runtime_error("Could not open bush.obj for write");
        }
        fprintf(pFile, "mtllib bush.mtl\n");
        fprintf(pFile, "g Tree\n");
        fprintf(pFile, "o Tree\n");
        fprintf(pFile, "usemtl twig\n");
        draw_arrays(pFile, bush.vertices, bush.faces, 0);

        if (fclose(pFile) < 0) {
            throw std::runtime_error("Could not close file");
        }
    }
    {
        FILE * pFile = fopen("bush.mtl", "w");
        if (pFile == nullptr) {
            throw std::runtime_error("Could not open bush.mtl for write");
        }
        fprintf(pFile, "newmtl twig\n");
        fprintf(pFile, "map_Kd twig.png\n");
        fprintf(pFile, "map_d twig.png\n");

        if (fclose(pFile) < 0) {
            throw std::runtime_error("Could not close file");
        }
    }
}
