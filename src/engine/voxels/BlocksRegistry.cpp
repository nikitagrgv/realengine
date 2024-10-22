#include "BlocksRegistry.h"

#include "Texture.h"

BlocksRegistry::BlocksRegistry() {}

void BlocksRegistry::flush()
{
    invalidate();
    if (!atlas_)
    {
        return;
    }

    if (num_atlas_blocks_.x <= 0 || num_atlas_blocks_.y <= 0)
    {
        return;
    }

    const glm::vec2 atlas_size = (glm::vec2)atlas_->getSize();

    for (BlockDescription &block : blocks_)
    {

    }
}

void BlocksRegistry::invalidate()
{
    for (int i = 0; i < blocks_.size(); ++i)
    {
        BlockDescription &block = blocks_[i];
        assert(block.id == i);
        for (glm::vec2 &coords : block.cached.texture_coords)
        {
            coords = glm::vec2(0);
        }
        block.cached.valid = false;
    }
}