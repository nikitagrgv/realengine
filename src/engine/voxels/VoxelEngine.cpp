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
#include "noise/MapToMinMax.h"
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
    Texture::LoadParams params;
    params.target_format = Texture::Format::RGBA;
    params.wrap = Texture::Wrap::ClampToEdge;
    params.min_filter = Texture::Filter::NearestMipmapNearest;
    params.mag_filter = Texture::Filter::Nearest;
    params.max_mipmap_level = 4;
    atlas->load("vox/atlas.png", Texture::FlipMode::FlipY, params);

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

    constexpr int MULTIPLIER = 20;
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
        const glm::ivec3 pos = chunk.getPosition();
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
                const glm::ivec2 pos = chunk->getPositionXZ();
                const auto it = chunk_index_by_pos_.find(pos);
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

    chunks_for_render_.clear();
    for (const UPtr<Chunk> &chunk : chunks_)
    {
        if (!chunk->mesh_)
        {
            continue;
        }
        const math::BoundSphere &bound_sphere = chunk->getBoundSphere();
        const bool inside = bound_sphere.isInsideFrustum(camera->getFrustumPlanes());
        if (!inside)
        {
            continue;
        }
        chunks_for_render_.push_back(chunk.get());
    }

    // TODO# culling, sort by distance (nearest first)
    for (const Chunk *chunk : chunks_for_render_)
    {
        assert(chunk.mesh_);
        chunk->mesh_->bind();

        const glm::vec3 glob_position = chunk->getGlobalPositionFloat();

        auto value = camera->getViewProj() * glm::translate(glm::mat4{1.0f}, glob_position);
        shader_->setUniformMat4(model_view_proj_loc, value);

        GL_CHECKED(glDrawArrays(GL_TRIANGLES, 0, chunk->mesh_->getNumGpuVertices()));

        const uint64_t num_vertices = chunk->mesh_->getNumGpuVertices();
        eng.stat.addRenderedIndices(num_vertices);
        eng.stat.addRenderedChunks(1);
        eng.stat.addRenderedChunksVertices(num_vertices);
    }
}

