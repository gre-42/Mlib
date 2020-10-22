#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "Render.hpp"
#include <Mlib/Geometry/Homogeneous.hpp>
#include <Mlib/Images/Coordinates_Fixed.hpp>
#include <Mlib/Images/Revert_Axis.hpp>
#include <Mlib/Images/Vectorial_Pixels.hpp>
#include <Mlib/Math/Fixed_Cholesky.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/linmath.hpp>
#include <fenv.h>
#include <iostream>

using namespace Mlib;

static const char* vertex_shader_text =
"#version 330\n"
"uniform mat4 MVP;\n"
"attribute vec3 vCol;\n"
"attribute vec3 vPos;\n"
"varying vec3 color;\n"
"void main()\n"
"{\n"
"    gl_Position = MVP * vec4(vPos, 1.0);\n"
"    color = vCol;\n"
"}\n";

static const char* fragment_shader_text =
"#version 330\n"
"varying vec3 color;\n"
"out vec4 frag_color;\n"
"void main()\n"
"{\n"
"    frag_color = vec4(color, 1.0);\n"
"}\n";

static void error_callback(int error, const char* description)
{
    std::cerr << "Error: " << description << std::endl;
}

class UserClass {
public:
    float camera_z = -5;
    float angle_x = 0;
    float angle_y = 0;
};

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    UserClass* user_object = (UserClass*)glfwGetWindowUserPointer(window);
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
    if (action == GLFW_REPEAT || action == GLFW_PRESS) {
        if (key == GLFW_KEY_UP) {
            user_object->camera_z += 0.04;
        }
        if (key == GLFW_KEY_DOWN) {
            user_object->camera_z -= 0.04;
        }
        if (key == GLFW_KEY_LEFT) {
            user_object->angle_y += 0.04;
        }
        if (key == GLFW_KEY_RIGHT) {
            user_object->angle_y -= 0.04;
        }
        if (key == GLFW_KEY_PAGE_UP) {
            user_object->angle_x += 0.04;
        }
        if (key == GLFW_KEY_PAGE_DOWN) {
            user_object->angle_x -= 0.04;
        }
    }
}

/*static void print_mat4x4(const mat4x4& m) {
    for(size_t r = 0; r < 4; ++r) {
        for(size_t c = 0; c < 4; ++c) {
            std::cerr << m[r][c] << " ";
        }
        std::cerr << std::endl;
    }
}*/

