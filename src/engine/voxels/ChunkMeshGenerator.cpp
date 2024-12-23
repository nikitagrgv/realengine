#include "ChunkMeshGenerator.h"

#include "BlockDescription.h"
#include "BlocksRegistry.h"
#include "Chunk.h"
#include "ChunkMesh.h"
#include "EngineGlobals.h"
#include "VoxelEngine.h"
#include "VoxelsUtils.h"
#include "profiler/ScopedProfiler.h"

bool is_air(int x, int y, int z, const Descriptions3x3 &descs)
{
    const BlockDescription *desc = descs.getBlockAtOffset(x, y, z);
    // TODO# check transparent
    return desc->type == BlockType::AIR;
}

bool is_solid(int x, int y, int z, const Descriptions3x3 &descs)
{
    const BlockDescription *desc = descs.getBlockAtOffset(x, y, z);
    // TODO# check transparent
    return desc->type != BlockType::AIR;
}

bool is_solid(glm::ivec3 pos, const Descriptions3x3 &descs)
{
    return is_solid(pos.x, pos.y, pos.z, descs);
}


void ChunkMeshGenerator::rebuildMesh(const Chunk &chunk, ChunkMesh &mesh,
    const ExtendedNeighbourChunks &neighbours)
{
    assert(chunk.need_rebuild_mesh_ || chunk.need_rebuild_mesh_force_);

    SCOPED_FUNC_PROFILER;

    mesh.clear();

    BlocksRegistry *registry = eng.vox->getRegistry();

    int block_index = -1;
    for (int y = 0; y < Chunk::CHUNK_HEIGHT; ++y)
    {
        for (int z = 0; z < Chunk::CHUNK_WIDTH; ++z)
        {
            for (int x = 0; x < Chunk::CHUNK_WIDTH; ++x)
            {
                ++block_index;
                const BlockInfo block = chunk.getBlock(block_index);

                if (block.id == 0)
                {
                    continue;
                }

                const glm::vec3 min = glm::vec3{x, y, z};
                const glm::vec3 max = min + glm::vec3(1, 1, 1);

                Descriptions3x3 descs;
                for (int dy = -1; dy <= 1; ++dy)
                {
                    for (int dz = -1; dz <= 1; ++dz)
                    {
                        for (int dx = -1; dx <= 1; ++dx)
                        {
                            const BlockInfo b = utils::getBlock(x + dx, y + dy, z + dz, chunk,
                                neighbours);
                            const BlockDescription &d = registry->getBlock(b.id);
                            descs.setBlockAtOffset(dx, dy, dz, &d);
                        }
                    }
                }

                if (is_air(1, 0, 0, descs))
                {
                    gen_face_px(min, max, descs, mesh);
                }
                if (is_air(-1, 0, 0, descs))
                {
                    gen_face_nx(min, max, descs, mesh);
                }
                if (is_air(0, 1, 0, descs))
                {
                    gen_face_py(min, max, descs, mesh);
                }
                if (y != 0 && is_air(0, -1, 0, descs)) // don't spam -y faces for 0 blocks
                {
                    gen_face_ny(min, max, descs, mesh);
                }
                if (is_air(0, 0, 1, descs))
                {
                    gen_face_pz(min, max, descs, mesh);
                }
                if (is_air(0, 0, -1, descs))
                {
                    gen_face_nz(min, max, descs, mesh);
                }
            }
        }
    }

    {
        SCOPED_PROFILER("flush vbo");
        mesh.flush();
    }
    mesh.deallocate();
}

constexpr float FAR0 = 1.0f;
constexpr float TOTAL = FAR0 * 3;
constexpr float TOTAL_INV = 1.0f / TOTAL;

void ChunkMeshGenerator::gen_face_py(const glm::vec3 &min, const glm::vec3 &max,
    const Descriptions3x3 &descs, ChunkMesh &mesh)
{
    const BlockDescription::TexCoords &coords = descs.getCenter()->cached.texture_coord_py;

    ChunkMesh::Vertex vs[6];

    constexpr glm::ivec3 ioffset{0, 1, 0};
    constexpr glm::vec3 offset{ioffset};

    const glm::vec3 minmax[2] = {min, max};
    const auto get_min_max = [&](int off) {
        // off = -1 or 1
        return minmax[(off + 1) / 2];
    };

    const auto gen_vertex = [&](int off_x, int off_y, int off_z, glm::vec2 uv) {
        ChunkMesh::Vertex v;
        v.pos = glm::vec3{get_min_max(off_x).x, get_min_max(off_y).y, get_min_max(off_z).z};
        v.norm = offset;
        v.uv = uv;
        // clang-format off
        v.ao = 1.0f
            - (
                (float)is_solid(off_x, off_y, off_z, descs) * FAR0 +
                (float)is_solid(off_x, off_y, 0, descs) * FAR0 +
                (float)is_solid(0, off_y, off_z, descs) * FAR0
                )
                * TOTAL_INV;
        // clang-format on
        return v;
    };

    // tr 1
    vs[0] = gen_vertex(-1, +1, -1, coords.top_left);
    vs[1] = gen_vertex(-1, +1, +1, coords.bottom_left);
    vs[2] = gen_vertex(+1, +1, +1, coords.bottom_right);
    // tr 2
    vs[3] = vs[0];
    vs[4] = vs[2];
    vs[5] = gen_vertex(+1, +1, -1, coords.top_right);

    mesh.addRaw(vs, sizeof(ChunkMesh::Vertex) * 6);
}

