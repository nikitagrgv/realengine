#include "ChunksMap.h"

#include "math/Math.h"

namespace
{

REALENGINE_INLINE int arr_width_from_radius(int radius)
{
    return radius * 2 + 1;
}

REALENGINE_INLINE int arr_length_from_radius(int radius)
{
    const int width = arr_width_from_radius(radius);
    return width * width;
}

REALENGINE_INLINE bool is_valid_pos(int radius, glm::ivec2 loc_pos)
{
    return loc_pos.x >= -radius && loc_pos.x <= radius && loc_pos.y >= -radius
        && loc_pos.y <= radius;
}

REALENGINE_INLINE int get_index(int radius, glm::ivec2 loc_pos)
{
    assert(is_valid_pos(radius, loc_pos));
    return (loc_pos.x + radius) + (loc_pos.y + radius) * (radius * 2 + 1);
}

REALENGINE_INLINE UPtr<Chunk> &get_chunk_by_loc_pos(std::vector<UPtr<Chunk>> &chunks, int radius,
    glm::ivec2 loc_pos)
{
    assert(chunks.size() == arr_length_from_radius(radius));
    const int index = get_index(radius, loc_pos);
    return chunks[index];
}

REALENGINE_INLINE const UPtr<Chunk> &get_chunk_by_loc_pos(const std::vector<UPtr<Chunk>> &chunks,
    int radius, glm::ivec2 loc_pos)
{
    assert(chunks.size() == arr_length_from_radius(radius));
    const int index = get_index(radius, loc_pos);
    return chunks[index];
}

} // namespace

ChunksMap::ChunksMap()
{
    const int len = arr_length_from_radius(radius_);
    chunks_.resize(len);
    chunks_old_.resize(len);
}

void ChunksMap::setRadius(int radius)
{
    if (radius == radius_)
    {
        return;
    }

    assert(check_sizes());

    const bool expand = radius > radius_;
    const bool collapse = !expand;
    const int min_radius = expand ? radius_ : radius;

    assert(check_buf_empty());
    std::swap(chunks_, chunks_old_);

    const int new_len = arr_length_from_radius(radius);
    chunks_.resize(new_len);

    // TODO: walk by array coordinates
    for (int loc_y = -min_radius; loc_y <= min_radius; ++loc_y)
    {
        for (int loc_x = -min_radius; loc_x <= min_radius; ++loc_x)
        {
            if (collapse && math::isOutsideRadius(loc_x, loc_y, radius))
            {
                continue;
            }

            UPtr<Chunk> &old = get_chunk_by_loc_pos(chunks_old_, radius_, glm::ivec2{loc_x, loc_y});
            UPtr<Chunk> &neww = get_chunk_by_loc_pos(chunks_, radius, glm::ivec2{loc_x, loc_y});
            neww = std::move(old);
        }
    }

    if (collapse)
    {
        for (UPtr<Chunk> &old : chunks_old_)
        {
            if (old && unload_callback_)
            {
                unload_callback_(std::move(old));
            }
            assert(!old);
        }
    }

    radius_ = radius;
    chunks_old_.resize(new_len);

    assert(check_buf_empty());
    assert(check_sizes());
}

int ChunksMap::getRadius() const
{
    return radius_;
}

