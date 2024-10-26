#include "ChunkMesh.h"

ChunkMesh::ChunkMesh()
{
    vao.bind();
    vao.addAttributeFloat(3); // pos
    vao.addAttributeFloat(3); // norm
    vao.addAttributeFloat(2); // uv

    vbo.bind();

    vao.flush();

    vbo.flush(true);
}
