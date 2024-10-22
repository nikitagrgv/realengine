#pragma once

#include "Base.h"
#include "VertexBufferObject.h"

#include "glm/vec2.hpp"
#include "glm/vec3.hpp"


struct GlobalLight;
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
    void render(Camera *camera, GlobalLight *light);

    BlocksRegistry *getRegistry() const { return registry_.get(); }

    unsigned int getSeed() const { return seed_; }
    void setSeed(unsigned int seed);

private:
    void register_blocks();

    UPtr<Chunk> generate_chunk(glm::vec3 pos);

private:
    unsigned int seed_{0};
    struct Perlin;
    UPtr<Perlin> perlin_;

    std::vector<UPtr<Chunk>> chunks_;

    // TODO# TEMP
    UPtr<ShaderSource> shader_source_;
    UPtr<Shader> shader_;

    UPtr<BlocksRegistry> registry_;
};
