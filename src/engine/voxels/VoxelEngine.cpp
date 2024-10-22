#include "VoxelEngine.h"

// clang-format off
#include "glad/glad.h"
// clang-format on

#include "BasicBlocks.h"
#include "BlockInfo.h"
#include "BlocksRegistry.h"
#include "Camera.h"
#include "Chunk.h"
#include "EngineGlobals.h"
#include "Shader.h"
#include "ShaderSource.h"
#include "TextureManager.h"
#include "VertexArrayObject.h"

VoxelEngine::VoxelEngine() = default;

VoxelEngine::~VoxelEngine() = default;

void VoxelEngine::init()
{
    registry_ = makeU<BlocksRegistry>();
    register_blocks();

    Texture *atlas = eng.texture_manager->create("atlas");
    atlas->load("vox/atlas.png", Texture::Format::RGBA, Texture::Wrap::ClampToEdge,
        Texture::Filter::Nearest, Texture::Filter::Nearest, Texture::FlipMode::FlipY);

    registry_->setAtlas(atlas, glm::ivec2(16, 16));
    registry_->flush();

    // shader
    shader_source_ = makeU<ShaderSource>();
    shader_source_->setFile("vox/vox.shader");

    shader_ = makeU<Shader>();
    shader_->setSource(shader_source_.get());
    shader_->recompile();
}

void VoxelEngine::update(const glm::vec3 &position)
{
    const auto pos_to_chunk = [](const glm::vec2 pos_xz) {
        const auto x = pos_xz[0] / Chunk::CHUNK_WIDTH;
        const auto z = pos_xz[1] / Chunk::CHUNK_WIDTH;
        return glm::ivec2{x, z};
    };

    const auto has_chunk = [&](glm::ivec2 pos) {
        for (const UPtr<Chunk> &c : chunks_)
        {
            if (c->position_ == pos)
            {
                return true;
            }
        }
        return false;
    };

    const glm::ivec2 chunk_pos = pos_to_chunk(position);
    if (!has_chunk(chunk_pos))
    {
        UPtr<Chunk> new_chunk = generate_chunk(chunk_pos);
        chunks_.push_back(std::move(new_chunk));
    }
}

void VoxelEngine::render(Camera *camera)
{
    GL_CHECKED(glEnable(GL_BLEND));
    GL_CHECKED(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

    GL_CHECKED(glEnable(GL_DEPTH_TEST));
    GL_CHECKED(glDepthMask(GL_TRUE));

    GL_CHECKED(glCullFace(GL_BACK));

    GL_CHECKED(glEnable(GL_CULL_FACE));

    assert(!shader_->isDirty());
    shader_->bind();
    shader_->setUniformMat4("uViewProj", camera->getViewProj());

    // TODO# CACHE
    const int atlas_loc = shader_->getUniformLocation("atlas");
    assert(atlas_loc != -1);
    constexpr int atlas_index = 0;
    registry_->getAtlas()->bind(atlas_index);
    shader_->setUniformInt(atlas_loc, atlas_index);

    // TODO# culling, sort by distance (nearest first)
    for (const UPtr<Chunk> &chunk : chunks_)
    {
        chunk->flush();
        chunk->vao_->bind();
        GL_CHECKED(glDrawArrays(GL_TRIANGLES, 0, chunk->vbo_->getNumVertices()));
        eng.stat.addRenderedIndices(chunk->vbo_->getNumVertices());
    }
}

void VoxelEngine::register_blocks()
{
    BlocksRegistry &reg = *registry_;

    {
        BlockDescription &b = reg.addBlock();
        BasicBlocks::AIR = b.id;
        b.name = "Air";
        b.type = BlockType::AIR;
        b.texture_index_px = 0;
        b.texture_index_nx = 0;
        b.texture_index_py = 0;
        b.texture_index_ny = 0;
        b.texture_index_pz = 0;
        b.texture_index_nz = 0;
    }
    {
        BlockDescription &b = reg.addBlock();
        BasicBlocks::DIRT = b.id;
        b.name = "Dirt";
        b.type = BlockType::SOLID;
        b.texture_index_px = 2;
        b.texture_index_nx = 2;
        b.texture_index_py = 2;
        b.texture_index_ny = 2;
        b.texture_index_pz = 2;
        b.texture_index_nz = 2;
    }
    {
        BlockDescription &b = reg.addBlock();
        BasicBlocks::GRASS = b.id;
        b.name = "Grass";
        b.type = BlockType::SOLID;
        b.texture_index_px = 1;
        b.texture_index_nx = 1;
        b.texture_index_py = 0;
        b.texture_index_ny = 2;
        b.texture_index_pz = 1;
        b.texture_index_nz = 1;
    }
}

UPtr<Chunk> VoxelEngine::generate_chunk(glm::vec2 pos)
{
    UPtr<Chunk> chunk = makeU<Chunk>(pos);

    return chunk;
}
