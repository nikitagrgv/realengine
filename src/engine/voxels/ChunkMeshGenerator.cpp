#include "ChunkMeshGenerator.h"

#include "BlockDescription.h"
#include "BlocksRegistry.h"
#include "Chunk.h"
#include "ChunkMesh.h"
#include "EngineGlobals.h"
#include "VoxelEngine.h"
#include "profiler/ScopedProfiler.h"
#include "profiler/ScopedTimer.h"

void ChunkMeshGenerator::rebuildMesh(const Chunk &chunk, ChunkMesh &mesh,
    const NeighbourChunks &neighbours)
{
    assert(chunk.need_rebuild_mesh_);

    SCOPED_PROFILER;

    ScopedTimer t("");

    mesh.clear();

    BlocksRegistry *registry = eng.vox->getRegistry();

    const auto is_air = [&](int x, int y, int z) {
        if (y < 0)
        {
            return false;
        }
        if (y >= Chunk::CHUNK_HEIGHT)
        {
            return true;
        }

        // TODO# check transparent

        if (x < 0)
        {
            const BlockInfo block = neighbours.nx->getBlock(Chunk::CHUNK_WIDTH - 1, y, z);
            return block.id == 0;
        }
        if (x >= Chunk::CHUNK_WIDTH)
        {
            const BlockInfo block = neighbours.px->getBlock(0, y, z);
            return block.id == 0;
        }

        if (z < 0)
        {
            const BlockInfo block = neighbours.nz->getBlock(x, y, Chunk::CHUNK_WIDTH - 1);
            return block.id == 0;
        }
        if (z >= Chunk::CHUNK_WIDTH)
        {
            const BlockInfo block = neighbours.pz->getBlock(x, y, 0);
            return block.id == 0;
        }

        const BlockInfo block = chunk.getBlock(x, y, z);
        return block.id == 0;
    };

    if (false)
    {
        bool disp = true;

        for (int y = 0; y < Chunk::CHUNK_HEIGHT; ++y)
        {
            for (int z = 0; z < Chunk::CHUNK_WIDTH; ++z)
            {
                for (int x = disp; x < Chunk::CHUNK_WIDTH; x += 2)
                {
                    const int my_id = chunk.getBlock(x, y, z).id;
                    const bool cur_air = my_id == 0;

                    const glm::vec3 my_min = glm::vec3{x, y, z};
                    const glm::vec3 my_max = my_min + glm::vec3(1, 1, 1);
                    const BlockDescription &my_desc = registry->getBlock(my_id);

                    const bool px_air = is_air(x + 1, y, z);
                    if (!cur_air && px_air)
                    {
                        gen_face_px(my_min, my_max, my_desc, mesh);
                    }
                    if (x < Chunk::CHUNK_WIDTH - 1 && cur_air && !px_air)
                    {
                        constexpr glm::vec3 offset{1, 0, 0};
                        const int id = chunk.getBlock(x + 1, y, z).id;
                        const BlockDescription &desc = registry->getBlock(id);
                        gen_face_nx(my_min + offset, my_max + offset, desc, mesh);
                    }

                    const bool nx_air = is_air(x - 1, y, z);
                    if (!cur_air && nx_air)
                    {
                        gen_face_nx(my_min, my_max, my_desc, mesh);
                    }
                    if (x > 0 && cur_air && !nx_air)
                    {
                        constexpr glm::vec3 offset{-1, 0, 0};
                        const int id = chunk.getBlock(x - 1, y, z).id;
                        const BlockDescription &desc = registry->getBlock(id);
                        gen_face_px(my_min + offset, my_max + offset, desc, mesh);
                    }

                    const bool py_air = is_air(x, y + 1, z);
                    if (!cur_air && py_air)
                    {
                        gen_face_py(my_min, my_max, my_desc, mesh);
                    }
                    if (y < Chunk::CHUNK_HEIGHT - 1 && cur_air && !py_air)
                    {
                        constexpr glm::vec3 offset{0, 1, 0};
                        const int id = chunk.getBlock(x, y + 1, z).id;
                        const BlockDescription &desc = registry->getBlock(id);
                        gen_face_ny(my_min + offset, my_max + offset, desc, mesh);
                    }

                    const bool ny_air = is_air(x, y - 1, z);
                    if (!cur_air && ny_air)
                    {
                        gen_face_ny(my_min, my_max, my_desc, mesh);
                    }
                    if (y > 0 && cur_air && !ny_air)
                    {
                        constexpr glm::vec3 offset{0, -1, 0};
                        const int id = chunk.getBlock(x, y - 1, z).id;
                        const BlockDescription &desc = registry->getBlock(id);
                        gen_face_py(my_min + offset, my_max + offset, desc, mesh);
                    }

                    const bool pz_air = is_air(x, y, z + 1);
                    if (!cur_air && pz_air)
                    {
                        gen_face_pz(my_min, my_max, my_desc, mesh);
                    }
                    if (z < Chunk::CHUNK_WIDTH - 1 && cur_air && !pz_air)
                    {
                        constexpr glm::vec3 offset{0, 0, 1};
                        const int id = chunk.getBlock(x, y, z + 1).id;
                        const BlockDescription &desc = registry->getBlock(id);
                        gen_face_nz(my_min + offset, my_max + offset, desc, mesh);
                    }

                    const bool nz_air = is_air(x, y, z - 1);
                    if (!cur_air && nz_air)
                    {
                        gen_face_nz(my_min, my_max, my_desc, mesh);
                    }
                    if (z > 0 && cur_air && !nz_air)
                    {
                        constexpr glm::vec3 offset{0, 0, -1};
                        const int id = chunk.getBlock(x, y, z - 1).id;
                        const BlockDescription &desc = registry->getBlock(id);
                        gen_face_pz(my_min + offset, my_max + offset, desc, mesh);
                    }
                }
                disp = !disp;
            }
            disp = !disp;
        }
    }
    {
        chunk.visitRead([&](int x, int y, int z, BlockInfo block) {
            if (block.id == 0)
            {
                return;
            }

            const glm::vec3 min = glm::vec3{x, y, z};
            const glm::vec3 max = min + glm::vec3(1, 1, 1);

            const BlockDescription &desc = registry->getBlock(block.id);
            assert(desc.cached.valid);

            if (is_air(x + 1, y, z))
            {
                gen_face_px(min, max, desc, mesh);
            }
            if (is_air(x - 1, y, z))
            {
                gen_face_nx(min, max, desc, mesh);
            }
            if (is_air(x, y + 1, z))
            {
                gen_face_py(min, max, desc, mesh);
            }
            if (is_air(x, y - 1, z))
            {
                gen_face_ny(min, max, desc, mesh);
            }
            if (is_air(x, y, z + 1))
            {
                gen_face_pz(min, max, desc, mesh);
            }
            if (is_air(x, y, z - 1))
            {
                gen_face_nz(min, max, desc, mesh);
            }
        });
    }

    {
        ScopedProfiler p("flush vbo");
        mesh.flush();
    }
}

