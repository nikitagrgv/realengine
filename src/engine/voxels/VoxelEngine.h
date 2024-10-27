#pragma once

#include "Base.h"
#include "BlockInfo.h"
#include "Common.h"
#include "VertexBufferObject.h"
#include "utils/Hashers.h"

#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include <unordered_map>

struct ChunkMesh;
struct GlobalLight;
struct Chunk;
struct NeighbourChunks;
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

    int getNumRenderChunks() const;
    uint64_t getNumRenderVertices() const;

    Chunk *getChunkAtPosition(const glm::vec3 &position);

    bool setBlockAtPosition(const glm::ivec3 &position, BlockInfo block);

private:
    void register_blocks();

    UPtr<ChunkMesh> get_mesh_cached();
    void release_mesh(UPtr<ChunkMesh> mesh);

    UPtr<Chunk> get_chunk_cached(const glm::ivec3 &pos);
    void release_chunk(UPtr<Chunk> chunk);

    void generate_chunk(Chunk &chunk);

    glm::ivec3 pos_to_chunk_pos(const glm::vec3 &pos) const;

    Chunk *get_chunk_at_pos(int x, int z) const;
    REALENGINE_INLINE bool has_chunk_at_pos(int x, int z) const
    {
        const glm::ivec2 pos = glm::ivec2{x, z};
        const auto it = chunk_index_by_pos_.find(pos);
        return it != chunk_index_by_pos_.end();
    }
    bool has_all_neighbours(Chunk *chunk) const;
    NeighbourChunks get_neighbour_chunks(Chunk *chunk) const;
    NeighbourChunks get_neighbour_chunks_lazy(Chunk *chunk) const;

    void refresh_chunk_index_by_pos();

private:
    unsigned int seed_{0};
    struct Perlin;
    UPtr<Perlin> perlin_;

    std::vector<UPtr<ChunkMesh>> meshes_pool_;
    std::vector<UPtr<Chunk>> chunks_pool_;

    std::vector<UPtr<Chunk>> chunks_;
    std::unordered_map<glm::ivec2, int> chunk_index_by_pos_;

    std::vector<Chunk *> chunks_to_generate_;

    // TODO# TEMP
    UPtr<ShaderSource> shader_source_;
    UPtr<Shader> shader_;

    UPtr<BlocksRegistry> registry_;
};
