#include "VoxelEngine.h"

// clang-format off
#include <glad/glad.h>
// clang-format on

#include "BasicBlocks.h"
#include "BlockInfo.h"
#include "BlocksRegistry.h"
#include "Camera.h"
#include "Chunk.h"
#include "ChunkMesh.h"
#include "ChunkMeshGenerator.h"
#include "Common.h"
#include "EngineGlobals.h"
#include "GlobalLight.h"
#include "Shader.h"
#include "ShaderSource.h"
#include "TextureManager.h"
#include "VertexArrayObject.h"
#include "math/Math.h"
#include "profiler/ScopedProfiler.h"
#include "profiler/ScopedTimer.h"
#include "utils/Algos.h"

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
    // TODO# generate mip maps! but custom
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

    const glm::ivec3 base_chunk_pos = pos_to_chunk_pos(position);

    constexpr int RADIUS_SPAWN_CHUNK = 3;
    constexpr int RADIUS_UNLOAD_MESH = 5;
    constexpr int RADIUS_UNLOAD_WHOLE_CHUNK = 20;

    static_assert(RADIUS_UNLOAD_WHOLE_CHUNK > RADIUS_UNLOAD_MESH
            && RADIUS_UNLOAD_MESH > RADIUS_SPAWN_CHUNK,
        "Invalid radiuses");

    const auto is_outside_radius = [&](const Chunk &chunk, int radius) {
        const glm::ivec3 pos = chunk.position_;
        const int max_radius = std::max(std::abs(base_chunk_pos.z - pos.z),
            std::abs(base_chunk_pos.x - pos.x));
        return max_radius > radius;
    };

    bool chunks_dirty = false;

    {
        ScopedProfiler p("Unload chunks");

        // Unload whole chunks outside radius
        // TODO# save in file or compress
        const int old_size = chunks_.size();
        Alg::removeIf(chunks_, [&](const UPtr<Chunk> &chunk) {
            return is_outside_radius(*chunk, RADIUS_UNLOAD_WHOLE_CHUNK);
        });
        if (old_size != chunks_.size())
        {
            chunks_dirty = true;
        }
    }

    {
        ScopedProfiler p("Unload meshes");

        // Unload meshes outside radius
        for (const UPtr<Chunk> &chunk : chunks_)
        {
            if (!chunk->mesh_)
            {
                continue;
            }
            if (is_outside_radius(*chunk, RADIUS_UNLOAD_MESH))
            {
                release_mesh(std::move(chunk->mesh_));
            }
        }
    }

    static std::vector<Chunk *> chunks_to_generate;
    chunks_to_generate.clear();

    {
        ScopedProfiler p("Init new chunks");

        for (int z = base_chunk_pos.z - RADIUS_SPAWN_CHUNK,
                 z_end = base_chunk_pos.z + RADIUS_SPAWN_CHUNK;
             z <= z_end; ++z)
        {
            for (int x = base_chunk_pos.x - RADIUS_SPAWN_CHUNK,
                     x_end = base_chunk_pos.x + RADIUS_SPAWN_CHUNK;
                 x <= x_end; ++x)
            {
                if (has_chunk_at_pos(x, z))
                {
                    continue;
                }
                UPtr<Chunk> new_chunk = makeU<Chunk>(glm::ivec3{x, 0, z});
                chunks_to_generate.push_back(new_chunk.get());
                chunks_.push_back(std::move(new_chunk));
                chunks_dirty = true;
            }
        }
    }

    if (chunks_dirty)
    {
        refresh_chunk_index_by_pos();
    }

    {
        ScopedProfiler p("Generate new chunks");

        for (Chunk *chunk : chunks_to_generate)
        {
            generate_chunk(*chunk);
        }
        chunks_to_generate.clear();
    }

    {
        ScopedProfiler p("Generate/unload meshes");

        // Generate/unload meshes for chunks according to neighbours chunks
        for (const UPtr<Chunk> &chunk : chunks_)
        {
            NeighbourChunks neighbours = get_neighbour_chunks_lazy(chunk.get());

            if (!neighbours.hasAll())
            {
                if (chunk->mesh_)
                {
                    release_mesh(std::move(chunk->mesh_));
                }
                continue;
            }

            // TODO: remove this check
            if (is_outside_radius(*chunk, RADIUS_UNLOAD_MESH))
            {
                continue;
            }

            if (!chunk->mesh_)
            {
                chunk->mesh_ = get_mesh_cached();
                chunk->need_rebuild_mesh_ = true;
            }

            if (chunk->need_rebuild_mesh_)
            {
                ChunkMeshGenerator generator;
                generator.rebuildMesh(*chunk, *chunk->mesh_, neighbours);
                chunk->need_rebuild_mesh_ = false;
            }
        }
    }

#ifndef NDEBUG
    for (const UPtr<Chunk> &chunk : chunks_)
    {
        if (is_outside_radius(*chunk, RADIUS_UNLOAD_WHOLE_CHUNK))
        {
            assert(0);
        }
        if (!chunk->mesh_)
        {
            continue;
        }
        if (is_outside_radius(*chunk, RADIUS_UNLOAD_MESH))
        {
            assert(0);
        }
    }
#endif
}

