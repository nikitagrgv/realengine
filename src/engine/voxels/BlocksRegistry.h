#pragma once

#include "Base.h"
#include "BlockDescription.h"

#include <vector>

class Texture;

class BlocksRegistry
{
public:
    BlocksRegistry();

    BlockDescription &addBlock()
    {
        BlockDescription block;
        const int old_size = blocks_.size();
        block.id = old_size;
        blocks_.push_back(std::move(block));
        return blocks_[old_size];
    }

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

    void setAtlas(Texture *texture, glm::ivec2 block_size);

    REALENGINE_INLINE Texture *getAtlas() const { return atlas_; }

    void flush();

private:
    void invalidate();

private:
    Texture *atlas_{};
    glm::vec2 atlas_size_{-1, -1};
    glm::vec2 block_size_{-1, -1};
    glm::ivec2 num_atlas_blocks_{-1, -1};
    glm::vec2 factor_{-1, -1};
    std::vector<BlockDescription> blocks_;
};
