#pragma once

#include "Base.h"
#include "BlockInfo.h"
#include "VertexBufferObject.h"

#include "glm/vec2.hpp"

struct BlockDescription;
class VertexArrayObject;

// Blocks order in memory: XZY
struct Chunk
{
public:
    static constexpr int CHUNK_WIDTH = 32;
    static constexpr int CHUNK_HEIGHT = 128;

    static constexpr int CHUNK_WIDTH2 = CHUNK_WIDTH * CHUNK_WIDTH;
    static constexpr int NUM_BLOCKS = CHUNK_WIDTH2 * CHUNK_HEIGHT;

public:
    explicit Chunk(glm::ivec2 position);

    template<typename F>
    void visitRead(F &&func) const
    {
        int block_index = -1;
        for (int y = 0; y < CHUNK_HEIGHT; ++y)
        {
            for (int z = 0; z < CHUNK_WIDTH; ++z)
            {
                for (int x = 0; x < CHUNK_WIDTH; ++x)
                {
                    ++block_index;
                    assert(block_index == getBlockIndex(x, y, z));
                    const BlockInfo &b = blocks_[block_index];
                    func(x, y, z, b);
                }
            }
        }
    }

    REALENGINE_INLINE const BlockInfo &getBlock(int x, int y, int z) const
    {
        return blocks_[getBlockIndex(x, y, z)];
    }

    REALENGINE_INLINE void setBlock(int x, int y, int z, const BlockInfo &block)
    {
        dirty_ = true; // TODO# REMOVE?
        blocks_[getBlockIndex(x, y, z)] = block;
    }

    static REALENGINE_INLINE int getBlockIndex(int x, int y, int z)
    {
        return x + CHUNK_WIDTH * z + CHUNK_WIDTH2 * y;
    }

    void flush();

private:
    void gen_face_py(const glm::vec3 &min, const glm::vec3 &max, const BlockDescription &desc);
    void gen_face_ny(const glm::vec3 &min, const glm::vec3 &max, const BlockDescription &desc);
    void gen_face_pz(const glm::vec3 &min, const glm::vec3 &max, const BlockDescription &desc);
    void gen_face_nz(const glm::vec3 &min, const glm::vec3 &max, const BlockDescription &desc);
    void gen_face_px(const glm::vec3 &min, const glm::vec3 &max, const BlockDescription &desc);
    void gen_face_nx(const glm::vec3 &min, const glm::vec3 &max, const BlockDescription &desc);

public:
    bool dirty_{true};

    glm::ivec2 position_{0, 0};

    struct Vertex
    {
        glm::vec3 pos_;
        glm::vec2 uv_;
    };
    UPtr<VertexArrayObject> vao_;
    UPtr<VertexBufferObject<Vertex>> vbo_;

    BlockInfo blocks_[NUM_BLOCKS];
};
