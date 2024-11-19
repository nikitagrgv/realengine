#include "ChunksMap.h"

void ChunksMap::setRadius(int radius) {}

int ChunksMap::getRadius() const {}

void ChunksMap::setCenter(glm::ivec2 center) {}

void ChunksMap::getCenter() const {}

Chunk *ChunksMap::getChunk(glm::ivec2 pos) const {}

bool ChunksMap::hasChunk(glm::ivec2 pos) const {}

void ChunksMap::setChunk(glm::ivec2 pos, UPtr<Chunk> chunk) const {}

Chunk *ChunksMap::takeChunk(glm::ivec2 pos) {}

void ChunksMap::setUnloadCallback(UnloadCallback callback)
{
    unload_callback_ = std::move(callback);
}

void ChunksMap::clearUnloadCallback()
{
    unload_callback_.clear();
}