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
#include "threads/Job.h"
#include "threads/JobQueue.h"
#include "threads/Threads.h"
#include "utils/Algos.h"

#include <noise/noise.h>
#include <noise/noiseutils.h>

#include <glm/ext/matrix_transform.hpp>

struct VoxelEngine::Perlin
{
    explicit Perlin() {}

    noise::module::Perlin perlin_;
};

VoxelEngine::VoxelEngine() = default;

VoxelEngine::~VoxelEngine() = default;

void VoxelEngine::init()
{
    perlin_ = makeU<Perlin>();

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

#ifndef NDEBUG
    constexpr int MAX_INIT_CHUNKS_PER_UPDATE = 30;
    constexpr int MAX_REGENERATED_MESHES_PER_UPDATE = 2;
#else
    constexpr int MAX_INIT_CHUNKS_PER_UPDATE = 100;
    constexpr int MAX_REGENERATED_MESHES_PER_UPDATE = 10;
#endif

    constexpr int MULTIPLIER = 16;
    constexpr int RADIUS_SPAWN_CHUNK = 2 * MULTIPLIER;
    constexpr int RADIUS_UNLOAD_MESH = 3 * MULTIPLIER;
    constexpr int RADIUS_UNLOAD_WHOLE_CHUNK = 4 * MULTIPLIER;

    static_assert(RADIUS_UNLOAD_WHOLE_CHUNK > RADIUS_UNLOAD_MESH
            && RADIUS_UNLOAD_MESH > RADIUS_SPAWN_CHUNK,
        "Invalid radiuses");

    const auto get_distance2 = [&](int x, int z) {
        const int dz = base_chunk_pos.z - z;
        const int dx = base_chunk_pos.x - x;
        return dz * dz + dx * dx;
    };

    const auto get_chunk_distance2 = [&](const Chunk &chunk) {
        const glm::ivec3 pos = chunk.position_;
        return get_distance2(pos.x, pos.z);
    };

    const auto is_outside_radius = [&](int x, int z, int radius) {
        return get_distance2(x, z) > radius * radius;
    };

    const auto is_chunk_outside_radius = [&](const Chunk &chunk, int radius) {
        return get_chunk_distance2(chunk) > radius * radius;
    };

    bool chunks_dirty = false;

    {
        ScopedProfiler p("Cancel chunks jobs");
        const bool chunk_changed = last_base_chunk_pos_ != base_chunk_pos;
        for (EnqueuedChunk &c : enqueued_chunks_)
        {
            assert(c.cancel_token.isAlive());
            if (chunk_changed || is_outside_radius(c.pos.x, c.pos.y, RADIUS_UNLOAD_WHOLE_CHUNK))
            {
                c.cancel_token.cancel();
            }
        }
    }

    {
        ScopedProfiler p("Unload chunks");

        // Unload whole chunks outside radius

        // TODO# save in file or compress

        for (UPtr<Chunk> &chunk : chunks_)
        {
            const bool outside = is_chunk_outside_radius(*chunk, RADIUS_UNLOAD_WHOLE_CHUNK);
            if (outside)
            {
                const auto it = chunk_index_by_pos_.find(
                    glm::ivec2(chunk->position_.x, chunk->position_.z));
                assert(it != chunk_index_by_pos_.end());
                chunk_index_by_pos_.erase(it);
                release_chunk(std::move(chunk));
            }
        }

        const int old_size = chunks_.size();
        Alg::removeIf(chunks_, [&](const UPtr<Chunk> &chunk) { return chunk == nullptr; });
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
            if (is_chunk_outside_radius(*chunk, RADIUS_UNLOAD_MESH))
            {
                release_mesh(std::move(chunk->mesh_));
            }
        }
    }

    {
        ScopedProfiler p("Init new chunks");

        int num_inited_chunks = 0;

        const std::vector<glm::ivec2> offsets = offsets_cache.getOffsets(RADIUS_SPAWN_CHUNK);
        for (const glm::ivec2 &offset : offsets)
        {
            const int x = base_chunk_pos.x + offset.x;
            const int z = base_chunk_pos.z + offset.y;

            assert(!is_outside_radius(x, z, RADIUS_SPAWN_CHUNK));

            // NOTE: chunk_index_by_pos_ has invalid values here! but we can use it in this case
            if (has_chunk_at_pos(x, z))
            {
                continue;
            }
            if (is_enqued_for_generation(x, z))
            {
                continue;
            }
            // TODO# shitty
            if (is_generated(x, z))
            {
                continue;
            }

            if (num_inited_chunks > MAX_INIT_CHUNKS_PER_UPDATE)
            {
                continue;
            }

            UPtr<Chunk> new_chunk = get_chunk_cached(glm::ivec3{x, 0, z});
            chunks_to_generate_.push_back(std::move(new_chunk));
            ++num_inited_chunks;
        }
    }

    {
        ScopedProfiler p("Release canceled chunks");
        for (UPtr<Chunk> &chunk : canceled_chunks_)
        {
            assert(chunk);
            release_chunk(std::move(chunk));
        }
        canceled_chunks_.clear();
    }

    // TODO# move upper?
    {
        // TODO# CHECK OUTSIDE RADIUS

        ScopedProfiler p("Add generated chunks");
        for (UPtr<Chunk> &chunk : generated_chunks_)
        {
            assert(chunk);

            if (is_chunk_outside_radius(*chunk, RADIUS_UNLOAD_WHOLE_CHUNK))
            {
                release_chunk(std::move(chunk));
                continue;
            }

            assert(Alg::noneOf(chunks_,
                [&](const UPtr<Chunk> &c) { return c->position_ == chunk->position_; }));
            chunks_.push_back(std::move(chunk));
            chunks_dirty = true;
        }
        generated_chunks_.clear();
    }

    if (chunks_dirty)
    {
        refresh_chunk_index_by_pos();
    }

    {
        ScopedProfiler p("Enqueue new chunks for generation");

        {
            ScopedProfiler p2("Sort by distance");
            Alg::sort(chunks_to_generate_, [&](const UPtr<Chunk> &lhs, const UPtr<Chunk> &rhs) {
                return get_chunk_distance2(*lhs) < get_chunk_distance2(*rhs);
            });
        }

        for (UPtr<Chunk> &chunk : chunks_to_generate_)
        {
            queue_generate_chunk(std::move(chunk));
        }
        chunks_to_generate_.clear();
    }

    {
        ScopedProfiler p("Generate/unload meshes");

        chunks_for_regenerate_.clear();

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
            if (is_chunk_outside_radius(*chunk, RADIUS_UNLOAD_MESH))
            {
                continue;
            }

            if (!chunk->mesh_ || chunk->need_rebuild_mesh_ || chunk->need_rebuild_mesh_force_)
            {
                chunks_for_regenerate_.push_back(chunk.get());
            }
        }

        {
            ScopedProfiler p1("Sort by distance");
            Alg::sort(chunks_for_regenerate_, [&](const Chunk *lhs, const Chunk *rhs) {
                return get_chunk_distance2(*lhs) < get_chunk_distance2(*rhs);
            });
        }

        {
            ScopedProfiler p1("Generate meshes");

            int num_regenerated_meshes = 0;
            for (Chunk *chunk : chunks_for_regenerate_)
            {
                if (!chunk->need_rebuild_mesh_force_
                    && num_regenerated_meshes >= MAX_REGENERATED_MESHES_PER_UPDATE)
                {
                    continue;
                }

                if (!chunk->mesh_)
                {
                    chunk->mesh_ = get_mesh_cached();
                    chunk->need_rebuild_mesh_ = true;
                }

                if (chunk->need_rebuild_mesh_ || chunk->need_rebuild_mesh_force_)
                {
                    NeighbourChunks neighbours = get_neighbour_chunks_lazy(chunk);
                    assert(neighbours.hasAll());

                    ChunkMeshGenerator generator;
                    generator.rebuildMesh(*chunk, *chunk->mesh_, neighbours);
                    chunk->need_rebuild_mesh_ = false;
                    chunk->need_rebuild_mesh_force_ = false;

                    num_regenerated_meshes++;
                }
            }
        }
    }