void ChunkMeshGenerator::gen_face_ny(const glm::vec3 &min, const glm::vec3 &max,
    const Descriptions3x3 &descs, ChunkMesh &mesh)
{
    const BlockDescription::TexCoords &coords = descs.getCenter()->cached.texture_coord_ny;
    ChunkMesh::Vertex vs[6];

    constexpr glm::ivec3 ioffset{0, -1, 0};
    constexpr glm::vec3 offset{ioffset};

    const glm::vec3 minmax[2] = {min, max};
    const auto get_min_max = [&](int off) {
        // off = -1 or 1
        return minmax[(off + 1) / 2];
    };

    const auto gen_vertex = [&](int off_x, int off_y, int off_z, glm::vec2 uv) {
        ChunkMesh::Vertex v;
        v.pos = glm::vec3{get_min_max(off_x).x, get_min_max(off_y).y, get_min_max(off_z).z};
        v.norm = offset;
        v.uv = uv;
        // clang-format off
        v.ao = 1.0f
            - (
                (float)is_solid(off_x, off_y, off_z, descs) * FAR0 +
                (float)is_solid(off_x, off_y, 0, descs) * FAR0 +
                (float)is_solid(0, off_y, off_z, descs) * FAR0
                )
                * TOTAL_INV;
        // clang-format on
        return v;
    };

    // tr 1
    vs[0] = gen_vertex(-1, -1, -1, coords.top_right);
    vs[1] = gen_vertex(+1, -1, +1, coords.bottom_left);
    vs[2] = gen_vertex(-1, -1, +1, coords.bottom_right);
    // tr 2
    vs[3] = vs[0];
    vs[4] = gen_vertex(+1, -1, -1, coords.top_left);
    vs[5] = vs[1];

    mesh.addRaw(vs, sizeof(ChunkMesh::Vertex) * 6);
}

void ChunkMeshGenerator::gen_face_pz(const glm::vec3 &min, const glm::vec3 &max,
    const Descriptions3x3 &descs, ChunkMesh &mesh)
{
    const BlockDescription::TexCoords &coords = descs.getCenter()->cached.texture_coord_pz;
    ChunkMesh::Vertex vs[6];

    constexpr glm::ivec3 ioffset{0, 0, 1};
    constexpr glm::vec3 offset{ioffset};

    const glm::vec3 minmax[2] = {min, max};
    const auto get_min_max = [&](int off) {
        // off = -1 or 1
        return minmax[(off + 1) / 2];
    };

    const auto gen_vertex = [&](int off_x, int off_y, int off_z, glm::vec2 uv) {
        ChunkMesh::Vertex v;
        v.pos = glm::vec3{get_min_max(off_x).x, get_min_max(off_y).y, get_min_max(off_z).z};
        v.norm = offset;
        v.uv = uv;
        // clang-format off
        v.ao = 1.0f
            - (
                (float)is_solid(off_x, off_y, off_z, descs) * FAR0 +
                (float)is_solid(off_x, 0, off_z, descs) * FAR0 +
                (float)is_solid(0, off_y, off_z, descs) * FAR0
                )
                * TOTAL_INV;
        // clang-format on
        return v;
    };

    // tr 1
    vs[0] = gen_vertex(-1, +1, +1, coords.top_left);
    vs[1] = gen_vertex(-1, -1, +1, coords.bottom_left);
    vs[2] = gen_vertex(+1, -1, +1, coords.bottom_right);
    // tr 2
    vs[3] = vs[0];
    vs[4] = vs[2];
    vs[5] = gen_vertex(+1, +1, +1, coords.top_right);

    mesh.addRaw(vs, sizeof(ChunkMesh::Vertex) * 6);
}

