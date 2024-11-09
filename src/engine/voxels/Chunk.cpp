#include "Chunk.h"

#include "ChunkMesh.h"

#include <cmath>

float Chunk::BOUND_SPHERE_RADIUS = std::sqrt((float)CHUNK_WIDTH2 * 2 + (float)CHUNK_HEIGHT2);

Chunk::Chunk()
    : Chunk(glm::ivec3{})
{}

Chunk::Chunk(glm::ivec3 position)
{
    position_ = position;
    clear();
    update_values();
}

Chunk::~Chunk() = default;

void Chunk::clear()
{
    for (BlockInfo &b : blocks_)
    {
        b.id = 0;
    }
}

void Chunk::update_values()
{
    bound_sphere_.center_and_radius = glm::vec4(getGlobalCenterPositionFloat(),
        BOUND_SPHERE_RADIUS);
}