void ChunkMeshGenerator::gen_face_py(const glm::vec3 &min, const glm::vec3 &max,
    const BlockDescription &desc, ChunkMesh &mesh)
{
    const BlockDescription::TexCoords &coords = desc.cached.texture_coord_py;

    ChunkMesh::Vertex vs[6];

    // tr 1
    vs[0].pos = glm::vec3{min.x, max.y, min.z};
    vs[0].norm = glm::vec3{0, 1, 0};
    vs[0].uv = coords.top_left;

    vs[1].pos = glm::vec3{min.x, max.y, max.z};
    vs[1].norm = glm::vec3{0, 1, 0};
    vs[1].uv = coords.bottom_left;

    vs[2].pos = glm::vec3{max.x, max.y, max.z};
    vs[2].norm = glm::vec3{0, 1, 0};
    vs[2].uv = coords.bottom_right;

    // tr 2
    vs[3].pos = glm::vec3{min.x, max.y, min.z};
    vs[3].norm = glm::vec3{0, 1, 0};
    vs[3].uv = coords.top_left;

    vs[4].pos = glm::vec3{max.x, max.y, max.z};
    vs[4].norm = glm::vec3{0, 1, 0};
    vs[4].uv = coords.bottom_right;

    vs[5].pos = glm::vec3{max.x, max.y, min.z};
    vs[5].norm = glm::vec3{0, 1, 0};
    vs[5].uv = coords.top_right;

    mesh.addRaw(vs, sizeof(ChunkMesh::Vertex) * 6);
}