void ChunksMap::setCenter(glm::ivec2 center)
{
    if (center_chunk_pos_ == center)
    {
        return;
    }

    assert(check_sizes());

    assert(check_buf_empty());
    std::swap(chunks_, chunks_old_);

    const glm::ivec2 delta = center - center_chunk_pos_;

    const glm::ivec2 radius_vec{radius_, radius_};

    glm::ivec2 old_top_right_in_new_coords = radius_vec - delta;
    glm::ivec2 old_bottom_left_in_new_coords = -radius_vec - delta;

    if (old_top_right_in_new_coords.x >= -radius_ && old_top_right_in_new_coords.y >= -radius_
        && old_bottom_left_in_new_coords.x <= radius_ && old_bottom_left_in_new_coords.y <= radius_)
    {
        old_top_right_in_new_coords = glm::min(old_top_right_in_new_coords, radius_vec);
        old_bottom_left_in_new_coords = glm::max(old_bottom_left_in_new_coords, -radius_vec);

        for (int loc_y = old_bottom_left_in_new_coords.y; loc_y <= old_top_right_in_new_coords.y;
             ++loc_y)
        {
            for (int loc_x = old_bottom_left_in_new_coords.x;
                 loc_x <= old_top_right_in_new_coords.x; ++loc_x)
            {
                const glm::ivec2 pos{loc_x, loc_y};
                UPtr<Chunk> &old = get_chunk_by_loc_pos(chunks_old_, radius_, pos + delta);
                UPtr<Chunk> &neww = get_chunk_by_loc_pos(chunks_, radius_, pos);
                neww = std::move(old);
            }
        }
    }

    for (UPtr<Chunk> &old : chunks_old_)
    {
        if (old && unload_callback_)
        {
            unload_callback_(std::move(old));
        }
        assert(!old);
    }

    assert(check_buf_empty());

    center_chunk_pos_ = center;
}

glm::vec2 ChunksMap::getCenter() const
{
    return center_chunk_pos_;
}

bool ChunksMap::isValidPos(glm::ivec2 pos) const
{
    const glm::ivec2 loc_pos = pos - center_chunk_pos_;
    return is_valid_pos(radius_, loc_pos);
}

Chunk *ChunksMap::getChunkUnsafe(glm::ivec2 pos) const
{
    const glm::ivec2 loc_pos = pos - center_chunk_pos_;
    const UPtr<Chunk> &c = get_chunk_by_loc_pos(chunks_, radius_, loc_pos);
    return c.get();
}

bool ChunksMap::hasChunkUnsafe(glm::ivec2 pos) const
{
    return getChunkUnsafe(pos) != nullptr;
}

void ChunksMap::setChunkUnsafe(glm::ivec2 pos, UPtr<Chunk> chunk)
{
    assert(!hasChunkUnsafe(pos));
    const glm::ivec2 loc_pos = pos - center_chunk_pos_;
    UPtr<Chunk> &c = get_chunk_by_loc_pos(chunks_, radius_, loc_pos);

    chunk->setPosition(glm::ivec3{pos.x, 0, pos.y});
    c = std::move(chunk);
}

UPtr<Chunk> ChunksMap::takeChunkUnsafe(glm::ivec2 pos)
{
    const glm::ivec2 loc_pos = pos - center_chunk_pos_;
    UPtr<Chunk> &c = get_chunk_by_loc_pos(chunks_, radius_, loc_pos);
    return std::move(c);
}

Chunk *ChunksMap::getChunk(glm::ivec2 pos) const
{
    const glm::ivec2 loc_pos = pos - center_chunk_pos_;
    if (!is_valid_pos(radius_, loc_pos))
    {
        return nullptr;
    }
    const UPtr<Chunk> &c = get_chunk_by_loc_pos(chunks_, radius_, loc_pos);
    return c.get();
}

bool ChunksMap::hasChunk(glm::ivec2 pos) const
{
    return getChunk(pos) != nullptr;
}

UPtr<Chunk> ChunksMap::takeChunk(glm::ivec2 pos)
{
    const glm::ivec2 loc_pos = pos - center_chunk_pos_;
    if (!is_valid_pos(radius_, loc_pos))
    {
        return nullptr;
    }
    UPtr<Chunk> &c = get_chunk_by_loc_pos(chunks_, radius_, loc_pos);
    return std::move(c);
}

void ChunksMap::setUnloadCallback(UnloadCallback callback)
{
    unload_callback_ = std::move(callback);
}

void ChunksMap::clearUnloadCallback()
{
    unload_callback_ = nullptr;
}

bool ChunksMap::check_sizes() const
{
    const int len = arr_length_from_radius(radius_);
    return chunks_.size() == len && chunks_old_.size() == len;
}

bool ChunksMap::check_buf_empty() const
{
    for (const UPtr<Chunk> &b : chunks_old_)
    {
        if (b)
        {
            return false;
        }
    }
    return true;
}
