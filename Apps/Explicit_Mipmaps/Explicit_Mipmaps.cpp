// From: https:// stackoverflow.com/questions/42596516/generating-mipmaps-manually

#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <Mlib/Render/CHK.hpp>
#include <iostream>

const GLchar* VERTEX_SHADER_SOURCE =
        "#version 330 core                           \n"
        "layout (location = 0) in vec2 position;     \n"
        "layout (location = 1) in vec2 myTexCoord;   \n"
        "                                            \n"
        "out vec2 TexCoord;                          \n"
        "                                            \n"
        "void main()                                 \n"
        "{                                           \n"
        "    gl_Position = vec4(position,0.0f, 1.0f);\n"
        "    TexCoord = myTexCoord;                  \n"
        "}                                           \n";

const GLchar* FRAGMENT_SHADER_SOURCE =
        "#version 330 core                         \n"
        "in vec2 TexCoord;                         \n"
        "                                          \n"
        "out vec4 color;                           \n"
        "                                          \n"
        "uniform sampler2D ourTexture;             \n"
        "                                          \n"
        "void main()                               \n"
        "{                                         \n"
        "    color = texture(ourTexture, TexCoord);\n"
        "}                                         \n";

int main() {
    // Change the value of size by 1,2,4,8,16,32,64,128 or 256 and see how the color of the triangle doesn't change. Why does it happen?
    GLint size = 128;

    // INIT STUFFS
    GLFW_CHK(glfwInit());
    GLFW_CHK(glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3));
    GLFW_CHK(glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3));
    GLFW_CHK(glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE));
    GLFW_CHK(glfwWindowHint(GLFW_RESIZABLE, GL_FALSE));
    GLFW_CHK(GLFWwindow* window = glfwCreateWindow(size, size, "", nullptr, nullptr));
    GLFW_CHK(glfwMakeContextCurrent(window));
    gladLoadGL(glfwGetProcAddress);
    glViewport(0, 0, size, size);

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    // Our image for the mipmap with odd level and green color
    // The dimensions are 256 x 256 and is multiplied by 3 because I defined R,G and B colors
    unsigned char imageArray[256 * 256 * 3];
    for (int i = 0; i < 256 * 256 * 3; i++) {
        if ((i + 2) % 3 == 0)
            imageArray[i] = 255;
        else
            imageArray[i] = 0;
    }

    // Our image for the mipmap with pair level and red color
    unsigned char imageArray2[256 * 256 * 3];
    for (int i = 0; i < 256 * 256 * 3; i++) {
        if (i % 3 == 0)
            imageArray2[i] = 255;
        else
            imageArray2[i] = 0;
    }

    CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST));
    CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 8));

    // All mipmap levels defined by hand
    CHK(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 256, 256, 0, GL_RGB, GL_UNSIGNED_BYTE, imageArray));
    CHK(glTexImage2D(GL_TEXTURE_2D, 1, GL_RGB, 128, 128, 0, GL_RGB, GL_UNSIGNED_BYTE, imageArray2));
    CHK(glTexImage2D(GL_TEXTURE_2D, 2, GL_RGB, 64,  64,  0, GL_RGB, GL_UNSIGNED_BYTE, imageArray));
    CHK(glTexImage2D(GL_TEXTURE_2D, 3, GL_RGB, 32,  32,  0, GL_RGB, GL_UNSIGNED_BYTE, imageArray2));
    CHK(glTexImage2D(GL_TEXTURE_2D, 4, GL_RGB, 16,  16,  0, GL_RGB, GL_UNSIGNED_BYTE, imageArray));
    CHK(glTexImage2D(GL_TEXTURE_2D, 5, GL_RGB, 8,   8,   0, GL_RGB, GL_UNSIGNED_BYTE, imageArray2));
    CHK(glTexImage2D(GL_TEXTURE_2D, 6, GL_RGB, 4,   4,   0, GL_RGB, GL_UNSIGNED_BYTE, imageArray));
    CHK(glTexImage2D(GL_TEXTURE_2D, 7, GL_RGB, 2,   2,   0, GL_RGB, GL_UNSIGNED_BYTE, imageArray2));
    CHK(glTexImage2D(GL_TEXTURE_2D, 8, GL_RGB, 1,   1,   0, GL_RGB, GL_UNSIGNED_BYTE, imageArray));

    GLfloat vertices[] = {
            -1.f, -1.f, 1.f, -1.f, 0.f, 1.f,  // Vertex coordinates
            0.f, 0.f, 1.f, 0.f, 0.5f, 1.f     // Triangle's texture coordinates
    };

    // VAOs and VBOs
    GLuint VAO, VBO;
    CHK(glGenVertexArrays(1, &VAO));
    CHK(glGenBuffers(1, &VBO));
    CHK(glBindVertexArray(VAO));
    CHK(glBindBuffer(GL_ARRAY_BUFFER, VBO));
    CHK(glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW));

    CHK(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (GLvoid*)0));
    CHK(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (GLvoid*)(sizeof(GLfloat)*3*2)));

    CHK(glEnableVertexAttribArray(0));
    CHK(glEnableVertexAttribArray(1));

    // Shaders
    GLuint program;
    GLuint vertex, fragment;

    CHK(vertex = glCreateShader(GL_VERTEX_SHADER));
    CHK(glShaderSource(vertex, 1, &VERTEX_SHADER_SOURCE, NULL));
    CHK(glCompileShader(vertex));

    CHK(fragment = glCreateShader(GL_FRAGMENT_SHADER));
    CHK(glShaderSource(fragment, 1, &FRAGMENT_SHADER_SOURCE, NULL));
    CHK(glCompileShader(fragment));

    CHK(program = glCreateProgram());
    CHK(glAttachShader(program, vertex));
    CHK(glAttachShader(program, fragment));
    CHK(glLinkProgram(program));

    CHK(glDeleteShader(vertex));
    CHK(glDeleteShader(fragment));
    CHK(glUseProgram(program));

    // Drawing our triangle
    CHK(glClearColor(0.2f, 0.3f, 0.3f, 1.0f));
    CHK(glClear(GL_COLOR_BUFFER_BIT));
    CHK(glDrawArrays(GL_TRIANGLES, 0, 3));
    GLFW_CHK(glfwSwapBuffers(window));
    while (!glfwWindowShouldClose(window)) {
        GLFW_CHK(glfwPollEvents());
    }

    CHK(glDeleteVertexArrays(1, &VAO));
    CHK(glDeleteBuffers(1, &VBO));
    GLFW_CHK(glfwTerminate());
    return 0;
}
