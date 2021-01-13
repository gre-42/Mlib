#include "Substitution_Info.hpp"
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Instance_Handles/Vertex_Array.hpp>
#include <Mlib/Render/Renderables/Renderable_Colored_Vertex_Array.hpp>

using namespace Mlib;

void SubstitutionInfo::delete_triangle(size_t id, FixedArray<ColoredVertex, 3>* ptr) {
    assert(ntriangles > 0);
    if (triangles_local_ids[id] == ntriangles - 1) {
        triangles_local_ids[id] = SIZE_MAX;
        triangles_global_ids[ntriangles - 1] = SIZE_MAX;
    } else if (ntriangles > 1) {
        size_t from = ntriangles - 1;
        size_t to = triangles_local_ids[id];
        assert(to != SIZE_MAX);
        ptr[to] = cva->triangles[triangles_global_ids[from]];
        triangles_local_ids[id] = SIZE_MAX;
        triangles_local_ids[triangles_global_ids[from]] = to;
        triangles_global_ids[to] = triangles_global_ids[from];
        triangles_global_ids[from] = SIZE_MAX;
    }
    --ntriangles;
}

void SubstitutionInfo::insert_triangle(size_t id, FixedArray<ColoredVertex, 3>* ptr) {
    assert(triangles_global_ids[ntriangles] == SIZE_MAX);
    assert(triangles_local_ids[id] == SIZE_MAX);
    triangles_local_ids[id] = ntriangles;
    triangles_global_ids[ntriangles] = id;
    ptr[ntriangles] = cva->triangles[id];
    ++ntriangles;
}

/**
 * From: https://learnopengl.com/Advanced-OpenGL/Advanced-Data
 */
void SubstitutionInfo::delete_triangles_far_away(
    const FixedArray<float, 3>& position,
    const TransformationMatrix<float>& m,
    float draw_distance_add,
    float draw_distance_slop,
    size_t noperations)
{
    if (cva->triangles.empty()) {
        return;
    }
    if (triangles_local_ids.empty()) {
        triangles_local_ids.resize(cva->triangles.size());
        triangles_global_ids.resize(cva->triangles.size());
        for (size_t i = 0; i < triangles_local_ids.size(); ++i) {
            triangles_local_ids[i] = i;
            triangles_global_ids[i] = i;
        }
        current_triangle_id = 0;
    }
    assert(triangles_local_ids.size() == cva->triangles.size());
    CHK(glBindBuffer(GL_ARRAY_BUFFER, va.vertex_buffer));
    typedef FixedArray<ColoredVertex, 3> Triangle;
    CHK(Triangle* ptr = (Triangle*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY));
    float add2 = squared(draw_distance_add);
    float remove2 = squared(draw_distance_add + draw_distance_slop);
    for (size_t i = 0; i < noperations; ++i) {
        auto center_local = mean(cva->triangles[current_triangle_id].applied<FixedArray<float, 3>>([](const ColoredVertex& c){return c.position;}));
        auto center = m * center_local;
        float dist2 = sum(squared(center - position));
        if (triangles_local_ids[current_triangle_id] != SIZE_MAX) {
            if (dist2 > remove2) {
                delete_triangle(current_triangle_id, ptr);
            }
        } else {
            if (dist2 < add2) {
                insert_triangle(current_triangle_id, ptr);
            }
        }
        current_triangle_id = (current_triangle_id + 1) % triangles_local_ids.size();
    }
    CHK(glUnmapBuffer(GL_ARRAY_BUFFER));
}
