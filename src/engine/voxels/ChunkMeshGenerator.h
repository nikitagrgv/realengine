#pragma once

#include <glm/vec3.hpp>


struct NeighbourChunks;
struct Chunk;
struct ChunkMesh;
struct BlockDescription;

class ChunkMeshGenerator
{
public:
    void rebuildMesh(const Chunk &chunk, ChunkMesh &mesh, const NeighbourChunks &neighbours);

private:
    static void gen_face_py(const glm::vec3 &min, const glm::vec3 &max,
        const BlockDescription &desc, ChunkMesh &mesh);
    static void gen_face_ny(const glm::vec3 &min, const glm::vec3 &max,
        const BlockDescription &desc, ChunkMesh &mesh);
    static void gen_face_pz(const glm::vec3 &min, const glm::vec3 &max,
        const BlockDescription &desc, ChunkMesh &mesh);
    static void gen_face_nz(const glm::vec3 &min, const glm::vec3 &max,
        const BlockDescription &desc, ChunkMesh &mesh);
    static void gen_face_px(const glm::vec3 &min, const glm::vec3 &max,
        const BlockDescription &desc, ChunkMesh &mesh);
    static void gen_face_nx(const glm::vec3 &min, const glm::vec3 &max,
        const BlockDescription &desc, ChunkMesh &mesh);
};
