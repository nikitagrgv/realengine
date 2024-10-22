#include "Chunk.h"

#include "BasicBlocks.h"
#include "BlockDescription.h"
#include "BlocksRegistry.h"
#include "EngineGlobals.h"
#include "VertexArrayObject.h"
#include "VoxelEngine.h"

Chunk::Chunk(glm::ivec2 position)
{
    position_ = position;
    for (BlockInfo &b : blocks_)
    {
        b.id = BasicBlocks::AIR;
    }

    vao_ = makeU<VertexArrayObject>();
    vbo_ = makeU<VertexBufferObject<Vertex>>();

    vao_->bind();
    vao_->addAttributeFloat(3); // pos
    vao_->addAttributeFloat(2); // uv
    vbo_->bind();
    vao_->flush();
    vbo_->flush();
}

void Chunk::flush()
{
    if (!dirty_)
    {
        return;
    }
    dirty_ = false;

    vbo_->clear();

    BlocksRegistry *registry = eng.vox->getRegistry();

    auto generate_block = [&](BlockInfo block, const glm::vec3 &pos) {
        const glm::vec3 min = pos;
        const glm::vec3 max = min + glm::vec3(1, 1, 1);

        const BlockDescription &desc = registry->getBlock(block.id);
        assert(desc.cached.valid);

        gen_face_py(min, max, desc);
        gen_face_ny(min, max, desc);
        gen_face_pz(min, max, desc);
        gen_face_nz(min, max, desc);
        gen_face_px(min, max, desc);
        gen_face_nx(min, max, desc);
    };


    int block_index = -1;
    for (int y = 0; y < CHUNK_HEIGHT; ++y)
    {
        for (int z = 0; z < CHUNK_WIDTH; ++z)
        {
            for (int x = 0; x < CHUNK_WIDTH; ++x)
            {
                ++block_index;
                assert(block_index == getBlockIndex(x, y, z));
                const BlockInfo &block = blocks_[block_index];
                if (block.id == BasicBlocks::AIR)
                {
                    continue;
                }
                generate_block(block, glm::vec3{x, y, z});
            }
        }
    }

    for (const BlockInfo &b : blocks_)
    {
        if (b.id == BasicBlocks::AIR)
        {
            continue;
        }
        generate_block(b, glm::vec3{0, 0, 0});
    }

    vbo_->flush();
}

void Chunk::gen_face_py(const glm::vec3 &min, const glm::vec3 &max, const BlockDescription &desc)
{
    const BlockDescription::TexCoords &coords = desc.cached.texture_coord_py;
    Vertex v;

    // tr 1
    v.pos_ = glm::vec3{min.x, max.y, min.z};
    v.uv_ = coords.top_left;
    vbo_->addVertex(v);

    v.pos_ = glm::vec3{min.x, max.y, max.z};
    v.uv_ = coords.bottom_left;
    vbo_->addVertex(v);

    v.pos_ = glm::vec3{max.x, max.y, max.z};
    v.uv_ = coords.bottom_right;
    vbo_->addVertex(v);

    // tr 2
    v.pos_ = glm::vec3{min.x, max.y, min.z};
    v.uv_ = coords.top_left;
    vbo_->addVertex(v);

    v.pos_ = glm::vec3{max.x, max.y, max.z};
    v.uv_ = coords.bottom_right;
    vbo_->addVertex(v);

    v.pos_ = glm::vec3{max.x, max.y, min.z};
    v.uv_ = coords.top_right;
    vbo_->addVertex(v);
}

void Chunk::gen_face_ny(const glm::vec3 &min, const glm::vec3 &max, const BlockDescription &desc)
{
    const BlockDescription::TexCoords &coords = desc.cached.texture_coord_ny;
    Vertex v;

    // tr 1
    v.pos_ = glm::vec3{min.x, min.y, min.z};
    v.uv_ = coords.top_right;
    vbo_->addVertex(v);

    v.pos_ = glm::vec3{max.x, min.y, max.z};
    v.uv_ = coords.bottom_left;
    vbo_->addVertex(v);

    v.pos_ = glm::vec3{min.x, min.y, max.z};
    v.uv_ = coords.bottom_right;
    vbo_->addVertex(v);


    // tr 2
    v.pos_ = glm::vec3{min.x, min.y, min.z};
    v.uv_ = coords.top_right;
    vbo_->addVertex(v);

    v.pos_ = glm::vec3{max.x, min.y, min.z};
    v.uv_ = coords.top_left;
    vbo_->addVertex(v);

    v.pos_ = glm::vec3{max.x, min.y, max.z};
    v.uv_ = coords.bottom_left;
    vbo_->addVertex(v);
}