#ifndef NDEBUG
    for (const UPtr<Chunk> &chunk : chunks_)
    {
        if (is_chunk_outside_radius(*chunk, RADIUS_UNLOAD_WHOLE_CHUNK))
        {
            assert(0);
        }
        if (!chunk->mesh_)
        {
            continue;
        }
        if (is_chunk_outside_radius(*chunk, RADIUS_UNLOAD_MESH))
        {
            assert(0);
        }
    }
#endif

    last_base_chunk_pos_ = base_chunk_pos;
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
    perlin_ = makeU<Perlin>();
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

bool VoxelEngine::setBlockAtPosition(const glm::ivec3 &position, BlockInfo block)
{
    const glm::ivec3 chunk_pos = pos_to_chunk_pos(position);

    Chunk *chunk = get_chunk_at_pos(chunk_pos.x, chunk_pos.z);

    if (!chunk)
    {
        return false;
    }

    const glm::ivec3 loc_pos = chunk->getBlockLocalPosition(position);
    assert(loc_pos.x >= 0 && loc_pos.x < Chunk::CHUNK_WIDTH);
    assert(loc_pos.z >= 0 && loc_pos.z < Chunk::CHUNK_WIDTH);

    if (loc_pos.y < 0 || loc_pos.y >= Chunk::CHUNK_HEIGHT)
    {
        return false;
    }

    BlockInfo &b = chunk->getBlockRef(loc_pos.x, loc_pos.y, loc_pos.z);
    if (b == block)
    {
        return true;
    }

    b = block;
    chunk->need_rebuild_mesh_force_ = true;

    if (loc_pos.x == 0)
    {
        Chunk *c = get_chunk_at_pos(chunk_pos.x - 1, chunk_pos.z);
        if (c)
        {
            c->need_rebuild_mesh_force_ = true;
        }
    }
    if (loc_pos.x == Chunk::CHUNK_WIDTH - 1)
    {
        Chunk *c = get_chunk_at_pos(chunk_pos.x + 1, chunk_pos.z);
        if (c)
        {
            c->need_rebuild_mesh_force_ = true;
        }
    }
    if (loc_pos.z == 0)
    {
        Chunk *c = get_chunk_at_pos(chunk_pos.x, chunk_pos.z - 1);
        if (c)
        {
            c->need_rebuild_mesh_force_ = true;
        }
    }
    if (loc_pos.z == Chunk::CHUNK_WIDTH - 1)
    {
        Chunk *c = get_chunk_at_pos(chunk_pos.x, chunk_pos.z + 1);
        if (c)
        {
            c->need_rebuild_mesh_force_ = true;
        }
    }

    return true;
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

UPtr<Chunk> VoxelEngine::get_chunk_cached(const glm::ivec3 &pos)
{
    if (chunks_pool_.empty())
    {
        return makeU<Chunk>(pos);
    }
    UPtr<Chunk> chunk = std::move(chunks_pool_.back());
    chunks_pool_.pop_back();
    assert(chunk);
    chunk->clear();
    chunk->position_ = pos;
    return chunk;
}

void VoxelEngine::release_chunk(UPtr<Chunk> chunk)
{
    assert(chunk);
    chunks_pool_.push_back(std::move(chunk));
}


void VoxelEngine::queue_generate_chunk(UPtr<Chunk> chunk)
{
    SCOPED_PROFILER;

    struct Job : tbb::Job
    {
    public:
        explicit Job(UPtr<Chunk> chunk, VoxelEngine &v)
            : v_(v)
            , chunk_(std::move(chunk))
        {
            assert(chunk_);
        }

        void execute() override
        {
            if (!isCanceled())
            {
                v_.generate_chunk_threadsafe(*chunk_);
                generated_ = true;
            }
        }
        void finishMainThread() override
        {
            v_.finish_generate_chunk(std::move(chunk_), generated_);
        }

    private:
        bool generated_ = false;
        VoxelEngine &v_;
        UPtr<Chunk> chunk_;
    };

    const int x = chunk->position_.x;
    const int z = chunk->position_.z;

    assert(!is_enqued_for_generation(x, z));
    UPtr<Job> job = makeU<Job>(std::move(chunk), *this);
    enqueued_chunks_.emplace_back(x, z, job->getCancelToken());
    eng.queue->enqueueJob(std::move(job));
}

void VoxelEngine::generate_chunk_threadsafe(Chunk &chunk) const
{
    using namespace math;

    SCOPED_PROFILER;

    constexpr float FREQ = 0.002f;

    assert(perlin_);
    noise::module::Perlin &perlin = perlin_->perlin_;
    perlin.SetOctaveCount(6);
    perlin.SetFrequency(FREQ);
    perlin.SetPersistence(0.52);

    noise::module::RidgedMulti mountain;
    mountain.SetFrequency(FREQ);

    noise::module::Billow base_flat;
    base_flat.SetFrequency(FREQ);

    const glm::vec2 chunk_pos = glm::vec2(chunk.getBlocksOffset());
    const glm::vec2 chunk_end = glm::vec2(chunk.getBlocksEndOffset());

    noise::utils::NoiseMap height_map_;
    noise::utils::NoiseMapBuilderPlane height_map_builder_;
    height_map_builder_.SetSourceModule(mountain);
    height_map_builder_.SetDestNoiseMap(height_map_);
    height_map_builder_.SetDestSize(16, 16);
    height_map_builder_.SetBounds(chunk_pos.x, chunk_end.x, chunk_pos.y, chunk_end.y);
    height_map_builder_.Build();

    // TODO!# ADD FLATTNESS
    constexpr int MIN = 40;
    constexpr int HEIGHT_DIFF = 180;
    constexpr int MAX = MIN + HEIGHT_DIFF;
    static_assert(MIN < MAX && MAX < Chunk::CHUNK_HEIGHT);

    chunk.visitWrite([&](int x, int y, int z, BlockInfo &block) {
        const float height_norm = math::mapTo01(height_map_.GetValue(x, z));
        const int height = (int)(height_norm * (float)HEIGHT_DIFF + (float)MIN);

        const int diff = y - height;

        // if (diff <= 0)
        // {
        //     glm::vec3 norm_pos = (offset + glm::vec3{x, y, z}) * 0.02f;
        //     const auto noise = mapTo01(perlin.GetValue(norm_pos.x, norm_pos.y, norm_pos.z));
        //     if (noise > 0.8f)
        //     {
        //         block = BlockInfo(BasicBlocks::AIR);
        //         return;
        //     }
        // }

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

void VoxelEngine::finish_generate_chunk(UPtr<Chunk> chunk, bool generated)
{
    const int x = chunk->position_.x;
    const int z = chunk->position_.z;
    assert(is_enqued_for_generation(x, z));
    glm::ivec2 pos{x, z};
    Alg::removeOneIf(enqueued_chunks_, [&](const EnqueuedChunk &c) { return c.pos == pos; });
    assert(!is_enqued_for_generation(x, z));
    if (generated)
    {
        generated_chunks_.push_back(std::move(chunk));
    }
    else
    {
        canceled_chunks_.push_back(std::move(chunk));
    }
}

Chunk *VoxelEngine::get_chunk_at_pos(int x, int z) const
{
    const glm::ivec2 pos = glm::ivec2{x, z};
    const auto it = chunk_index_by_pos_.find(pos);
    if (it == chunk_index_by_pos_.end())
    {
        return nullptr;
    }
    const int index = it->second;
    Chunk *chunk = chunks_[index].get();
    assert(chunk->position_.x == x && chunk->position_.z == z);
    return chunk;
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
        assert(chunk_index_by_pos_.find(pos) == chunk_index_by_pos_.end());
        chunk_index_by_pos_[pos] = i;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

const std::vector<glm::ivec2> &VoxelEngine::OffsetsCache::getOffsets(int radius)
{
    if (radius == this->radius && !values.empty())
    {
        return values;
    }
    values.clear();
    const int radius2 = radius * radius;
    for (int z = -radius, z_end = radius; z <= z_end; ++z)
    {
        for (int x = -radius, x_end = radius; x <= x_end; ++x)
        {
            if (x * x + z * z <= radius2)
            {
                values.emplace_back(x, z);
            }
        }
    }
    Alg::sort(values, [](glm::ivec2 lhs, glm::ivec2 rhs) {
        return lhs.x * lhs.x + lhs.y * lhs.y < rhs.x * rhs.x + rhs.y * rhs.y;
    });
    return values;
}
