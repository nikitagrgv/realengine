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
    const glm::vec2 elem_size = atlas_size / (glm::vec2)num_atlas_blocks_;

    auto get_texture_coords = [&](int index) -> BlockDescription::TexCoords {
        const int column = index / num_atlas_blocks_.x;
        const int row = index % num_atlas_blocks_.x;
        const float column_f = float(column);
        const float row_f = float(row);
        BlockDescription::TexCoords tc;
        tc.bottom_left = glm::vec2(column_f * elem_size.x, row_f * elem_size.y);
        tc.bottom_right = glm::vec2(column_f * elem_size.x, (row_f + 1) * elem_size.y);
        tc.top_left = glm::vec2((column_f + 1) * elem_size.x, row_f * elem_size.y);
        tc.top_right = glm::vec2((column_f + 1) * elem_size.x, (row_f + 1) * elem_size.y);
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