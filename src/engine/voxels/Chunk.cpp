#include "Chunk.h"

#include "ChunkMesh.h"

Chunk::Chunk(glm::ivec3 position)
{
    position_ = position;
    clear();
}

Chunk::~Chunk() = default;

void Chunk::clear()
{
    for (BlockInfo &b : blocks_)
    {
        b.id = 0;
    }
}
