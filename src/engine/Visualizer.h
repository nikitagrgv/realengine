#pragma once

#include "Shader.h"
#include "ShaderSource.h"
#include "VertexArrayObject.h"
#include "VertexBufferObject.h"
#include "math/Bounds.h"

#include "glm/fwd.hpp"
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"


class Mesh;
class NodeMesh;


class Visualizer
{
public:
    Visualizer();

    void addLine(const glm::vec3 &s0, const glm::vec3 &s1, const glm::vec4 &color,
        bool depth_test = true);
    void addLine(const glm::vec3 &s0, const glm::vec3 &s1, const glm::vec4 &color0,
        const glm::vec4 &color1, bool depth_test = true);

    void addBoundBox(const math::BoundBox &bb, const glm::vec4 &color, bool depth_test = true);

    void addNormals(NodeMesh *node);
    void addNormals(const Mesh *mesh, const glm::mat4 &transform);

    void render(const glm::mat4 &viewproj);

private:
    struct LinePoint
    {
        glm::vec3 pos;
        glm::vec4 color;
    };
    ShaderSource source_;
    Shader shader;
    VertexBufferObject<LinePoint> lines_vbo_;
    VertexArrayObject lines_vao_;

    VertexBufferObject<LinePoint> nodepth_lines_vbo_;
    VertexArrayObject nodepth_lines_vao_;
};
