#pragma once

#include "Base.h"
#include "Chunk.h"
#include "signals/Signals.h"

#include <vector>

class ChunksMap
{
public:
    using UnloadCallback = Callback<glm::ivec2, UPtr<Chunk>>;

public:
    void setRadius(int radius);
    int getRadius() const;

    void setCenter(glm::ivec2 center);
    void getCenter() const;

    Chunk *getChunk(glm::ivec2 pos) const;
    bool hasChunk(glm::ivec2 pos) const;
    void setChunk(glm::ivec2 pos, UPtr<Chunk> chunk) const;
    Chunk *takeChunk(glm::ivec2 pos);

    void setUnloadCallback(UnloadCallback callback);
    void clearUnloadCallback();

private:


private:
    int radius{0};
    glm::ivec2 center_chunk_pos_{};
    glm::ivec2 center_chunk_in_vec_{};
    std::vector<UPtr<Chunk>> chunks_;
    UnloadCallback unload_callback_;
};
