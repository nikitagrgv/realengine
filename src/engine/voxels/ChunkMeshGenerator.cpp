#include "ChunkMeshGenerator.h"

#include "BlockDescription.h"
#include "BlocksRegistry.h"
#include "Chunk.h"
#include "ChunkMesh.h"
#include "EngineGlobals.h"
#include "VoxelEngine.h"
#include "profiler/ScopedProfiler.h"

void ChunkMeshGenerator::rebuildMesh(const Chunk &chunk, ChunkMesh &mesh)
{
    assert(chunk.dirty_);

    SCOPED_PROFILER;

    mesh.vbo.clear();

    BlocksRegistry *registry = eng.vox->getRegistry();

    const auto is_air_or_outside = [&](int x, int y, int z) {
        if (!Chunk::isInsideChunk(x, y, z))
        {
            return true;
        }
        const BlockInfo block = chunk.getBlock(x, y, z);
        if (block.id == 0)
        {
            return true;
        }
        // TODO# check transparent
        return false;
    };

    chunk.visitRead([&](int x, int y, int z, BlockInfo block) {
        if (block.id == 0)
        {
            return;
        }

        const glm::vec3 min = glm::vec3{x, y, z};
        const glm::vec3 max = min + glm::vec3(1, 1, 1);

        const BlockDescription &desc = registry->getBlock(block.id);
        assert(desc.cached.valid);

        if (is_air_or_outside(x + 1, y, z))
        {
            gen_face_px(min, max, desc, mesh);
        }
        if (is_air_or_outside(x - 1, y, z))
        {
            gen_face_nx(min, max, desc, mesh);
        }
        if (is_air_or_outside(x, y + 1, z))
        {
            gen_face_py(min, max, desc, mesh);
        }
        if (is_air_or_outside(x, y - 1, z))
        {
            gen_face_ny(min, max, desc, mesh);
        }
        if (is_air_or_outside(x, y, z + 1))
        {
            gen_face_pz(min, max, desc, mesh);
        }
        if (is_air_or_outside(x, y, z - 1))
        {
            gen_face_nz(min, max, desc, mesh);
        }
    });

    {
        ScopedProfiler p("flush vbo");
        mesh.vbo.flush(true);
    }
}

void ChunkMeshGenerator::gen_face_py(const glm::vec3 &min, const glm::vec3 &max,
    const BlockDescription &desc, ChunkMesh &mesh)
{
    const BlockDescription::TexCoords &coords = desc.cached.texture_coord_py;
    ChunkMesh::Vertex v;

    v.norm = glm::vec3{0, 1, 0};

    // tr 1
    v.pos = glm::vec3{min.x, max.y, min.z};
    v.uv = coords.top_left;
    mesh.vbo.addVertex(v);

    v.pos = glm::vec3{min.x, max.y, max.z};
    v.uv = coords.bottom_left;
    mesh.vbo.addVertex(v);

    v.pos = glm::vec3{max.x, max.y, max.z};
    v.uv = coords.bottom_right;
    mesh.vbo.addVertex(v);

    // tr 2
    v.pos = glm::vec3{min.x, max.y, min.z};
    v.uv = coords.top_left;
    mesh.vbo.addVertex(v);

    v.pos = glm::vec3{max.x, max.y, max.z};
    v.uv = coords.bottom_right;
    mesh.vbo.addVertex(v);

    v.pos = glm::vec3{max.x, max.y, min.z};
    v.uv = coords.top_right;
    mesh.vbo.addVertex(v);
}

void ChunkMeshGenerator::gen_face_ny(const glm::vec3 &min, const glm::vec3 &max,
    const BlockDescription &desc, ChunkMesh &mesh)
{
    const BlockDescription::TexCoords &coords = desc.cached.texture_coord_ny;
    ChunkMesh::Vertex v;

    v.norm = glm::vec3{0, -1, 0};

    // tr 1
    v.pos = glm::vec3{min.x, min.y, min.z};
    v.uv = coords.top_right;
    mesh.vbo.addVertex(v);

    v.pos = glm::vec3{max.x, min.y, max.z};
    v.uv = coords.bottom_left;
    mesh.vbo.addVertex(v);

    v.pos = glm::vec3{min.x, min.y, max.z};
    v.uv = coords.bottom_right;
    mesh.vbo.addVertex(v);


    // tr 2
    v.pos = glm::vec3{min.x, min.y, min.z};
    v.uv = coords.top_right;
    mesh.vbo.addVertex(v);

    v.pos = glm::vec3{max.x, min.y, min.z};
    v.uv = coords.top_left;
    mesh.vbo.addVertex(v);

    v.pos = glm::vec3{max.x, min.y, max.z};
    v.uv = coords.bottom_left;
    mesh.vbo.addVertex(v);
}