void ChunkMeshGenerator::gen_face_ny(const glm::vec3 &min, const glm::vec3 &max,
    const BlockDescription &desc, ChunkMesh &mesh)
{
    const BlockDescription::TexCoords &coords = desc.cached.texture_coord_ny;
    ChunkMesh::Vertex vs[6];

    // tr 1
    vs[0].pos = glm::vec3{min.x, min.y, min.z};
    vs[0].norm = glm::vec3{0, -1, 0};
    vs[0].uv = coords.top_right;

    vs[1].pos = glm::vec3{max.x, min.y, max.z};
    vs[1].norm = glm::vec3{0, -1, 0};
    vs[1].uv = coords.bottom_left;

    vs[2].pos = glm::vec3{min.x, min.y, max.z};
    vs[2].norm = glm::vec3{0, -1, 0};
    vs[2].uv = coords.bottom_right;

    // tr 2
    vs[3].pos = glm::vec3{min.x, min.y, min.z};
    vs[3].norm = glm::vec3{0, -1, 0};
    vs[3].uv = coords.top_right;

    vs[4].pos = glm::vec3{max.x, min.y, min.z};
    vs[4].norm = glm::vec3{0, -1, 0};
    vs[4].uv = coords.top_left;

    vs[5].pos = glm::vec3{max.x, min.y, max.z};
    vs[5].norm = glm::vec3{0, -1, 0};
    vs[5].uv = coords.bottom_left;

    mesh.addRaw(vs, sizeof(ChunkMesh::Vertex) * 6);
}

void ChunkMeshGenerator::gen_face_pz(const glm::vec3 &min, const glm::vec3 &max,
    const BlockDescription &desc, ChunkMesh &mesh)
{
    const BlockDescription::TexCoords &coords = desc.cached.texture_coord_pz;
    ChunkMesh::Vertex vs[6];

    // tr 1
    vs[0].pos = glm::vec3{min.x, max.y, max.z};
    vs[0].norm = glm::vec3{0, 0, 1};
    vs[0].uv = coords.top_left;

    vs[1].pos = glm::vec3{min.x, min.y, max.z};
    vs[1].norm = glm::vec3{0, 0, 1};
    vs[1].uv = coords.bottom_left;

    vs[2].pos = glm::vec3{max.x, min.y, max.z};
    vs[2].norm = glm::vec3{0, 0, 1};
    vs[2].uv = coords.bottom_right;

    // tr 2
    vs[3].pos = glm::vec3{min.x, max.y, max.z};
    vs[3].norm = glm::vec3{0, 0, 1};
    vs[3].uv = coords.top_left;

    vs[4].pos = glm::vec3{max.x, min.y, max.z};
    vs[4].norm = glm::vec3{0, 0, 1};
    vs[4].uv = coords.bottom_right;

    vs[5].pos = glm::vec3{max.x, max.y, max.z};
    vs[5].norm = glm::vec3{0, 0, 1};
    vs[5].uv = coords.top_right;

    mesh.addRaw(vs, sizeof(ChunkMesh::Vertex) * 6);
}

