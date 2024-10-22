#pragma once

#include "Base.h"
#include "BlockDescription.h"

#include <vector>

class Texture;

class BlocksRegistry
{
public:
    BlocksRegistry();

    void addBlock(const BlockDescription &block)
    {
        assert(block.isValid());
        if (block.id > blocks_.size())
        {
            blocks_.resize(block.id + 1);
        }
        assert(!blocks_[block.id].isValid());
        blocks_[block.id] = block;
    }

    REALENGINE_INLINE const BlockDescription &getBlock(int id) const { return blocks_[id]; }

    void setAtlas(Texture *texture, glm::ivec2 num_atlas_blocks)
    {
        atlas_ = texture;
        num_atlas_blocks_ = num_atlas_blocks;
    }

    REALENGINE_INLINE Texture *getAtlas() const { return atlas_; }
    REALENGINE_INLINE glm::ivec2 getNumAtlasBlocks() const { return num_atlas_blocks_; }

    void flush();

private:
    void invalidate();

private:
    Texture *atlas_{};
    glm::ivec2 num_atlas_blocks_{-1, -1};
    std::vector<BlockDescription> blocks_;
};
