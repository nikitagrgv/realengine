#pragma once

#include "BlockInfo.h"

struct Chunk;
struct BlockDescription;

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

template<typename T>
struct Blocks3x3
{
    T getCenter() const { return blocks[13]; }
    void setCenter(T block) { blocks[13] = block; }

    T getBlockAtOffset(int x, int y, int z) const
    {
        assert(x >= -1 && x <= 1);
        assert(y >= -1 && y <= 1);
        assert(z >= -1 && z <= 1);
        return blocks[getIndex(x, y, z)];
    }

    void setBlockAtOffset(int x, int y, int z, T block)
    {
        assert(x >= -1 && x <= 1);
        assert(y >= -1 && y <= 1);
        assert(z >= -1 && z <= 1);
        blocks[getIndex(x, y, z)] = block;
    }

    static constexpr int getIndex(int x, int y, int z)
    {
        int ix = x + 1;
        int iy = y + 1;
        int iz = z + 1;
        return ix + 3 * iz + 9 * iy;
    }

    T blocks[27];
};

struct Descriptions3x3 : public Blocks3x3<const BlockDescription *>
{
};