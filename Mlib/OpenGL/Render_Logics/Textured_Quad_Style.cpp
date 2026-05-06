#include "Textured_Quad_Style.hpp"

using namespace Mlib;

static const float standard_quad_vertices_values[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
    // positions   // tex_coords
    -1.0f,  1.0f,  0.0f, 1.0f,
    -1.0f, -1.0f,  0.0f, 0.0f,
    1.0f, -1.0f,  1.0f, 0.0f,

    -1.0f,  1.0f,  0.0f, 1.0f,
    1.0f, -1.0f,  1.0f, 0.0f,
    1.0f,  1.0f,  1.0f, 1.0f
};

static const float horizontally_flipped_quad_vertices_values[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
    // positions   // tex_coords
    -1.0f,  1.0f,  1.0f, 1.0f,
    -1.0f, -1.0f,  1.0f, 0.0f,
    1.0f, -1.0f,  0.0f, 0.0f,

    -1.0f,  1.0f,  1.0f, 1.0f,
    1.0f, -1.0f,  0.0f, 0.0f,
    1.0f,  1.0f,  0.0f, 1.0f
};

static const float vertically_flipped_quad_vertices_values[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
    // positions   // tex_coords
    -1.0f,  1.0f,  0.0f, 0.0f,
    -1.0f, -1.0f,  0.0f, 1.0f,
    1.0f, -1.0f,  1.0f, 1.0f,

    -1.0f,  1.0f,  0.0f, 0.0f,
    1.0f, -1.0f,  1.0f, 1.0f,
    1.0f,  1.0f,  1.0f, 0.0f
};

const float* Mlib::standard_quad_vertices = standard_quad_vertices_values;
const float* Mlib::horizontally_flipped_quad_vertices = horizontally_flipped_quad_vertices_values;
const float* Mlib::vertically_flipped_quad_vertices = vertically_flipped_quad_vertices_values;
