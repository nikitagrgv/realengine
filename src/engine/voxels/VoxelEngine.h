#pragma once

#include "Base.h"

class BlocksRegistry;

class VoxelEngine
{
public:
    REMOVE_COPY_MOVE_CLASS(VoxelEngine);

    VoxelEngine();
    ~VoxelEngine();

    void init();

private:
    void register_blocks();

private:
    UPtr<BlocksRegistry> registry_;
};
