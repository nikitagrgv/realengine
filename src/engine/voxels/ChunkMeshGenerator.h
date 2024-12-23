#pragma once

#include <glm/vec3.hpp>
#include "Common.h"

struct ExtendedNeighbourChunks;
struct Chunk;
struct ChunkMesh;

class ChunkMeshGenerator
{
public:
    void rebuildMesh(const Chunk &chunk, ChunkMesh &mesh,
        const ExtendedNeighbourChunks &neighbours);

private:
    static void gen_face_py(const glm::vec3 &min, const glm::vec3 &max,
        const Descriptions3x3 &descs, ChunkMesh &mesh);
    static void gen_face_ny(const glm::vec3 &min, const glm::vec3 &max,
        const Descriptions3x3 &descs, ChunkMesh &mesh);
    static void gen_face_pz(const glm::vec3 &min, const glm::vec3 &max,
        const Descriptions3x3 &descs, ChunkMesh &mesh);
    static void gen_face_nz(const glm::vec3 &min, const glm::vec3 &max,
        const Descriptions3x3 &descs, ChunkMesh &mesh);
    static void gen_face_px(const glm::vec3 &min, const glm::vec3 &max,
        const Descriptions3x3 &descs, ChunkMesh &mesh);
    static void gen_face_nx(const glm::vec3 &min, const glm::vec3 &max,
        const Descriptions3x3 &descs, ChunkMesh &mesh);
};
