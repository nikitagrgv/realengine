#pragma once

#include "Base.h"
#include "BlockInfo.h"
#include "VertexBufferObject.h"

#include "glm/vec2.hpp"
#include "glm/vec3.hpp"

struct ChunkMesh;
struct BlockDescription;
class VertexArrayObject;

// Blocks order in memory: XZY
struct Chunk
{
public:
    static constexpr int CHUNK_WIDTH = 32;
    static constexpr int CHUNK_HEIGHT = 400;

    static constexpr int CHUNK_WIDTH2 = CHUNK_WIDTH * CHUNK_WIDTH;
    static constexpr int NUM_BLOCKS = CHUNK_WIDTH2 * CHUNK_HEIGHT;

public:
    explicit Chunk(glm::ivec3 position);
    ~Chunk();

    void clear();

    REMOVE_COPY_MOVE_CLASS(Chunk);

    template<typename F>
    REALENGINE_INLINE void visitRead(F &&func) const;

    template<typename F>
    REALENGINE_INLINE void visitWrite(F &&func);

    template<typename F>
    REALENGINE_INLINE void visitReadGlobal(F &&func) const;

    template<typename F>
    REALENGINE_INLINE void visitWriteGlobal(F &&func);

    static REALENGINE_INLINE bool isInsideChunk(int x, int y, int z)
    {
        return x >= 0 && x < CHUNK_WIDTH && y >= 0 && y < CHUNK_HEIGHT && z >= 0 && z < CHUNK_WIDTH;
    }

    REALENGINE_INLINE const BlockInfo &getBlock(int x, int y, int z) const
    {
        assert(isInsideChunk(x, y, z));
        return blocks_[getBlockIndex(x, y, z)];
    }

    REALENGINE_INLINE void setBlock(int x, int y, int z, const BlockInfo &block)
    {
        assert(isInsideChunk(x, y, z));
        blocks_[getBlockIndex(x, y, z)] = block;
    }

    static REALENGINE_INLINE int getBlockIndex(int x, int y, int z)
    {
        assert(isInsideChunk(x, y, z));
        return x + CHUNK_WIDTH * z + CHUNK_WIDTH2 * y;
    }

    REALENGINE_INLINE glm::ivec3 getBlockGlobalPosition(glm::ivec3 local_pos) const
    {
        glm::ivec3 pos = local_pos;
        pos.x += position_.x * CHUNK_WIDTH;
        pos.z += position_.z * CHUNK_WIDTH;
        return pos;
    }

    REALENGINE_INLINE glm::ivec3 getBlockLocalPosition(glm::ivec3 global_pos) const
    {
        glm::ivec3 pos = global_pos;
        pos.x -= position_.x * CHUNK_WIDTH;
        pos.z -= position_.z * CHUNK_WIDTH;
        return pos;
    }

    REALENGINE_INLINE glm::ivec3 getBlockLocalPosition(glm::vec3 global_pos) const
    {
        return getBlockLocalPosition(glm::ivec3{global_pos});
    }

    int getBlocksOffsetX() const { return position_.x * CHUNK_WIDTH; }
    int getBlocksOffsetZ() const { return position_.z * CHUNK_WIDTH; }

public:
    BlockInfo blocks_[NUM_BLOCKS];
    glm::ivec3 position_{0, 0, 0};

    // TODO: rename this class to ChunkData and move this fields to Chunk
    bool need_rebuild_mesh_{true};
    UPtr<ChunkMesh> mesh_; // could be null
};

////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename F>
void Chunk::visitRead(F &&func) const
{
    int block_index = -1;
    for (int y = 0; y < CHUNK_HEIGHT; ++y)
    {
        for (int z = 0; z < CHUNK_WIDTH; ++z)
        {
            for (int x = 0; x < CHUNK_WIDTH; ++x)
            {
                ++block_index;
                const BlockInfo &b = blocks_[block_index];
                func(x, y, z, b);
            }
        }
    }
}

template<typename F>
void Chunk::visitWrite(F &&func)
{
    need_rebuild_mesh_ = true;
    int block_index = -1;
    for (int y = 0; y < CHUNK_HEIGHT; ++y)
    {
        for (int z = 0; z < CHUNK_WIDTH; ++z)
        {
            for (int x = 0; x < CHUNK_WIDTH; ++x)
            {
                ++block_index;
                BlockInfo &b = blocks_[block_index];
                func(x, y, z, b);
            }
        }
    }
}

template<typename F>
void Chunk::visitReadGlobal(F &&func) const
{
    int block_index = -1;
    const auto x_begin = position_.x * CHUNK_WIDTH;
    const auto x_end = x_begin + CHUNK_WIDTH;
    const auto z_begin = position_.z * CHUNK_WIDTH;
    const auto z_end = z_begin + CHUNK_WIDTH;
    for (int y = 0; y < CHUNK_HEIGHT; ++y)
    {
        for (int z = z_begin; z < z_end; ++z)
        {
            for (int x = x_begin; x < x_end; ++x)
            {
                ++block_index;
                const BlockInfo &b = blocks_[block_index];
                func(x, y, z, b);
            }
        }
    }
}

template<typename F>
void Chunk::visitWriteGlobal(F &&func)
{
    need_rebuild_mesh_ = true;
    int block_index = -1;
    const auto x_begin = position_.x * CHUNK_WIDTH;
    const auto x_end = x_begin + CHUNK_WIDTH;
    const auto z_begin = position_.z * CHUNK_WIDTH;
    const auto z_end = z_begin + CHUNK_WIDTH;
    for (int y = 0; y < CHUNK_HEIGHT; ++y)
    {
        for (int z = z_begin; z < z_end; ++z)
        {
            for (int x = x_begin; x < x_end; ++x)
            {
                ++block_index;
                BlockInfo &b = blocks_[block_index];
                func(x, y, z, b);
            }
        }
    }
}
