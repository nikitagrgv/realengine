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
    assert(chunk.need_rebuild_mesh_);

    SCOPED_PROFILER;

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
                if (is_air(0, -1, 0, descs))
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
        ScopedProfiler p("flush vbo");
        mesh.flush();
    }
    mesh.deallocate();
}

constexpr float FAR0 = 1.0f;
constexpr float FAR1 = 0.5f;
constexpr float FAR2 = 0.3f;
constexpr float FAR3 = 0.2f;
constexpr float TOTAL = FAR0 * 3 + FAR1 * 2 + FAR2 * 2 + FAR3;
constexpr float TOTAL_INV = 1.0f / TOTAL;

void ChunkMeshGenerator::gen_face_py(const glm::vec3 &min, const glm::vec3 &max,
    const Descriptions3x3 &descs, ChunkMesh &mesh)
{
    const BlockDescription::TexCoords &coords = descs.getCenter()->cached.texture_coord_py;

    ChunkMesh::Vertex vs[6];

    constexpr glm::ivec3 ioffset{0, 1, 0};
    constexpr glm::vec3 offset{ioffset};

    // tr 1
    vs[0].pos = glm::vec3{min.x, max.y, min.z};
    vs[0].norm = offset;
    vs[0].uv = coords.top_left;
    // clang-format off
    vs[0].ao = 1.0f
        - (
            (float)is_solid(-1, 1, -1, descs) * FAR0 +
            (float)is_solid(-1, 1, +0, descs) * FAR0 +
            (float)is_solid(+0, 1, -1, descs) * FAR0 +
            (float)is_solid(+1, 1, -1, descs) * FAR1 +
            (float)is_solid(-1, 1, +1, descs) * FAR1 +
            (float)is_solid(+1, 1, +0, descs) * FAR2 +
            (float)is_solid(+0, 1, +1, descs) * FAR2 +
            (float)is_solid(+1, 1, +1, descs) * FAR3
            )
            * TOTAL_INV;
    // clang-format on

    vs[1].pos = glm::vec3{min.x, max.y, max.z};
    vs[1].norm = offset;
    vs[1].uv = coords.bottom_left;
    vs[1].ao = 1.0f;

    vs[2].pos = glm::vec3{max.x, max.y, max.z};
    vs[2].norm = offset;
    vs[2].uv = coords.bottom_right;
    vs[2].ao = 1.0f;

    // tr 2
    vs[3] = vs[0];

    vs[4].pos = glm::vec3{max.x, max.y, max.z};
    vs[4].norm = offset;
    vs[4].uv = coords.bottom_right;
    vs[4].ao = 1.0f;

    vs[5].pos = glm::vec3{max.x, max.y, min.z};
    vs[5].norm = offset;
    vs[5].uv = coords.top_right;
    vs[5].ao = 1.0f;

    mesh.addRaw(vs, sizeof(ChunkMesh::Vertex) * 6);
}

void ChunkMeshGenerator::gen_face_ny(const glm::vec3 &min, const glm::vec3 &max,
    const Descriptions3x3 &descs, ChunkMesh &mesh)
{
    const BlockDescription::TexCoords &coords = descs.getCenter()->cached.texture_coord_ny;
    ChunkMesh::Vertex vs[6];

    // tr 1
    vs[0].pos = glm::vec3{min.x, min.y, min.z};
    vs[0].norm = glm::vec3{0, -1, 0};
    vs[0].uv = coords.top_right;
    vs[0].ao = 1.0f;

    vs[1].pos = glm::vec3{max.x, min.y, max.z};
    vs[1].norm = glm::vec3{0, -1, 0};
    vs[1].uv = coords.bottom_left;
    vs[1].ao = 1.0f;

    vs[2].pos = glm::vec3{min.x, min.y, max.z};
    vs[2].norm = glm::vec3{0, -1, 0};
    vs[2].uv = coords.bottom_right;
    vs[2].ao = 1.0f;

    // tr 2
    vs[3].pos = glm::vec3{min.x, min.y, min.z};
    vs[3].norm = glm::vec3{0, -1, 0};
    vs[3].uv = coords.top_right;
    vs[3].ao = 1.0f;

    vs[4].pos = glm::vec3{max.x, min.y, min.z};
    vs[4].norm = glm::vec3{0, -1, 0};
    vs[4].uv = coords.top_left;
    vs[4].ao = 1.0f;

    vs[5].pos = glm::vec3{max.x, min.y, max.z};
    vs[5].norm = glm::vec3{0, -1, 0};
    vs[5].uv = coords.bottom_left;
    vs[5].ao = 1.0f;

    mesh.addRaw(vs, sizeof(ChunkMesh::Vertex) * 6);
}

