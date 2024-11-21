#pragma once

#include "Base.h"
#include "BlockInfo.h"
#include "Chunk.h"
#include "ChunksMap.h"
#include "Common.h"
#include "VertexBufferObject.h"
#include "math/Math.h"
#include "signals/Signals.h"
#include "threads/Job.h"
#include "utils/Algos.h"
#include "utils/Hashers.h"

#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include <utility>

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

    void setAmbientOcclusionEnabled(bool enabled);
    bool isAmbientOcclusionEnabled() const;

    void init();

    void update(const glm::vec3 &position);
    void render(Camera *camera, GlobalLight *light);

    BlocksRegistry *getRegistry() const { return registry_.get(); }

    unsigned int getSeed() const { return seed_; }
    void setSeed(unsigned int seed);

    static REALENGINE_INLINE glm::ivec3 toBlockPosition(const glm::vec3 &position)
    {
        // dont care about y < 0 flooring. but consider case eg -0.1 mustn't be rounded to 0
        return {(int)std::floor(position.x), int(position.y + 1) - 1, (int)std::floor(position.z)};
    }

    Chunk *getChunkAtPosition(const glm::ivec3 &position) const
    {
        const glm::ivec3 chunk_pos = pos_to_chunk_pos(position);
        return get_chunk_at_pos(chunk_pos.x, chunk_pos.z);
    }

    REALENGINE_INLINE Chunk *getChunkAtPosition(const glm::vec3 &position) const
    {
        const glm::ivec3 chunk_pos = pos_to_chunk_pos(position);
        return get_chunk_at_pos(chunk_pos.x, chunk_pos.z);
    }

    REALENGINE_INLINE bool getBlockAtPosition(const glm::vec3 &position, BlockInfo &out_block) const
    {
        const glm::ivec3 block_pos = toBlockPosition(position);
        return getBlockAtPosition(block_pos, out_block);
    }

    bool getBlockAtPosition(const glm::ivec3 &position, BlockInfo &out_block) const;

    REALENGINE_INLINE bool setBlockAtPosition(const glm::vec3 &position, BlockInfo block)
    {
        const glm::ivec3 block_pos = toBlockPosition(position);
        return setBlockAtPosition(block_pos, block);
    }

    bool setBlockAtPosition(const glm::ivec3 &position, BlockInfo block);

    struct IntersectionResult
    {
        IntersectionResult() = default;
        IntersectionResult(Chunk *chunk, const BlockInfo &block, const glm::ivec3 &loc_pos,
            const glm::ivec3 &glob_pos)
            : chunk(chunk)
            , block(block)
            , loc_pos(loc_pos)
            , glob_pos(glob_pos)
        {}
        bool isValid() const { return chunk; }

        Chunk *chunk{};
        BlockInfo block{};
        glm::ivec3 loc_pos{};
        glm::ivec3 glob_pos{};
        float distance{};
    };

    IntersectionResult getIntersection(const glm::vec3 &position, const glm::vec3 &dir,
        float max_distance) const;

    // bool &continue
    using VisitIntersectionCallback = Callback<const IntersectionResult &, bool &>;
    void visitIntersection(const glm::vec3 &position, const glm::vec3 &dir,
        const VisitIntersectionCallback &callback) const;

private:
    void register_blocks();

    UPtr<ChunkMesh> get_mesh_cached();
    void release_mesh(UPtr<ChunkMesh> mesh);

    UPtr<Chunk> get_chunk_cached(const glm::ivec3 &pos);
    void release_chunk(UPtr<Chunk> chunk);

    void queue_generate_chunk(UPtr<Chunk> chunk);
    void generate_chunk_threadsafe(Chunk &chunk) const;
    void finish_generate_chunk(UPtr<Chunk> chunk, bool generated);

    void on_chunk_unloaded_from_map(UPtr<Chunk> chunk);

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
        return chunks_map_.hasChunk({x, z});
    }

    REALENGINE_INLINE bool is_enqued_for_generation(int x, int z) const
    {
        const glm::ivec2 pos = glm::ivec2{x, z};
        return Alg::anyOf(enqueued_chunks_, [&](const EnqueuedChunk &c) { return c.pos == pos; });
    }

    REALENGINE_INLINE bool is_generated(int x, int z) const
    {
        return Alg::anyOf(generated_chunks_, [&](const UPtr<Chunk> &c) {
            const glm::ivec3 pos = c->getPosition();
            return pos.x == x && pos.z == z;
        });
    }

    bool has_all_neighbours(Chunk *chunk) const;
    void get_neighbour_chunks_lazy(const Chunk *chunk, ExtendedNeighbourChunks &chunks,
        bool &has_all) const;

private:
    bool first_update_{true};

    unsigned int seed_{0};
    struct Perlin;
    UPtr<Perlin> perlin_;

    glm::ivec3 last_base_chunk_pos_{};

    std::vector<UPtr<ChunkMesh>> meshes_pool_;
    std::vector<UPtr<Chunk>> chunks_pool_;

    ChunksMap chunks_map_;

    struct EnqueuedChunk
    {
        EnqueuedChunk(int x, int z, tbb::CancelToken cancel_token)
            : pos(x, z)
            , cancel_token(std::move(cancel_token))
        {}
        glm::ivec2 pos;
        tbb::CancelToken cancel_token;
    };

    std::vector<EnqueuedChunk> enqueued_chunks_;

    std::vector<UPtr<Chunk>> chunks_to_generate_;

    std::vector<UPtr<Chunk>> generated_chunks_;
    std::vector<UPtr<Chunk>> canceled_chunks_;

    // TODO# TEMP
    UPtr<ShaderSource> shader_source_;
    UPtr<Shader> shader_;

    UPtr<BlocksRegistry> registry_;

    // TEMPORAY IN FUNCTION
    std::vector<Chunk *> chunks_for_regenerate_;
    std::vector<Chunk *> chunks_for_render_;

    int old_num_inited_chunks_{0};

    // TODO# SHIT?
    struct OffsetsCache
    {
        const std::vector<glm::ivec2> &getOffsets(int radius);

    private:
        // now only for RADIUS_SPAWN_CHUNK now
        std::vector<glm::ivec2> values;
        int radius = -1;
    } offsets_cache;
};
