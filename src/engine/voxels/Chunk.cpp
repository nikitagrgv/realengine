#include "Chunk.h"

#include "BasicBlocks.h"
#include "BlockDescription.h"
#include "BlocksRegistry.h"
#include "EngineGlobals.h"
#include "VertexArrayObject.h"
#include "VoxelEngine.h"
#include "profiler/ScopedTimer.h"

Chunk::Chunk(glm::ivec3 position)
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
    vao_->addAttributeFloat(3); // norm
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

    ScopedTimer timer("Flush chunk");

    vbo_->clear();

    BlocksRegistry *registry = eng.vox->getRegistry();

    const auto is_air_or_outside = [&](int x, int y, int z) {
        if (!isInsideChunk(x, y, z))
        {
            return true;
        }
        const BlockInfo block = getBlock(x, y, z);
        if (block.id == 0)
        {
            return true;
        }
        // TODO# check transparent
        return false;
    };

    visitRead([&](int x, int y, int z, BlockInfo block) {
        if (block.id == BasicBlocks::AIR)
        {
            return;
        }

        const glm::vec3 min = glm::vec3{x, y, z};
        const glm::vec3 max = min + glm::vec3(1, 1, 1);

        const BlockDescription &desc = registry->getBlock(block.id);
        assert(desc.cached.valid);

        if (is_air_or_outside(x + 1, y, z))
        {
            gen_face_px(min, max, desc);
        }
        if (is_air_or_outside(x - 1, y, z))
        {
            gen_face_nx(min, max, desc);
        }
        if (is_air_or_outside(x, y + 1, z))
        {
            gen_face_py(min, max, desc);
        }
        if (is_air_or_outside(x, y - 1, z))
        {
            gen_face_ny(min, max, desc);
        }
        if (is_air_or_outside(x, y, z + 1))
        {
            gen_face_pz(min, max, desc);
        }
        if (is_air_or_outside(x, y, z - 1))
        {
            gen_face_nz(min, max, desc);
        }
    });

    vbo_->flush();
}

void Chunk::gen_face_py(const glm::vec3 &min, const glm::vec3 &max, const BlockDescription &desc)
{
    const BlockDescription::TexCoords &coords = desc.cached.texture_coord_py;
    Vertex v;

    v.norm_ = glm::vec3{0, 1, 0};

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

    v.norm_ = glm::vec3{0, -1, 0};

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

    v.norm_ = glm::vec3{0, 0, 1};

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

    v.norm_ = glm::vec3{0, 0, -1};

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

    v.norm_ = glm::vec3{1, 0, 0};

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

    v.norm_ = glm::vec3{-1, 0, 0};

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
