#pragma once

#include "Base.h"

class BlocksRegistry;

class VoxelEngine
{
public:
    VoxelEngine();
    ~VoxelEngine();
    void init();

private:
    UPtr<BlocksRegistry> registry_;
};