void Mlib::render(const std::vector<ColoredVertex>& vertices, bool rotate, Array<float>* output)
{
    GLuint vertex_buffer, vertex_array, vertex_shader, fragment_shader, program;
    GLint mvp_location, vpos_location, vcol_location;

    UserClass user_object;

    glfwSetErrorCallback(error_callback);

    if (!glfwInit()) {
        throw std::runtime_error("glfwInit failed");
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    if (output != nullptr) {
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    }

#ifndef WIN32
    int fpeflags = fegetexcept();
    fedisableexcept(FE_ALL_EXCEPT);
#endif
    GLFWwindow* window = glfwCreateWindow(640, 480, "Simple example", NULL, NULL);
#ifndef WIN32
    feenableexcept(fpeflags);
#endif
    if (!window)
    {
        glfwTerminate();
        throw std::runtime_error("Could not initialize window");
    }
    glfwSetWindowUserPointer(window, &user_object);

    glfwSetKeyCallback(window, key_callback);

    glfwMakeContextCurrent(window);
    CHK(int version = gladLoadGL(glfwGetProcAddress));
    if (version == 0) {
        throw std::runtime_error("gladLoadGL failed");
    }
    GLFW_CHK(glfwSwapInterval(1));

    // NOTE: OpenGL error checks have been omitted for brevity

    // https://stackoverflow.com/a/13405205/2292832
    CHK(glGenVertexArrays(1, &vertex_array));
    CHK(glBindVertexArray(vertex_array));

    CHK(glGenBuffers(1, &vertex_buffer));
    CHK(glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer));
    CHK(glBufferData(GL_ARRAY_BUFFER, sizeof(ColoredVertex) * vertices.size(), &*vertices.begin(), GL_STATIC_DRAW));

    CHK(vertex_shader = glCreateShader(GL_VERTEX_SHADER));
    CHK(glShaderSource(vertex_shader, 1, &vertex_shader_text, NULL));
    checked_glCompileShader(vertex_shader);

    CHK(fragment_shader = glCreateShader(GL_FRAGMENT_SHADER));
    CHK(glShaderSource(fragment_shader, 1, &fragment_shader_text, NULL));
    checked_glCompileShader(fragment_shader);

    CHK(program = glCreateProgram());
    CHK(glAttachShader(program, vertex_shader));
    CHK(glAttachShader(program, fragment_shader));
    CHK(glLinkProgram(program));

    mvp_location = checked_glGetUniformLocation(program, "MVP");
    CHK(vpos_location = glGetAttribLocation(program, "vPos"));
    CHK(vcol_location = glGetAttribLocation(program, "vCol"));
    if (mvp_location == -1 || vpos_location == -1 || vcol_location == -1) {
        throw std::runtime_error("Unknown OpenGL variable");
    }

    CHK(glEnableVertexAttribArray(vpos_location));
    CHK(glVertexAttribPointer(vpos_location, 3, GL_FLOAT, GL_FALSE,
                              sizeof(vertices[0]), (void*) 0));
    CHK(glEnableVertexAttribArray(vcol_location));
    CHK(glVertexAttribPointer(vcol_location, 3, GL_FLOAT, GL_FALSE,
                              sizeof(vertices[0]), (void*) (sizeof(float) * 3)));

    CHK(glEnable(GL_CULL_FACE));
    CHK(glEnable(GL_DEPTH_TEST));

    while (!glfwWindowShouldClose(window))
    {
        float ratio;
        int width, height;
        mat4x4 m, v, p, mvp;

        GLFW_CHK(glfwGetFramebufferSize(window, &width, &height));
        ratio = width / (float) height;

        CHK(glViewport(0, 0, width, height));
        CHK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

        mat4x4_identity(m);
        mat4x4_rotate_Y(m, m, rotate ? (float)glfwGetTime() : user_object.angle_y);
        mat4x4_rotate_X(m, m, user_object.angle_x);
        mat4x4_translate(v, 0, 0, user_object.camera_z);
        //mat4x4_ortho(p, -ratio, ratio, -1.f, 1.f, 1.f, 10.f);
        //mat4x4_frustum(p, -ratio / 10, ratio / 10, -1.f / 10, 1.f / 10, 2.f, 10.f);
        mat4x4_perspective(p, 0.1, ratio, 1.f, 100.f);

        mat4x4_mul(mvp, p, v);
        mat4x4_mul(mvp, mvp, m);

        CHK(glUseProgram(program));
        CHK(glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*) mvp));
        CHK(glDrawArrays(GL_TRIANGLES, 0, vertices.size()));

        if(output != nullptr) {
            VectorialPixels<float, 3> vp{ArrayShape{size_t(height), size_t(width)}};
            CHK(glReadPixels(0, 0, width, height, GL_RGB, GL_FLOAT, vp->flat_iterable().begin()));
            glfwSetWindowShouldClose(window, GLFW_TRUE);
            *output = reverted_axis(vp.to_array(), 1);
        }

        GLFW_CHK(glfwSwapBuffers(window));
        GLFW_CHK(glfwPollEvents());
    }

    GLFW_CHK(glfwDestroyWindow(window));

    glfwTerminate();
}