void ChunkMeshGenerator::gen_face_pz(const glm::vec3 &min, const glm::vec3 &max,
    const BlockDescription &desc, ChunkMesh &mesh)
{
    const BlockDescription::TexCoords &coords = desc.cached.texture_coord_pz;
    ChunkMesh::Vertex v;

    v.norm = glm::vec3{0, 0, 1};

    // tr 1
    v.pos = glm::vec3{min.x, max.y, max.z};
    v.uv = coords.top_left;
    mesh.vbo.addVertex(v);

    v.pos = glm::vec3{min.x, min.y, max.z};
    v.uv = coords.bottom_left;
    mesh.vbo.addVertex(v);

    v.pos = glm::vec3{max.x, min.y, max.z};
    v.uv = coords.bottom_right;
    mesh.vbo.addVertex(v);

    // tr 2
    v.pos = glm::vec3{min.x, max.y, max.z};
    v.uv = coords.top_left;
    mesh.vbo.addVertex(v);

    v.pos = glm::vec3{max.x, min.y, max.z};
    v.uv = coords.bottom_right;
    mesh.vbo.addVertex(v);

    v.pos = glm::vec3{max.x, max.y, max.z};
    v.uv = coords.top_right;
    mesh.vbo.addVertex(v);
}

void ChunkMeshGenerator::gen_face_nz(const glm::vec3 &min, const glm::vec3 &max,
    const BlockDescription &desc, ChunkMesh &mesh)
{
    const BlockDescription::TexCoords &coords = desc.cached.texture_coord_nz;
    ChunkMesh::Vertex v;

    v.norm = glm::vec3{0, 0, -1};

    // tr 1
    v.pos = glm::vec3{min.x, max.y, min.z};
    v.uv = coords.top_right;
    mesh.vbo.addVertex(v);

    v.pos = glm::vec3{max.x, min.y, min.z};
    v.uv = coords.bottom_left;
    mesh.vbo.addVertex(v);

    v.pos = glm::vec3{min.x, min.y, min.z};
    v.uv = coords.bottom_right;
    mesh.vbo.addVertex(v);

    // tr 2
    v.pos = glm::vec3{min.x, max.y, min.z};
    v.uv = coords.top_right;
    mesh.vbo.addVertex(v);

    v.pos = glm::vec3{max.x, max.y, min.z};
    v.uv = coords.top_left;
    mesh.vbo.addVertex(v);

    v.pos = glm::vec3{max.x, min.y, min.z};
    v.uv = coords.bottom_left;
    mesh.vbo.addVertex(v);
}

void ChunkMeshGenerator::gen_face_px(const glm::vec3 &min, const glm::vec3 &max,
    const BlockDescription &desc, ChunkMesh &mesh)
{
    const BlockDescription::TexCoords &coords = desc.cached.texture_coord_px;
    ChunkMesh::Vertex v;

    v.norm = glm::vec3{1, 0, 0};

    // tr 1
    v.pos = glm::vec3{max.x, max.y, max.z};
    v.uv = coords.top_left;
    mesh.vbo.addVertex(v);

    v.pos = glm::vec3{max.x, min.y, max.z};
    v.uv = coords.bottom_left;
    mesh.vbo.addVertex(v);

    v.pos = glm::vec3{max.x, min.y, min.z};
    v.uv = coords.bottom_right;
    mesh.vbo.addVertex(v);

    // tr 2
    v.pos = glm::vec3{max.x, max.y, max.z};
    v.uv = coords.top_left;
    mesh.vbo.addVertex(v);

    v.pos = glm::vec3{max.x, min.y, min.z};
    v.uv = coords.bottom_right;
    mesh.vbo.addVertex(v);

    v.pos = glm::vec3{max.x, max.y, min.z};
    v.uv = coords.top_right;
    mesh.vbo.addVertex(v);
}

void ChunkMeshGenerator::gen_face_nx(const glm::vec3 &min, const glm::vec3 &max,
    const BlockDescription &desc, ChunkMesh &mesh)
{
    const BlockDescription::TexCoords &coords = desc.cached.texture_coord_nx;
    ChunkMesh::Vertex v;

    v.norm = glm::vec3{-1, 0, 0};

    // tr 1
    v.pos = glm::vec3{min.x, max.y, max.z};
    v.uv = coords.top_right;
    mesh.vbo.addVertex(v);

    v.pos = glm::vec3{min.x, min.y, min.z};
    v.uv = coords.bottom_left;
    mesh.vbo.addVertex(v);

    v.pos = glm::vec3{min.x, min.y, max.z};
    v.uv = coords.bottom_right;
    mesh.vbo.addVertex(v);

    // tr 2
    v.pos = glm::vec3{min.x, max.y, max.z};
    v.uv = coords.top_right;
    mesh.vbo.addVertex(v);

    v.pos = glm::vec3{min.x, max.y, min.z};
    v.uv = coords.top_left;
    mesh.vbo.addVertex(v);

    v.pos = glm::vec3{min.x, min.y, min.z};
    v.uv = coords.bottom_left;
    mesh.vbo.addVertex(v);
}
