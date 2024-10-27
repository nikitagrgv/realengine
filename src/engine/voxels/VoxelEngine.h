#pragma once

#include "Base.h"
#include "BlockInfo.h"
#include "Chunk.h"
#include "Common.h"
#include "VertexBufferObject.h"
#include "math/Math.h"
#include "utils/Algos.h"
#include "utils/Hashers.h"

#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include <unordered_map>

struct ChunkMesh;
struct GlobalLight;
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

    static REALENGINE_INLINE glm::ivec3 toBlockPosition(const glm::vec3 &position)
    {
        // dont care about y < 0 flooring. but consider case eg -0.1 mustn't be rounded to 0
        return {(int)std::floor(position.x), int(position.y + 1) - 1, (int)std::floor(position.z)};
    }

    Chunk *getChunkAtPosition(const glm::ivec3 &position)
    {
        const glm::ivec3 chunk_pos = pos_to_chunk_pos(position);
        return get_chunk_at_pos(chunk_pos.x, chunk_pos.z);
    }

    REALENGINE_INLINE Chunk *getChunkAtPosition(const glm::vec3 &position)
    {
        const glm::ivec3 chunk_pos = pos_to_chunk_pos(position);
        return get_chunk_at_pos(chunk_pos.x, chunk_pos.z);
    }

    REALENGINE_INLINE bool setBlockAtPosition(const glm::vec3 &position, BlockInfo block)
    {
        const glm::ivec3 block_pos = toBlockPosition(position);
        return setBlockAtPosition(block_pos, block);
    }

    bool setBlockAtPosition(const glm::ivec3 &position, BlockInfo block);

private:
    void register_blocks();

    UPtr<ChunkMesh> get_mesh_cached();
    void release_mesh(UPtr<ChunkMesh> mesh);

    UPtr<Chunk> get_chunk_cached(const glm::ivec3 &pos);
    void release_chunk(UPtr<Chunk> chunk);

    void queue_generate_chunk(UPtr<Chunk> chunk);
    void generate_chunk_threadsafe(Chunk &chunk) const;
    void finish_generate_chunk(UPtr<Chunk> chunk);

    static REALENGINE_INLINE glm::ivec3 pos_to_chunk_pos(const glm::vec3 &pos)
    {
        const auto x = math::floorToCell(std::floor(pos.x), Chunk::CHUNK_WIDTH);
        const auto y = 0;
        const auto z = math::floorToCell(std::floor(pos.z), Chunk::CHUNK_WIDTH);
        return glm::ivec3{x, y, z};
    }

    static REALENGINE_INLINE glm::ivec3 pos_to_chunk_pos(const glm::ivec3 &pos)
    {
        const auto x = math::floorToCell(pos.x, Chunk::CHUNK_WIDTH);
        const auto y = 0;
        const auto z = math::floorToCell(pos.z, Chunk::CHUNK_WIDTH);
        return glm::ivec3{x, y, z};
    }

    Chunk *get_chunk_at_pos(int x, int z) const;
    REALENGINE_INLINE bool has_chunk_at_pos(int x, int z) const
    {
        const glm::ivec2 pos = glm::ivec2{x, z};
        const auto it = chunk_index_by_pos_.find(pos);
        return it != chunk_index_by_pos_.end();
    }

    REALENGINE_INLINE bool is_enqued_for_generation(int x, int z) const
    {
        const glm::ivec2 pos = glm::ivec2{x, z};
        return Alg::contains(enqueued_chunks_, pos);
    }

    REALENGINE_INLINE bool is_generated(int x, int z) const
    {
        return Alg::anyOf(generated_chunks_,
            [&](const UPtr<Chunk> &c) { return c->position_.x == x && c->position_.z == z; });
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

    std::vector<glm::ivec2> enqueued_chunks_;

    std::vector<UPtr<Chunk>> chunks_to_generate_;

    std::vector<UPtr<Chunk>> generated_chunks_;

    // TODO# TEMP
    UPtr<ShaderSource> shader_source_;
    UPtr<Shader> shader_;

    UPtr<BlocksRegistry> registry_;
};