void ChunkMeshGenerator::gen_face_pz(const glm::vec3 &min, const glm::vec3 &max,
    const Descriptions3x3 &descs, ChunkMesh &mesh)
{
    const BlockDescription::TexCoords &coords = descs.getCenter()->cached.texture_coord_pz;
    ChunkMesh::Vertex vs[6];

    // tr 1
    vs[0].pos = glm::vec3{min.x, max.y, max.z};
    vs[0].norm = glm::vec3{0, 0, 1};
    vs[0].uv = coords.top_left;
    vs[0].ao = 1.0f;

    vs[1].pos = glm::vec3{min.x, min.y, max.z};
    vs[1].norm = glm::vec3{0, 0, 1};
    vs[1].uv = coords.bottom_left;
    vs[1].ao = 1.0f;

    vs[2].pos = glm::vec3{max.x, min.y, max.z};
    vs[2].norm = glm::vec3{0, 0, 1};
    vs[2].uv = coords.bottom_right;
    vs[2].ao = 1.0f;

    // tr 2
    vs[3].pos = glm::vec3{min.x, max.y, max.z};
    vs[3].norm = glm::vec3{0, 0, 1};
    vs[3].uv = coords.top_left;
    vs[3].ao = 1.0f;

    vs[4].pos = glm::vec3{max.x, min.y, max.z};
    vs[4].norm = glm::vec3{0, 0, 1};
    vs[4].uv = coords.bottom_right;
    vs[4].ao = 1.0f;

    vs[5].pos = glm::vec3{max.x, max.y, max.z};
    vs[5].norm = glm::vec3{0, 0, 1};
    vs[5].uv = coords.top_right;
    vs[5].ao = 1.0f;

    mesh.addRaw(vs, sizeof(ChunkMesh::Vertex) * 6);
}

void ChunkMeshGenerator::gen_face_nz(const glm::vec3 &min, const glm::vec3 &max,
    const Descriptions3x3 &descs, ChunkMesh &mesh)
{
    const BlockDescription::TexCoords &coords = descs.getCenter()->cached.texture_coord_nz;
    ChunkMesh::Vertex vs[6];

    // tr 1
    vs[0].pos = glm::vec3{min.x, max.y, min.z};
    vs[0].norm = glm::vec3{0, 0, -1};
    vs[0].uv = coords.top_right;
    vs[0].ao = 1.0f;

    vs[1].pos = glm::vec3{max.x, min.y, min.z};
    vs[1].norm = glm::vec3{0, 0, -1};
    vs[1].uv = coords.bottom_left;
    vs[1].ao = 1.0f;

    vs[2].pos = glm::vec3{min.x, min.y, min.z};
    vs[2].norm = glm::vec3{0, 0, -1};
    vs[2].uv = coords.bottom_right;
    vs[2].ao = 1.0f;

    // tr 2
    vs[3].pos = glm::vec3{min.x, max.y, min.z};
    vs[3].norm = glm::vec3{0, 0, -1};
    vs[3].uv = coords.top_right;
    vs[3].ao = 1.0f;

    vs[4].pos = glm::vec3{max.x, max.y, min.z};
    vs[4].norm = glm::vec3{0, 0, -1};
    vs[4].uv = coords.top_left;
    vs[4].ao = 1.0f;

    vs[5].pos = glm::vec3{max.x, min.y, min.z};
    vs[5].norm = glm::vec3{0, 0, -1};
    vs[5].uv = coords.bottom_left;
    vs[5].ao = 1.0f;

    mesh.addRaw(vs, sizeof(ChunkMesh::Vertex) * 6);
}

