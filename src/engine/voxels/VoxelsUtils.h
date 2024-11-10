#pragma once

#include "Base.h"
#include "BlockInfo.h"
#include "Chunk.h"
#include "Common.h"

namespace utils
{

REALENGINE_INLINE BlockInfo getBlock(int x, int y, int z, const Chunk &chunk,
    const NeighbourChunks &neighbours)
{
    if (x < 0)
    {
        return neighbours.nx->getBlock(Chunk::CHUNK_WIDTH - 1, y, z);
    }
    if (x >= Chunk::CHUNK_WIDTH)
    {
        return neighbours.px->getBlock(0, y, z);
    }

    if (z < 0)
    {
        return neighbours.nz->getBlock(x, y, Chunk::CHUNK_WIDTH - 1);
    }
    if (z >= Chunk::CHUNK_WIDTH)
    {
        return neighbours.pz->getBlock(x, y, 0);
    }

    return chunk.getBlock(x, y, z);
}

} // namespace utils
