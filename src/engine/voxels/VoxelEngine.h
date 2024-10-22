#pragma once

#include "Base.h"
#include "VertexBufferObject.h"

#include "glm/vec2.hpp"
#include "glm/vec3.hpp"

struct Chunk;
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

    void update(const glm::vec3 &position);
    void render(Camera *camera);

    BlocksRegistry *getRegistry() const { return registry_.get(); }

private:
    void register_blocks();

    UPtr<Chunk> generate_chunk(glm::vec3 pos);

private:
    std::vector<UPtr<Chunk>> chunks_;

    // TODO# TEMP
    UPtr<ShaderSource> shader_source_;
    UPtr<Shader> shader_;

    UPtr<BlocksRegistry> registry_;
};