void VoxelEngine::setSeed(unsigned int seed)
{
    seed_ = seed;
    perlin_ = makeU<Perlin>();
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
    {
        BlockDescription &b = reg.addBlock();
        BasicBlocks::SNOW = b.id;
        b.name = "Snow";
        b.type = BlockType::SOLID;
        b.texture_index_px = 4;
        b.texture_index_nx = 4;
        b.texture_index_py = 4;
        b.texture_index_ny = 4;
        b.texture_index_pz = 4;
        b.texture_index_nz = 4;
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
    chunk->setPosition(pos);
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

    const glm::ivec3 pos = chunk->getPosition();

    assert(!is_enqued_for_generation(x, z));
    UPtr<Job> job = makeU<Job>(std::move(chunk), *this);
    enqueued_chunks_.emplace_back(pos.x, pos.z, job->getCancelToken());
    eng.queue->enqueueJob(std::move(job));
}

void VoxelEngine::generate_chunk_threadsafe(Chunk &chunk) const
{
    using namespace math;

    SCOPED_PROFILER;

    constexpr float BASE_FREQ = 0.002f;

    constexpr int MIN = 40;
    constexpr int HEIGHT = 180;
    constexpr int MAX = MIN + HEIGHT;

    constexpr int SNOW_OFFSET = MAX - 20;
    constexpr int SNOW_APLITUDE = 15;

    constexpr int HEIGHT_DISPLACE_AMPLITUDE = 120;

    static_assert(MIN < MAX && MAX < Chunk::CHUNK_HEIGHT && SNOW_OFFSET < Chunk::CHUNK_HEIGHT);

    const glm::ivec2 chunk_pos_i = chunk.getBlocksOffset();
    const glm::vec2 chunk_pos = glm::vec2(chunk_pos_i);
    const glm::vec2 chunk_end = glm::vec2(chunk.getBlocksEndOffset());

    // Snow height map
    noise::module::Billow base_snow;
    base_snow.SetFrequency(BASE_FREQ * 3);
    base_snow.SetPersistence(0.7);

    noise::module::MapToMinMax snow_final_blocks;
    snow_final_blocks.SetSourceModule(0, base_snow);
    snow_final_blocks.SetMinAndHeight(SNOW_OFFSET, SNOW_APLITUDE);

    noise::utils::NoiseMap snow_map_;
    noise::utils::NoiseMapBuilderPlane snow_map_builder_;
    snow_map_builder_.SetSourceModule(snow_final_blocks);
    snow_map_builder_.SetDestNoiseMap(snow_map_);
    snow_map_builder_.SetDestSize(16, 16);
    snow_map_builder_.SetBounds(chunk_pos.x, chunk_end.x, chunk_pos.y, chunk_end.y);
    snow_map_builder_.Build();

    // Height map
    noise::module::RidgedMulti mountain;
    mountain.SetFrequency(BASE_FREQ);

    noise::module::Billow base_flat;
    base_flat.SetFrequency(BASE_FREQ * 2.2);

    noise::module::ScaleBias flat;
    flat.SetSourceModule(0, base_flat);
    flat.SetScale(0.095);
    flat.SetBias(-0.95);

    noise::module::Perlin type;
    type.SetFrequency(BASE_FREQ * 0.8);
    type.SetPersistence(0.4);

    noise::module::Select selector;
    selector.SetSourceModule(0, flat);
    selector.SetSourceModule(1, mountain);
    selector.SetControlModule(type);
    selector.SetBounds(0.4, 1000);
    selector.SetEdgeFalloff(0.1);

    noise::module::Turbulence turbulence;
    turbulence.SetSourceModule(0, selector);
    turbulence.SetFrequency(BASE_FREQ * 4);
    turbulence.SetPower(4);

    noise::module::MapToMinMax non_displacement_height_blocks;
    non_displacement_height_blocks.SetSourceModule(0, turbulence);
    non_displacement_height_blocks.SetMinAndHeight(MIN, HEIGHT);

    noise::module::Perlin height_displace_perlin;
    height_displace_perlin.SetFrequency(BASE_FREQ * 0.05);
    height_displace_perlin.SetOctaveCount(5);
    height_displace_perlin.SetPersistence(0.7);

    noise::module::MapToMinMax height_displace_blocks;
    height_displace_blocks.SetSourceModule(0, height_displace_perlin);
    height_displace_blocks.SetMinAndHeight(0, HEIGHT_DISPLACE_AMPLITUDE);

    noise::module::Add height_final_blocks;
    height_final_blocks.SetSourceModule(0, non_displacement_height_blocks);
    height_final_blocks.SetSourceModule(1, height_displace_blocks);

    noise::utils::NoiseMap height_map_;
    noise::utils::NoiseMapBuilderPlane height_map_builder_;
    height_map_builder_.SetSourceModule(height_final_blocks);
    height_map_builder_.SetDestNoiseMap(height_map_);
    height_map_builder_.SetDestSize(16, 16);
    height_map_builder_.SetBounds(chunk_pos.x, chunk_end.x, chunk_pos.y, chunk_end.y);
    height_map_builder_.Build();

    // Caves
    noise::module::RidgedMulti cave_base;
    cave_base.SetLacunarity(0.5);
    cave_base.SetFrequency(BASE_FREQ * 4);
    cave_base.SetOctaveCount(4);

    noise::module::Turbulence cave;
    cave.SetSourceModule(0, cave_base);
    cave.SetFrequency(BASE_FREQ * 3);
    cave.SetPower(20);

    int block_index = -1;
    chunk.need_rebuild_mesh_ = true;
    for (int y = 0; y < Chunk::CHUNK_HEIGHT; ++y)
    {
        const double y_glob = (double)y;
        for (int z = 0; z < Chunk::CHUNK_WIDTH; ++z)
        {
            const double z_glob = (double)z + chunk_pos.y;
            for (int x = 0; x < Chunk::CHUNK_WIDTH; ++x)
            {
                const double x_glob = (double)x + chunk_pos.x;
                ++block_index;
                BlockInfo &block = chunk.blocks_[block_index];
                const int height = (int)height_map_.GetValue(x, z);
                const int diff = y - height;

                if (diff <= 0)
                {
                    const double cave_value = cave.GetValue(x_glob, y_glob, z_glob);
                    // if (cave_value < -0.6 && cave_value > -0.8)
                    // if (cave_value > 0.1 && cave_value < 0.7)
                    if (cave_value > 20)
                    {
                        block = BlockInfo(BasicBlocks::AIR);
                        continue;
                    }
                }

                if (diff > 0)
                {
                    block = BlockInfo(BasicBlocks::AIR);
                }
                else
                {
                    const int snow_pos = (int)snow_map_.GetValue(x, z);
                    if (y > snow_pos)
                    {
                        block = BlockInfo(BasicBlocks::SNOW);
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
                }
            }
        }
    }
}

void VoxelEngine::finish_generate_chunk(UPtr<Chunk> chunk, bool generated)
{
    const auto chunk_pos = chunk->getPosition();
    const int x = chunk_pos.x;
    const int z = chunk_pos.z;
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
    const glm::ivec3 pos = chunk->getPosition();
    const int x = pos.x;
    const int z = pos.z;
    return has_chunk_at_pos(x + 1, z) && has_chunk_at_pos(x - 1, z) && has_chunk_at_pos(x, z + 1)
        && has_chunk_at_pos(x, z - 1);
}

NeighbourChunks VoxelEngine::get_neighbour_chunks(Chunk *chunk) const
{
    assert(chunk);
    const glm::ivec3 pos = chunk->getPosition();
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
    const glm::ivec3 pos = chunk->getPosition();
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
        const glm::ivec2 pos = c->getPositionXZ();
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