void Chunk::gen_face_pz(const glm::vec3 &min, const glm::vec3 &max, const BlockDescription &desc)
{
    const BlockDescription::TexCoords &coords = desc.cached.texture_coord_pz;
    Vertex v;

    // tr 1
    v.pos_ = glm::vec3{min.x, max.y, max.z};
    v.uv_ = coords.top_left;
    vbo_->addVertex(v);

    v.pos_ = glm::vec3{min.x, min.y, max.z};
    v.uv_ = coords.bottom_left;
    vbo_->addVertex(v);

    v.pos_ = glm::vec3{max.x, min.y, max.z};
    v.uv_ = coords.bottom_right;
    vbo_->addVertex(v);

    // tr 2
    v.pos_ = glm::vec3{min.x, max.y, max.z};
    v.uv_ = coords.top_left;
    vbo_->addVertex(v);

    v.pos_ = glm::vec3{max.x, min.y, max.z};
    v.uv_ = coords.bottom_right;
    vbo_->addVertex(v);

    v.pos_ = glm::vec3{max.x, max.y, max.z};
    v.uv_ = coords.top_right;
    vbo_->addVertex(v);
}

void Chunk::gen_face_nz(const glm::vec3 &min, const glm::vec3 &max, const BlockDescription &desc)
{
    const BlockDescription::TexCoords &coords = desc.cached.texture_coord_nz;
    Vertex v;

    // tr 1
    v.pos_ = glm::vec3{min.x, max.y, min.z};
    v.uv_ = coords.top_right;
    vbo_->addVertex(v);

    v.pos_ = glm::vec3{max.x, min.y, min.z};
    v.uv_ = coords.bottom_left;
    vbo_->addVertex(v);

    v.pos_ = glm::vec3{min.x, min.y, min.z};
    v.uv_ = coords.bottom_right;
    vbo_->addVertex(v);

    // tr 2
    v.pos_ = glm::vec3{min.x, max.y, min.z};
    v.uv_ = coords.top_right;
    vbo_->addVertex(v);

    v.pos_ = glm::vec3{max.x, max.y, min.z};
    v.uv_ = coords.top_left;
    vbo_->addVertex(v);

    v.pos_ = glm::vec3{max.x, min.y, min.z};
    v.uv_ = coords.bottom_left;
    vbo_->addVertex(v);
}

void Chunk::gen_face_px(const glm::vec3 &min, const glm::vec3 &max, const BlockDescription &desc)
{
    const BlockDescription::TexCoords &coords = desc.cached.texture_coord_px;
    Vertex v;

    // tr 1
    v.pos_ = glm::vec3{max.x, max.y, max.z};
    v.uv_ = coords.top_left;
    vbo_->addVertex(v);

    v.pos_ = glm::vec3{max.x, min.y, max.z};
    v.uv_ = coords.bottom_left;
    vbo_->addVertex(v);

    v.pos_ = glm::vec3{max.x, min.y, min.z};
    v.uv_ = coords.bottom_right;
    vbo_->addVertex(v);

    // tr 2
    v.pos_ = glm::vec3{max.x, max.y, max.z};
    v.uv_ = coords.top_left;
    vbo_->addVertex(v);

    v.pos_ = glm::vec3{max.x, min.y, min.z};
    v.uv_ = coords.bottom_right;
    vbo_->addVertex(v);

    v.pos_ = glm::vec3{max.x, max.y, min.z};
    v.uv_ = coords.top_right;
    vbo_->addVertex(v);
}

void Chunk::gen_face_nx(const glm::vec3 &min, const glm::vec3 &max, const BlockDescription &desc)
{
    const BlockDescription::TexCoords &coords = desc.cached.texture_coord_nx;
    Vertex v;

    // tr 1
    v.pos_ = glm::vec3{min.x, max.y, max.z};
    v.uv_ = coords.top_right;
    vbo_->addVertex(v);

    v.pos_ = glm::vec3{min.x, min.y, min.z};
    v.uv_ = coords.bottom_left;
    vbo_->addVertex(v);

    v.pos_ = glm::vec3{min.x, min.y, max.z};
    v.uv_ = coords.bottom_right;
    vbo_->addVertex(v);

    // tr 2
    v.pos_ = glm::vec3{min.x, max.y, max.z};
    v.uv_ = coords.top_right;
    vbo_->addVertex(v);

    v.pos_ = glm::vec3{min.x, max.y, min.z};
    v.uv_ = coords.top_left;
    vbo_->addVertex(v);

    v.pos_ = glm::vec3{min.x, min.y, min.z};
    v.uv_ = coords.bottom_left;
    vbo_->addVertex(v);
}
