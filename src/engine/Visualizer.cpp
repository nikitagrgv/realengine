#include "Visualizer.h"

// clang-format off
#include "Mesh.h"
#include "profiler/ScopedProfiler.h"
#include "utils/Algos.h"


#include <NodeMesh.h>
#include <glad/glad.h>
// clang-format on

Visualizer::Visualizer()
{
    lines_vao_.bind();
    lines_vao_.addAttributeFloat(3); // pos
    lines_vao_.addAttributeFloat(4); // color
    lines_vbo_.bind();
    lines_vao_.flush();

    nodepth_lines_vao_.bind();
    nodepth_lines_vao_.addAttributeFloat(3); // pos
    nodepth_lines_vao_.addAttributeFloat(4); // color
    nodepth_lines_vbo_.bind();
    nodepth_lines_vao_.flush();


    const char *vertex_shader = R"(
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
            out vec4 FragColor;
            in vec4 ioColor;
            void main()
            {
                FragColor = ioColor;
            })";
    source_.setSources(vertex_shader, fragment_shader);
    shader.setSource(&source_);
    shader.recompile();
}

void Visualizer::addLine(const glm::vec3 &s0, const glm::vec3 &s1, const glm::vec4 &color,
    bool depth_test, float time)
{
    if (time == 0.0f)
    {
        auto &vbo = depth_test ? lines_vbo_ : nodepth_lines_vbo_;

        LinePoint p;
        p.color = color;
        p.pos = s0;
        vbo.addVertex(p);
        p.pos = s1;
        vbo.addVertex(p);
    }
    else
    {
        queued_lines_.emplace_back(s0, s1, color, depth_test, time);
    }
}

void Visualizer::addLine(const glm::vec3 &s0, const glm::vec3 &s1, const glm::vec4 &color0,
    const glm::vec4 &color1, bool depth_test)
{
    auto &vbo = depth_test ? lines_vbo_ : nodepth_lines_vbo_;

    LinePoint p;
    p.color = color0;
    p.pos = s0;
    vbo.addVertex(p);
    p.color = color1;
    p.pos = s1;
    vbo.addVertex(p);
}

void Visualizer::addBoundBox(const math::BoundBox &bb, const glm::vec4 &color, bool depth_test)
{
    const auto min = bb.min;
    const auto max = bb.max;

    glm::vec3 a0 = glm::vec3{min.x, min.y, min.z};
    glm::vec3 a1 = glm::vec3{max.x, min.y, min.z};
    glm::vec3 a2 = glm::vec3{max.x, max.y, min.z};
    glm::vec3 a3 = glm::vec3{min.x, max.y, min.z};

    glm::vec3 b0 = glm::vec3{min.x, min.y, max.z};
    glm::vec3 b1 = glm::vec3{max.x, min.y, max.z};
    glm::vec3 b2 = glm::vec3{max.x, max.y, max.z};
    glm::vec3 b3 = glm::vec3{min.x, max.y, max.z};

    addLine(a0, a1, color, depth_test);
    addLine(a1, a2, color, depth_test);
    addLine(a2, a3, color, depth_test);
    addLine(a3, a0, color, depth_test);

    addLine(b0, b1, color, depth_test);
    addLine(b1, b2, color, depth_test);
    addLine(b2, b3, color, depth_test);
    addLine(b3, b0, color, depth_test);

    addLine(a0, b0, color, depth_test);
    addLine(a1, b1, color, depth_test);
    addLine(a2, b2, color, depth_test);
    addLine(a3, b3, color, depth_test);
}

void Visualizer::addNormals(NodeMesh *node)
{
    if (!node->getMesh())
    {
        return;
    }
    const glm::mat4 &transform = node->getTransform();
    const Mesh *mesh = node->getMesh();
    addNormals(mesh, transform);
}

void Visualizer::addNormals(const Mesh *mesh, const glm::mat4 &transform)
{
    const auto to_local = [&](const glm::vec3 &v) {
        return transform * glm::vec4(v, 1);
    };
    for (int i = 0; i < mesh->getNumCpuVertices(); i++)
    {
        const auto pos = mesh->getVertexPos(i);
        const auto norm = mesh->getVertexNormal(i);
        addLine(to_local(pos), to_local(pos + norm), {0.0f, 0.0f, 0.0f, 0.0f},
            {1.0f, 0.0f, 0.0f, 0.2f});
    }
}

void Visualizer::update(float dt)
{
    // TODO# shitty
    for (QueuedLine &l : queued_lines_)
    {
        if (l.time > 0.0f)
        {
            addLine(l.s0, l.s1, l.color, l.depth_test);
        }
        l.time -= dt;
    }
    Alg::removeIf(queued_lines_, [](const QueuedLine &l) { return l.time <= 0.0f; });
}

void Visualizer::render(const glm::mat4 &viewproj)
{
    SCOPED_FUNC_PROFILER;

    shader.bind();
    shader.setUniformMat4("uViewProj", viewproj);

    GL_CHECKED(glEnable(GL_DEPTH_TEST));
    lines_vao_.bind();
    lines_vbo_.flush(true);
    GL_CHECKED(glDrawArrays(GL_LINES, 0, lines_vbo_.getNumGpuVertices()));
    lines_vbo_.clear();

    GL_CHECKED(glDisable(GL_DEPTH_TEST));
    nodepth_lines_vao_.bind();
    nodepth_lines_vbo_.flush(true);
    GL_CHECKED(glDrawArrays(GL_LINES, 0, nodepth_lines_vbo_.getNumGpuVertices()));
    nodepth_lines_vbo_.clear();
}