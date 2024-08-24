#pragma once
#include "Shader.h"
#include "VertexArrayObject.h"
#include "VertexBufferObject.h"

#include "glm/fwd.hpp"
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"

class Visualizer
{
public:
    Visualizer();

    void addLine(const glm::vec3 &s0, const glm::vec3 &s1, const glm::vec4 &color);
    void addLine(const glm::vec3 &s0, const glm::vec3 &s1, const glm::vec4 &color0,
        const glm::vec4 &color1);

    void render(const glm::mat4 &viewproj);

private:
    struct LinePoint
    {
        glm::vec3 pos;
        glm::vec4 color;
    };
    Shader shader;
    VertexBufferObject<LinePoint> lines_vbo_;
    VertexArrayObject lines_vao_;
};
