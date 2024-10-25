#include "VoxelEngine.h"

// clang-format off
#include <glad/glad.h>
// clang-format on

#include "BasicBlocks.h"
#include "BlockInfo.h"
#include "BlocksRegistry.h"
#include "Camera.h"
#include "Chunk.h"
#include "EngineGlobals.h"
#include "GlobalLight.h"
#include "Shader.h"
#include "ShaderSource.h"
#include "TextureManager.h"
#include "VertexArrayObject.h"
#include "math/Math.h"
#include "profiler/ScopedProfiler.h"
#include "profiler/ScopedTimer.h"

#include <PerlinNoise.hpp>
#include <glm/ext/matrix_transform.hpp>

struct VoxelEngine::Perlin : siv::PerlinNoise
{
    explicit Perlin(seed_type seed)
        : siv::PerlinNoise(seed)
    {}
};

VoxelEngine::VoxelEngine() = default;

VoxelEngine::~VoxelEngine() = default;

void VoxelEngine::init()
{
    perlin_ = makeU<Perlin>(seed_);

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
    SCOPED_PROFILER;

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
    constexpr int RADIUS = 2;
    for (int z = chunk_pos.z - RADIUS, z_end = chunk_pos.z + RADIUS; z <= z_end; ++z)
    {
        for (int x = chunk_pos.x - RADIUS, x_end = chunk_pos.x + RADIUS; x <= x_end; ++x)
        {
            if (!has_chunk(glm::ivec3{x, 0, z}))
            {
                UPtr<Chunk> new_chunk = generate_chunk(glm::ivec3{x, 0, z});
                chunks_.push_back(std::move(new_chunk));
            }
        }
    }
}

void VoxelEngine::render(Camera *camera, GlobalLight *light)
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
    assert(math::isNormalized(light->dir));
    assert(shader_->getUniformLocation("uLight.dir") != -1);
    shader_->setUniformVec3("uLight.dir", light->dir);

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

void VoxelEngine::setSeed(unsigned int seed)
{
    seed_ = seed;
    perlin_ = makeU<Perlin>(seed_);
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
    {
        BlockDescription &b = reg.addBlock();
        BasicBlocks::STONE = b.id;
        b.name = "Stone";
        b.type = BlockType::SOLID;
        b.texture_index_px = 3;
        b.texture_index_nx = 3;
        b.texture_index_py = 3;
        b.texture_index_ny = 3;
        b.texture_index_pz = 3;
        b.texture_index_nz = 3;
    }

    assert(BasicBlocks::AIR == 0);
}

UPtr<Chunk> VoxelEngine::generate_chunk(glm::vec3 pos)
{
    ScopedTimer timer("Generate chunk");

    assert(perlin_);
    const siv::PerlinNoise &perlin = *perlin_;

    UPtr<Chunk> chunk = makeU<Chunk>(pos);

    int height_map[Chunk::CHUNK_WIDTH][Chunk::CHUNK_WIDTH];

    constexpr float FACTOR = 0.007f;
    constexpr int MIN = 10;
    constexpr int MAX = Chunk::CHUNK_HEIGHT - 120;
    constexpr int HEIGHT_DIFF = MAX - MIN;
    static_assert(MIN < MAX);

    const int offset_x = chunk->getBlocksOffsetX();
    const int offset_z = chunk->getBlocksOffsetZ();

    for (int z = 0; z < Chunk::CHUNK_WIDTH; ++z)
    {
        for (int x = 0; x < Chunk::CHUNK_WIDTH; ++x)
        {
            const float z_n = (float)(z + offset_z) * FACTOR;
            const float x_n = (float)(x + offset_x) * FACTOR;
            const float height_norm = perlin.octave2D_01(z_n, x_n, 5, 0.4);
            const int height = (int)(height_norm * (float)HEIGHT_DIFF + (float)MIN);
            height_map[z][x] = height;
        }
    }

    glm::vec3 offset;
    offset.x = chunk->getBlocksOffsetX();
    offset.y = 0;
    offset.z = chunk->getBlocksOffsetZ();
    chunk->visitWrite([&](int x, int y, int z, BlockInfo &block) {
        const int cur_height = height_map[z][x];
        const int diff = y - cur_height;

        if (diff <= 0)
        {
            glm::vec3 norm_pos = (offset + glm::vec3{x, y, z}) * 0.02f;
            const auto noise = perlin.octave3D_01(norm_pos.x, norm_pos.y, norm_pos.z, 4);
            if (noise > 0.8f)
            {
                block = BlockInfo(BasicBlocks::AIR);
                return;
            }
        }

        if (diff > 0)
        {
            block = BlockInfo(BasicBlocks::AIR);
        }
        else if (diff == 0)
        {
            block = BlockInfo(BasicBlocks::GRASS);
        }
        else if (diff > -4)
        {
            block = BlockInfo(BasicBlocks::DIRT);
        }
        else
        {
            block = BlockInfo(BasicBlocks::STONE);
        }
    });

    return chunk;
}
