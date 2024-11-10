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

// TODO: stupid, use array!

struct ExtendedNeighbourChunks
{
    bool hasAll() const { return px && nx && pz && nz && px_pz && px_nz && nx_pz && nx_nz; }

    Chunk *px{};
    Chunk *nx{};
    Chunk *pz{};
    Chunk *nz{};

    Chunk *px_pz{};
    Chunk *px_nz{};
    Chunk *nx_pz{};
    Chunk *nx_nz{};
};