void Mlib::render_depth_map(
    const Array<float>& rgb_picture,
    const Array<float>& depth_picture,
    const FixedArray<float, 3, 3>& intrinsic_matrix,
    float z_offset,
    bool rotate,
    Array<float>* output)
{
    FixedArray<float, 3, 3> iim{inv(intrinsic_matrix)};
    std::vector<ColoredVertex> vertices;
    vertices.reserve(3 * rgb_picture.nelements());
    assert(rgb_picture.ndim() == 3);
    assert(rgb_picture.shape(0) == 3);
    Array<float> R = rgb_picture[0];
    Array<float> G = rgb_picture[1];
    Array<float> B = rgb_picture[2];
    const Array<float>& Z = depth_picture;
    for(size_t r = 0; r < rgb_picture.shape(1) - 1; ++r) {
        for(size_t c = 0; c < rgb_picture.shape(2) - 1; ++c) {
            if (std::isnan(Z(r, c)) ||
                std::isnan(Z(r, c + 1)) ||
                std::isnan(Z(r + 1, c)) ||
                std::isnan(Z(r + 1, c + 1)))
            {
                continue;
            }
            FixedArray<size_t, 2> id0{r, c};
            FixedArray<size_t, 2> id1{r + 1, c + 1};
            FixedArray<float, 3> pos0 = dot1d(iim, homogenized_3(i2a(id0)));
            FixedArray<float, 3> pos1 = dot1d(iim, homogenized_3(i2a(id1)));
            pos0 /= pos0(2);
            pos1 /= pos1(2);
            ColoredVertex v00{
                FixedArray<float, 3>{
                    pos0(0) * Z(r, c),
                    -pos0(1) * Z(r, c),
                    -Z(r, c) + z_offset},
                FixedArray<float, 3>{
                    R(r, c),
                    G(r, c),
                    B(r, c)}};
            ColoredVertex v01{
                FixedArray<float, 3>{
                    pos1(0) * Z(r, c + 1),
                    -pos0(1) * Z(r, c + 1),
                    -Z(r, c + 1) + z_offset},
                FixedArray<float, 3>{
                    R(r, c + 1),
                    G(r, c + 1),
                    B(r, c + 1)}};
            ColoredVertex v10{
                FixedArray<float, 3>{
                    pos0(0) * Z(r + 1, c),
                    -pos1(1) * Z(r + 1, c),
                    -Z(r + 1, c) + z_offset},
                FixedArray<float, 3>{
                    R(r + 1, c),
                    G(r + 1, c),
                    B(r + 1, c)}};
            ColoredVertex v11{
                FixedArray<float, 3>{
                    pos1(0) * Z(r + 1, c + 1),
                    -pos1(1) * Z(r + 1, c + 1),
                    -Z(r + 1, c + 1) + z_offset},
                FixedArray<float, 3>{
                    R(r + 1, c + 1),
                    G(r + 1, c + 1),
                    B(r + 1, c + 1)}};

            vertices.push_back(v00);
            vertices.push_back(v11);
            vertices.push_back(v01);

            vertices.push_back(v11);
            vertices.push_back(v00);
            vertices.push_back(v10);
        }
    }
    render(vertices, rotate, output);
}

void Mlib::render_height_map(
    const Array<float>& rgb_picture,
    const Array<float>& height_picture,
    const FixedArray<float, 2, 3>& normalization_matrix,
    bool rotate,
    Array<float>* output)
{
    std::vector<ColoredVertex> vertices;
    vertices.reserve(6 * height_picture.nelements());
    assert(rgb_picture.ndim() == 3);
    assert(rgb_picture.shape(0) == 3);
    Array<float> R = rgb_picture[0];
    Array<float> G = rgb_picture[1];
    Array<float> B = rgb_picture[2];
    const Array<float>& Z = height_picture;
    for(size_t r = 0; r < rgb_picture.shape(1) - 1; ++r) {
        for(size_t c = 0; c < rgb_picture.shape(2) - 1; ++c) {
            if (std::isnan(Z(r, c)) ||
                std::isnan(Z(r, c + 1)) ||
                std::isnan(Z(r + 1, c)) ||
                std::isnan(Z(r + 1, c + 1)))
            {
                continue;
            }
            FixedArray<size_t, 2> id0{r, c};
            FixedArray<size_t, 2> id1{r + 1, c + 1};
            FixedArray<float, 2> pos0 = dot1d(normalization_matrix, homogenized_3(i2a(id0)));
            FixedArray<float, 2> pos1 = dot1d(normalization_matrix, homogenized_3(i2a(id1)));
            ColoredVertex v00{
                FixedArray<float, 3>{
                    pos0(0),
                    -pos0(1),
                    Z(r, c)},
                FixedArray<float, 3>{
                    R(r, c),
                    G(r, c),
                    B(r, c)}};
            ColoredVertex v01{
                FixedArray<float, 3>{
                    pos1(0),
                    -pos0(1),
                    Z(r, c + 1)},
                FixedArray<float, 3>{
                    R(r, c + 1),
                    G(r, c + 1),
                    B(r, c + 1)}};
            ColoredVertex v10{
                FixedArray<float, 3>{
                    pos0(0),
                    -pos1(1),
                    Z(r + 1, c)},
                FixedArray<float, 3>{
                    R(r + 1, c),
                    G(r + 1, c),
                    B(r + 1, c)}};
            ColoredVertex v11{
                FixedArray<float, 3>{
                    pos1(0),
                    -pos1(1),
                    Z(r + 1, c + 1)},
                FixedArray<float, 3>{
                    R(r + 1, c + 1),
                    G(r + 1, c + 1),
                    B(r + 1, c + 1)}};

            vertices.push_back(v00);
            vertices.push_back(v11);
            vertices.push_back(v01);

            vertices.push_back(v11);
            vertices.push_back(v00);
            vertices.push_back(v10);
        }
    }
    render(vertices, rotate, output);
}