void VoxelEngine::render(Camera *camera, GlobalLight *light)
{
    SCOPED_PROFILER;

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
        if (!chunk->mesh_)
        {
            continue;
        }

        chunk->mesh_->bind();

        glm::vec3 glob_position{0.0f};
        glob_position.x = chunk->position_.x * Chunk::CHUNK_WIDTH;
        glob_position.y = 0.0f;
        glob_position.z = chunk->position_.z * Chunk::CHUNK_WIDTH;

        auto value = camera->getViewProj() * glm::translate(glm::mat4{1.0f}, glob_position);
        shader_->setUniformMat4(model_view_proj_loc, value);

        GL_CHECKED(glDrawArrays(GL_TRIANGLES, 0, chunk->mesh_->getNumVertices()));
        eng.stat.addRenderedIndices(chunk->mesh_->getNumVertices());
    }
}

void VoxelEngine::setSeed(unsigned int seed)
{
    seed_ = seed;
    perlin_ = makeU<Perlin>(seed_);
}

int VoxelEngine::getNumRenderChunks() const
{
    int ret = 0;
    for (const UPtr<Chunk> &chunk : chunks_)
    {
        if (chunk->mesh_)
        {
            ++ret;
        }
    }
    return ret;
}

uint64_t VoxelEngine::getNumRenderVertices() const
{
    uint64_t ret = 0;
    for (const UPtr<Chunk> &chunk : chunks_)
    {
        if (chunk->mesh_)
        {
            ret += chunk->mesh_->getNumVertices();
        }
    }
    return ret;
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

UPtr<ChunkMesh> VoxelEngine::get_mesh_cached()
{
    if (meshes_pool_.empty())
    {
        return makeU<ChunkMesh>();
    }
    UPtr<ChunkMesh> mesh = std::move(meshes_pool_.back());
    meshes_pool_.pop_back();
    assert(mesh);
    mesh->clear();
    return mesh;
}

void VoxelEngine::release_mesh(UPtr<ChunkMesh> mesh)
{
    assert(mesh);
    meshes_pool_.push_back(std::move(mesh));
}

void VoxelEngine::generate_chunk(Chunk &chunk)
{
    SCOPED_PROFILER;

    assert(perlin_);
    const siv::PerlinNoise &perlin = *perlin_;

    int height_map[Chunk::CHUNK_WIDTH][Chunk::CHUNK_WIDTH];

    constexpr float FACTOR = 0.007f;
    constexpr int MIN = 10;
    constexpr int MAX = Chunk::CHUNK_HEIGHT - 120;
    constexpr int HEIGHT_DIFF = MAX - MIN;
    static_assert(MIN < MAX);

    const int offset_x = chunk.getBlocksOffsetX();
    const int offset_z = chunk.getBlocksOffsetZ();

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
    offset.x = chunk.getBlocksOffsetX();
    offset.y = 0;
    offset.z = chunk.getBlocksOffsetZ();
    chunk.visitWrite([&](int x, int y, int z, BlockInfo &block) {
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
}

glm::ivec3 VoxelEngine::pos_to_chunk_pos(const glm::vec3 &pos) const
{
    const auto x = std::floor(pos.x / Chunk::CHUNK_WIDTH);
    const auto y = 0;
    const auto z = std::floor(pos.z / Chunk::CHUNK_WIDTH);
    return glm::ivec3{x, y, z};
}

Chunk *VoxelEngine::get_chunk_at_pos(int x, int z) const
{
    const glm::ivec2 pos = glm::ivec2{x, z};
    const auto it = chunk_index_by_pos_.find(pos);
    if (it == chunk_index_by_pos_.end())
    {
        return nullptr;
    }
    return chunks_[it->second].get();
}

bool VoxelEngine::has_all_neighbours(Chunk *chunk) const
{
    assert(chunk);
    const glm::ivec3 pos = chunk->position_;
    const int x = pos.x;
    const int z = pos.z;
    return has_chunk_at_pos(x + 1, z) && has_chunk_at_pos(x - 1, z) && has_chunk_at_pos(x, z + 1)
        && has_chunk_at_pos(x, z - 1);
}

NeighbourChunks VoxelEngine::get_neighbour_chunks(Chunk *chunk) const
{
    assert(chunk);
    const glm::ivec3 pos = chunk->position_;
    const int x = pos.x;
    const int z = pos.z;

    NeighbourChunks chunks;
    chunks.px = get_chunk_at_pos(x + 1, z);
    chunks.nx = get_chunk_at_pos(x - 1, z);
    chunks.pz = get_chunk_at_pos(x, z + 1);
    chunks.nz = get_chunk_at_pos(x, z - 1);

    return chunks;
}

NeighbourChunks VoxelEngine::get_neighbour_chunks_lazy(Chunk *chunk) const
{
    assert(chunk);
    const glm::ivec3 pos = chunk->position_;
    const int x = pos.x;
    const int z = pos.z;

    NeighbourChunks chunks;
    chunks.px = get_chunk_at_pos(x + 1, z);
    if (!chunks.px)
    {
        return chunks;
    }
    chunks.nx = get_chunk_at_pos(x - 1, z);
    if (!chunks.nx)
    {
        return chunks;
    }
    chunks.pz = get_chunk_at_pos(x, z + 1);
    if (!chunks.pz)
    {
        return chunks;
    }
    chunks.nz = get_chunk_at_pos(x, z - 1);

    return chunks;
}

void VoxelEngine::refresh_chunk_index_by_pos()
{
    SCOPED_PROFILER;

    chunk_index_by_pos_.clear();
    chunk_index_by_pos_.reserve(chunks_.size());
    for (int i = 0, count = chunks_.size(); i < count; ++i)
    {
        const UPtr<Chunk> &c = chunks_[i];
        const glm::ivec2 pos = glm::ivec2{c->position_.x, c->position_.z};
        chunk_index_by_pos_[pos] = i;
    }
}
