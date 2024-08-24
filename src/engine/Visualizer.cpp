#include "Visualizer.h"

// clang-format off
#include <glad/glad.h>
// clang-format on

Visualizer::Visualizer()
{
    lines_vao_.bind();
    lines_vao_.addAttributeFloat(3); // pos
    lines_vao_.addAttributeFloat(4); // color
    lines_vbo_.bind();
    lines_vao_.flush();

    const char *vertex_shader = R"(
            #version 330 core
            layout (location = 0) in vec3 aPos;
            layout (location = 1) in vec4 aColor;
            out vec4 ioColor;
            uniform mat4 uViewProj;
            void main()
            {
                gl_Position = uViewProj * vec4(aPos, 1.0f);
                ioColor = aColor;
            })";
    const char *fragment_shader = R"(
            #version 330 core
            out vec4 FragColor;
            in vec4 ioColor;
            void main()
            {
                FragColor = ioColor;
            })";
    shader.loadSources(vertex_shader, fragment_shader);
}

void Visualizer::addLine(const glm::vec3 &s0, const glm::vec3 &s1, const glm::vec4 &color)
{
    LinePoint p;
    p.color = color;
    p.pos = s0;
    lines_vbo_.addVertex(p);
    p.pos = s1;
    lines_vbo_.addVertex(p);
}

void Visualizer::addLine(const glm::vec3 &s0, const glm::vec3 &s1, const glm::vec4 &color0,
    const glm::vec4 &color1)
{
    LinePoint p;
    p.color = color0;
    p.pos = s0;
    lines_vbo_.addVertex(p);
    p.color = color1;
    p.pos = s1;
    lines_vbo_.addVertex(p);
}

void Visualizer::render(const glm::mat4 &viewproj)
{
    shader.bind();
    shader.setUniformMat4("uViewProj", viewproj);

    lines_vao_.bind();
    lines_vbo_.flush(true);
    glDrawArrays(GL_LINES, 0, lines_vbo_.getNumVertices());
    lines_vbo_.clear();
}