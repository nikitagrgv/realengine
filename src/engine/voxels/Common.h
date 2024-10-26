#pragma once

struct Chunk;

struct NeighbourChunks
{
    bool hasAll() const { return px && nx && pz && nz; }

    Chunk *px{};
    Chunk *nx{};
    Chunk *pz{};
    Chunk *nz{};
};
