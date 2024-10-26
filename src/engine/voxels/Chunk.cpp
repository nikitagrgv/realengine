#include "Chunk.h"

#include "ChunkMesh.h"

Chunk::Chunk(glm::ivec3 position)
{
    position_ = position;
    for (BlockInfo &b : blocks_)
    {
        b.id = 0;
    }
}

Chunk::~Chunk() = default;
