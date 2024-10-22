#include "BlocksRegistry.h"

#include "Texture.h"

BlocksRegistry::BlocksRegistry() {}

void BlocksRegistry::setAtlas(Texture *texture, glm::ivec2 block_size)
{
    atlas_ = texture;
    atlas_size_ = atlas_->getSize();
    block_size_ = block_size;
    num_atlas_blocks_ = atlas_->getSize() / block_size;
    factor_ = glm::vec2{1.0f, 1.0f} / atlas_size_;
}

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

    auto get_texture_coords = [&](int index) -> BlockDescription::TexCoords {
        const float col = float(index % num_atlas_blocks_.x);
        const float row = float(index / num_atlas_blocks_.x);
        BlockDescription::TexCoords tc;
        tc.bottom_left = glm::vec2(col * block_size_.x, row * block_size_.y) * factor_;
        tc.bottom_right = glm::vec2((col + 1) * block_size_.x, row * block_size_.y) * factor_;
        tc.top_left = glm::vec2(col * block_size_.x, (row + 1) * block_size_.y) * factor_;
        tc.top_right = glm::vec2((col + 1) * block_size_.x, (row + 1) * block_size_.y) * factor_;
        return tc;
    };

    for (BlockDescription &block : blocks_)
    {
        if (!block.isValid())
        {
            continue;
        }
        for (int i = 0; i < 6; ++i)
        {
            const int index = block.texture_indexes[i];
            assert(index != -1);
            const BlockDescription::TexCoords tex_coords = get_texture_coords(index);
            block.cached.texture_coords[i] = tex_coords;
            block.cached.valid = true;
        }
    }
}

void BlocksRegistry::invalidate()
{
    for (int i = 0; i < blocks_.size(); ++i)
    {
        BlockDescription &block = blocks_[i];
        assert(block.id == i);
        block.cached.valid = false;
    }
}