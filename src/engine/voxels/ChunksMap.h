#pragma once

#include "Base.h"
#include "Chunk.h"
#include "signals/Signals.h"

#include <functional>
#include <vector>

// XXX0XXX - radius=3!
class ChunksMap
{
public:
    using UnloadCallback = std::function<void(UPtr<Chunk>)>;

public:
    ChunksMap();

    void setRadius(int radius);
    int getRadius() const;

    void setCenter(glm::ivec2 center);
    glm::vec2 getCenter() const;

    Chunk *getChunk(glm::ivec2 pos) const;
    bool hasChunk(glm::ivec2 pos) const;
    void setChunk(glm::ivec2 pos, UPtr<Chunk> chunk);
    UPtr<Chunk> takeChunk(glm::ivec2 pos);

    void setUnloadCallback(UnloadCallback callback);
    void clearUnloadCallback();

private:
    bool check_sizes() const;
    bool check_buf_empty() const;

private:
    int radius_{0};
    glm::ivec2 center_chunk_pos_{};
    glm::ivec2 center_chunk_in_vec_{};
    std::vector<UPtr<Chunk>> chunks_;
    std::vector<UPtr<Chunk>> chunks_old_;
    UnloadCallback unload_callback_;
};
