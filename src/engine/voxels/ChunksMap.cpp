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

REALENGINE_INLINE int get_index(int radius, glm::ivec2 pos)
{
    assert(std::abs(pos.x) <= radius && std::abs(pos.y) <= radius);
    return (pos.x + radius) + (pos.y + radius) * radius;
}

REALENGINE_INLINE UPtr<Chunk> &get_chunk_by_loc_pos(std::vector<UPtr<Chunk>> &chunks, int radius,
    glm::ivec2 pos)
{
    assert(chunks.size() == arr_length_from_radius(radius));
    const int index = get_index(radius, pos);
    return chunks[index];
}

REALENGINE_INLINE const UPtr<Chunk> &get_chunk_by_loc_pos(const std::vector<UPtr<Chunk>> &chunks,
    int radius, glm::ivec2 pos)
{
    assert(chunks.size() == arr_length_from_radius(radius));
    const int index = get_index(radius, pos);
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
    assert(check_buf_empty());

    radius_ = radius;
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
    center_chunk_pos_ = center;






}

glm::vec2 ChunksMap::getCenter() const
{
    return center_chunk_pos_;
}

Chunk *ChunksMap::getChunk(glm::ivec2 pos) const
{
    const glm::ivec2 loc_pos = pos - center_chunk_pos_;
    const UPtr<Chunk> &c = get_chunk_by_loc_pos(chunks_, radius_, loc_pos);
    return c.get();
}

bool ChunksMap::hasChunk(glm::ivec2 pos) const
{
    return getChunk(pos) != nullptr;
}

void ChunksMap::setChunk(glm::ivec2 pos, UPtr<Chunk> chunk)
{
    assert(!hasChunk(pos));
    const glm::ivec2 loc_pos = pos - center_chunk_pos_;
    UPtr<Chunk> &c = get_chunk_by_loc_pos(chunks_, radius_, loc_pos);

    chunk->setPosition(glm::ivec3{pos.x, 0, pos.y});
    c = std::move(chunk);
}

UPtr<Chunk> ChunksMap::takeChunk(glm::ivec2 pos)
{
    const glm::ivec2 loc_pos = pos - center_chunk_pos_;
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