void ChunkMeshGenerator::gen_face_nz(const glm::vec3 &min, const glm::vec3 &max,
    const Descriptions3x3 &descs, ChunkMesh &mesh)
{
    const BlockDescription::TexCoords &coords = descs.getCenter()->cached.texture_coord_nz;
    ChunkMesh::Vertex vs[6];

    constexpr glm::ivec3 ioffset{0, 0, -1};
    constexpr glm::vec3 offset{ioffset};

    const glm::vec3 minmax[2] = {min, max};
    const auto get_min_max = [&](int off) {
        // off = -1 or 1
        return minmax[(off + 1) / 2];
    };

    const auto gen_vertex = [&](int off_x, int off_y, int off_z, glm::vec2 uv) {
        ChunkMesh::Vertex v;
        v.pos = glm::vec3{get_min_max(off_x).x, get_min_max(off_y).y, get_min_max(off_z).z};
        v.norm = offset;
        v.uv = uv;
        // clang-format off
        v.ao = 1.0f
            - (
                (float)is_solid(off_x, off_y, off_z, descs) * FAR0 +
                (float)is_solid(off_x, 0, off_z, descs) * FAR0 +
                (float)is_solid(0, off_y, off_z, descs) * FAR0
                )
                * TOTAL_INV;
        // clang-format on
        return v;
    };

    // tr 1
    vs[0] = gen_vertex(-1, +1, -1, coords.top_right);
    vs[1] = gen_vertex(+1, -1, -1, coords.bottom_left);
    vs[2] = gen_vertex(-1, -1, -1, coords.bottom_right);
    // tr 2
    vs[3] = vs[0];
    vs[4] = gen_vertex(+1, +1, -1, coords.top_left);
    vs[5] = vs[1];

    mesh.addRaw(vs, sizeof(ChunkMesh::Vertex) * 6);
}

void ChunkMeshGenerator::gen_face_px(const glm::vec3 &min, const glm::vec3 &max,
    const Descriptions3x3 &descs, ChunkMesh &mesh)
{
    const BlockDescription::TexCoords &coords = descs.getCenter()->cached.texture_coord_px;
    ChunkMesh::Vertex vs[6];

    constexpr glm::ivec3 ioffset{1, 0, 0};
    constexpr glm::vec3 offset{ioffset};

    const glm::vec3 minmax[2] = {min, max};
    const auto get_min_max = [&](int off) {
        // off = -1 or 1
        return minmax[(off + 1) / 2];
    };

    const auto gen_vertex = [&](int off_x, int off_y, int off_z, glm::vec2 uv) {
        ChunkMesh::Vertex v;
        v.pos = glm::vec3{get_min_max(off_x).x, get_min_max(off_y).y, get_min_max(off_z).z};
        v.norm = offset;
        v.uv = uv;
        // clang-format off
        v.ao = 1.0f
            - (
                (float)is_solid(off_x, off_y, off_z, descs) * FAR0 +
                (float)is_solid(off_x, 0, off_z, descs) * FAR0 +
                (float)is_solid(off_x, off_y, 0, descs) * FAR0
                )
                * TOTAL_INV;
        // clang-format on
        return v;
    };

    // tr 1
    vs[0] = gen_vertex(+1, +1, +1, coords.top_left);
    vs[1] = gen_vertex(+1, -1, +1, coords.bottom_left);
    vs[2] = gen_vertex(+1, -1, -1, coords.bottom_right);
    // tr 2
    vs[3] = vs[0];
    vs[4] = vs[2];
    vs[5] = gen_vertex(+1, +1, -1, coords.top_right);

    mesh.addRaw(vs, sizeof(ChunkMesh::Vertex) * 6);
}

void ChunkMeshGenerator::gen_face_nx(const glm::vec3 &min, const glm::vec3 &max,
    const Descriptions3x3 &descs, ChunkMesh &mesh)
{
    const BlockDescription::TexCoords &coords = descs.getCenter()->cached.texture_coord_nx;
    ChunkMesh::Vertex vs[6];

    constexpr glm::ivec3 ioffset{-1, 0, 0};
    constexpr glm::vec3 offset{ioffset};

    const glm::vec3 minmax[2] = {min, max};
    const auto get_min_max = [&](int off) {
        // off = -1 or 1
        return minmax[(off + 1) / 2];
    };

    const auto gen_vertex = [&](int off_x, int off_y, int off_z, glm::vec2 uv) {
        ChunkMesh::Vertex v;
        v.pos = glm::vec3{get_min_max(off_x).x, get_min_max(off_y).y, get_min_max(off_z).z};
        v.norm = offset;
        v.uv = uv;
        // clang-format off
        v.ao = 1.0f
            - (
                (float)is_solid(off_x, off_y, off_z, descs) * FAR0 +
                (float)is_solid(off_x, 0, off_z, descs) * FAR0 +
                (float)is_solid(off_x, off_y, 0, descs) * FAR0
                )
                * TOTAL_INV;
        // clang-format on
        return v;
    };

    // tr 1
    vs[0] = gen_vertex(-1, +1, +1, coords.top_right);
    vs[1] = gen_vertex(-1, -1, -1, coords.bottom_left);
    vs[2] = gen_vertex(-1, -1, +1, coords.bottom_right);
    // tr 2
    vs[3] = vs[0];
    vs[4] = gen_vertex(-1, +1, -1, coords.top_left);
    vs[5] = vs[1];

    mesh.addRaw(vs, sizeof(ChunkMesh::Vertex) * 6);
}
