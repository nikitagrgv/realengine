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

#include "glm/ext/matrix_transform.hpp"

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
    const auto pos_to_chunk = [](const glm::vec3 pos) {
        const auto x = std::floor(pos.x / Chunk::CHUNK_WIDTH);
        const auto y = 0;
        const auto z = std::floor(pos.z / Chunk::CHUNK_WIDTH);
        return glm::ivec3{x, y, z};
    };

    const auto has_chunk = [&](glm::ivec3 pos) {
        for (const UPtr<Chunk> &c : chunks_)
        {
            if (c->position_.x == pos.x && c->position_.z == pos.z)
            {
                return true;
            }
        }
        return false;
    };

    const glm::ivec3 chunk_pos = pos_to_chunk(position);
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

    // TODO# CACHE
    const int model_view_proj_loc = shader_->getUniformLocation("uModelViewProj");
    assert(model_view_proj_loc != -1);

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

        glm::vec3 glob_position{0.0f};
        glob_position.x = chunk->position_.x * Chunk::CHUNK_WIDTH;
        glob_position.y = 0.0f;
        glob_position.z = chunk->position_.z * Chunk::CHUNK_WIDTH;

        auto value = camera->getViewProj() * glm::translate(glm::mat4{1.0f}, glob_position);
        shader_->setUniformMat4(model_view_proj_loc, value);

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

    assert(BasicBlocks::AIR == 0);
}

UPtr<Chunk> VoxelEngine::generate_chunk(glm::vec3 pos)
{
    UPtr<Chunk> chunk = makeU<Chunk>(pos);

    chunk->visitWriteGlobal([](int x, int y, int z, BlockInfo &block) {
        const float xx = (float)x / 14.f;
        const float zz = (float)z / 14.f;
        const float s = std::sin(xx + zz);
        const int bound = int((s + 1.2) * 6);
        if (y < bound)
        {
            block = BlockInfo(BasicBlocks::DIRT);
        }
        else if (y == bound)
        {
            block = BlockInfo(BasicBlocks::GRASS);
        }
    });

    return chunk;
}
