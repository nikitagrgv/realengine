#pragma once

#include "Base.h"
#include "VertexBufferObject.h"

#include "glm/vec2.hpp"
#include "glm/vec3.hpp"


struct BlockDescription;
class Camera;
class ShaderSource;
class Shader;
class VertexArrayObject;
class BlocksRegistry;

class VoxelEngine
{
public:
    REMOVE_COPY_MOVE_CLASS(VoxelEngine);

    VoxelEngine();
    ~VoxelEngine();

    void init();

    void render(Camera *camera);

private:
    void register_blocks();

    // TODO#
    void generate_chunk();

    void gen_top(const glm::vec3 &min, const glm::vec3 &max, const BlockDescription &desc);
    void gen_bottom(const glm::vec3 &min, const glm::vec3 &max, const BlockDescription &desc);

private:
    // TODO# TEMP
    struct Vertex
    {
        glm::vec3 pos_;
        glm::vec2 uv_;
    };
    UPtr<VertexArrayObject> vao_;
    UPtr<VertexBufferObject<Vertex>> vbo_;
    UPtr<ShaderSource> shader_source_;
    UPtr<Shader> shader_;

    UPtr<BlocksRegistry> registry_;
};
