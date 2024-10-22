#include "VoxelEngine.h"

#include "BlocksRegistry.h"

VoxelEngine::VoxelEngine() = default;

VoxelEngine::~VoxelEngine() = default;

void VoxelEngine::init()
{
    registry_ = makeU<BlocksRegistry>();
}