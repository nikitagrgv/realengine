#pragma once

#include "Base.h"
#include "BlockInfo.h"

#include "glm/vec2.hpp"

// Blocks order in memory: XZY
struct Chunk
{
public:
    static constexpr int CHUNK_WIDTH = 32;
    static constexpr int CHUNK_HEIGHT = 128;

    static constexpr int CHUNK_WIDTH2 = CHUNK_WIDTH * CHUNK_WIDTH;
    static constexpr int NUM_BLOCKS = CHUNK_WIDTH2 * CHUNK_HEIGHT;

public:
    explicit Chunk(glm::vec2 position, BlockInfo block)
    {
        position_ = position;
        for (BlockInfo &b : blocks_)
        {
            b = block;
        }
    }

    REALENGINE_INLINE const BlockInfo &getBlock(int x, int y, int z) const
    {
        return blocks_[getBlockIndex(x, y, z)];
    }

    REALENGINE_INLINE void setBlock(int x, int y, int z, const BlockInfo &block)
    {
        blocks_[getBlockIndex(x, y, z)] = block;
    }

    static REALENGINE_INLINE int getBlockIndex(int x, int y, int z)
    {
        return x + CHUNK_WIDTH * z + CHUNK_WIDTH2 * y;
    }

public:
    glm::vec2 position_{0, 0};

    BlockInfo blocks_[NUM_BLOCKS];
};