void ChunkMeshGenerator::gen_face_nz(const glm::vec3 &min, const glm::vec3 &max,
    const BlockDescription &desc, ChunkMesh &mesh)
{
    const BlockDescription::TexCoords &coords = desc.cached.texture_coord_nz;
    ChunkMesh::Vertex vs[6];

    // tr 1
    vs[0].pos = glm::vec3{min.x, max.y, min.z};
    vs[0].norm = glm::vec3{0, 0, -1};
    vs[0].uv = coords.top_right;

    vs[1].pos = glm::vec3{max.x, min.y, min.z};
    vs[1].norm = glm::vec3{0, 0, -1};
    vs[1].uv = coords.bottom_left;

    vs[2].pos = glm::vec3{min.x, min.y, min.z};
    vs[2].norm = glm::vec3{0, 0, -1};
    vs[2].uv = coords.bottom_right;

    // tr 2
    vs[3].pos = glm::vec3{min.x, max.y, min.z};
    vs[3].norm = glm::vec3{0, 0, -1};
    vs[3].uv = coords.top_right;

    vs[4].pos = glm::vec3{max.x, max.y, min.z};
    vs[4].norm = glm::vec3{0, 0, -1};
    vs[4].uv = coords.top_left;

    vs[5].pos = glm::vec3{max.x, min.y, min.z};
    vs[5].norm = glm::vec3{0, 0, -1};
    vs[5].uv = coords.bottom_left;

    mesh.addRaw(vs, sizeof(ChunkMesh::Vertex) * 6);
}

void ChunkMeshGenerator::gen_face_px(const glm::vec3 &min, const glm::vec3 &max,
    const BlockDescription &desc, ChunkMesh &mesh)
{
    const BlockDescription::TexCoords &coords = desc.cached.texture_coord_px;
    ChunkMesh::Vertex vs[6];

    // tr 1
    vs[0].pos = glm::vec3{max.x, max.y, max.z};
    vs[0].norm = glm::vec3{1, 0, 0};
    vs[0].uv = coords.top_left;

    vs[1].pos = glm::vec3{max.x, min.y, max.z};
    vs[1].norm = glm::vec3{1, 0, 0};
    vs[1].uv = coords.bottom_left;

    vs[2].pos = glm::vec3{max.x, min.y, min.z};
    vs[2].norm = glm::vec3{1, 0, 0};
    vs[2].uv = coords.bottom_right;

    // tr 2
    vs[3].pos = glm::vec3{max.x, max.y, max.z};
    vs[3].norm = glm::vec3{1, 0, 0};
    vs[3].uv = coords.top_left;

    vs[4].pos = glm::vec3{max.x, min.y, min.z};
    vs[4].norm = glm::vec3{1, 0, 0};
    vs[4].uv = coords.bottom_right;

    vs[5].pos = glm::vec3{max.x, max.y, min.z};
    vs[5].norm = glm::vec3{1, 0, 0};
    vs[5].uv = coords.top_right;

    mesh.addRaw(vs, sizeof(ChunkMesh::Vertex) * 6);
}

void ChunkMeshGenerator::gen_face_nx(const glm::vec3 &min, const glm::vec3 &max,
    const BlockDescription &desc, ChunkMesh &mesh)
{
    const BlockDescription::TexCoords &coords = desc.cached.texture_coord_nx;
    ChunkMesh::Vertex vs[6];

    // tr 1
    vs[0].pos = glm::vec3{min.x, max.y, max.z};
    vs[0].norm = glm::vec3{-1, 0, 0};
    vs[0].uv = coords.top_right;

    vs[1].pos = glm::vec3{min.x, min.y, min.z};
    vs[1].norm = glm::vec3{-1, 0, 0};
    vs[1].uv = coords.bottom_left;

    vs[2].pos = glm::vec3{min.x, min.y, max.z};
    vs[2].norm = glm::vec3{-1, 0, 0};
    vs[2].uv = coords.bottom_right;

    // tr 2
    vs[3].pos = glm::vec3{min.x, max.y, max.z};
    vs[3].norm = glm::vec3{-1, 0, 0};
    vs[3].uv = coords.top_right;

    vs[4].pos = glm::vec3{min.x, max.y, min.z};
    vs[4].norm = glm::vec3{-1, 0, 0};
    vs[4].uv = coords.top_left;

    vs[5].pos = glm::vec3{min.x, min.y, min.z};
    vs[5].norm = glm::vec3{-1, 0, 0};
    vs[5].uv = coords.bottom_left;

    mesh.addRaw(vs, sizeof(ChunkMesh::Vertex) * 6);
}