void ChunkMeshGenerator::gen_face_px(const glm::vec3 &min, const glm::vec3 &max,
    const Descriptions3x3 &descs, ChunkMesh &mesh)
{
    const BlockDescription::TexCoords &coords = descs.getCenter()->cached.texture_coord_px;
    ChunkMesh::Vertex vs[6];

    // tr 1
    vs[0].pos = glm::vec3{max.x, max.y, max.z};
    vs[0].norm = glm::vec3{1, 0, 0};
    vs[0].uv = coords.top_left;
    vs[0].ao = 1.0f;

    vs[1].pos = glm::vec3{max.x, min.y, max.z};
    vs[1].norm = glm::vec3{1, 0, 0};
    vs[1].uv = coords.bottom_left;
    vs[1].ao = 1.0f;

    vs[2].pos = glm::vec3{max.x, min.y, min.z};
    vs[2].norm = glm::vec3{1, 0, 0};
    vs[2].uv = coords.bottom_right;
    vs[2].ao = 1.0f;

    // tr 2
    vs[3].pos = glm::vec3{max.x, max.y, max.z};
    vs[3].norm = glm::vec3{1, 0, 0};
    vs[3].uv = coords.top_left;
    vs[3].ao = 1.0f;

    vs[4].pos = glm::vec3{max.x, min.y, min.z};
    vs[4].norm = glm::vec3{1, 0, 0};
    vs[4].uv = coords.bottom_right;
    vs[4].ao = 1.0f;

    vs[5].pos = glm::vec3{max.x, max.y, min.z};
    vs[5].norm = glm::vec3{1, 0, 0};
    vs[5].uv = coords.top_right;
    vs[5].ao = 1.0f;

    mesh.addRaw(vs, sizeof(ChunkMesh::Vertex) * 6);
}

void ChunkMeshGenerator::gen_face_nx(const glm::vec3 &min, const glm::vec3 &max,
    const Descriptions3x3 &descs, ChunkMesh &mesh)
{
    const BlockDescription::TexCoords &coords = descs.getCenter()->cached.texture_coord_nx;
    ChunkMesh::Vertex vs[6];

    // tr 1
    vs[0].pos = glm::vec3{min.x, max.y, max.z};
    vs[0].norm = glm::vec3{-1, 0, 0};
    vs[0].uv = coords.top_right;
    vs[0].ao = 1.0f;

    vs[1].pos = glm::vec3{min.x, min.y, min.z};
    vs[1].norm = glm::vec3{-1, 0, 0};
    vs[1].uv = coords.bottom_left;
    vs[1].ao = 1.0f;

    vs[2].pos = glm::vec3{min.x, min.y, max.z};
    vs[2].norm = glm::vec3{-1, 0, 0};
    vs[2].uv = coords.bottom_right;
    vs[2].ao = 1.0f;

    // tr 2
    vs[3].pos = glm::vec3{min.x, max.y, max.z};
    vs[3].norm = glm::vec3{-1, 0, 0};
    vs[3].uv = coords.top_right;
    vs[3].ao = 1.0f;

    vs[4].pos = glm::vec3{min.x, max.y, min.z};
    vs[4].norm = glm::vec3{-1, 0, 0};
    vs[4].uv = coords.top_left;
    vs[4].ao = 1.0f;

    vs[5].pos = glm::vec3{min.x, min.y, min.z};
    vs[5].norm = glm::vec3{-1, 0, 0};
    vs[5].uv = coords.bottom_left;
    vs[5].ao = 1.0f;

    mesh.addRaw(vs, sizeof(ChunkMesh::Vertex) * 6);
}
