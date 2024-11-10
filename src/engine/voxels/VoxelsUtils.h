#pragma once

#include "Base.h"
#include "BlockInfo.h"
#include "Chunk.h"
#include "Common.h"

namespace utils
{

REALENGINE_INLINE BlockInfo getBlock(int x, int y, int z, const Chunk &chunk,
    const ExtendedNeighbourChunks &neighbours)
{
    // TODO: branchless

    if (x < 0)
    {
        if (z < 0)
        {
            return neighbours.nx_nz->getBlock(x + Chunk::CHUNK_WIDTH, y, z + Chunk::CHUNK_WIDTH);
        }
        if (z >= Chunk::CHUNK_WIDTH)
        {
            return neighbours.nx_pz->getBlock(x + Chunk::CHUNK_WIDTH, y, z - Chunk::CHUNK_WIDTH);
        }
        return neighbours.nx->getBlock(x + Chunk::CHUNK_WIDTH, y, z);
    }

    if (x >= Chunk::CHUNK_WIDTH)
    {
        if (z < 0)
        {
            return neighbours.px_nz->getBlock(x - Chunk::CHUNK_WIDTH, y, z + Chunk::CHUNK_WIDTH);
        }
        if (z >= Chunk::CHUNK_WIDTH)
        {
            return neighbours.px_pz->getBlock(x - Chunk::CHUNK_WIDTH, y, z - Chunk::CHUNK_WIDTH);
        }
        return neighbours.px->getBlock(x - Chunk::CHUNK_WIDTH, y, z);
    }

    if (z < 0)
    {
        return neighbours.nz->getBlock(x, y, z + Chunk::CHUNK_WIDTH);
    }
    if (z >= Chunk::CHUNK_WIDTH)
    {
        return neighbours.pz->getBlock(x, y, z - Chunk::CHUNK_WIDTH);
    }

    return chunk.getBlock(x, y, z);
}

} // namespace utils
