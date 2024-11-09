#pragma once

#include <cstdint>

struct Statistics
{
    void finishFrame()
    {
        num_compiled_shaders_total_ += num_compiled_shaders_in_frame_;
        num_rendered_indices_total_ += num_rendered_indices_in_frame_;

        num_rendered_indices_in_frame_ = 0;
        num_compiled_shaders_in_frame_ = 0;

        vox.num_rendered_chunks_in_frame = 0;
        vox.num_rendered_vertices_in_frame = 0;
    }

    void addRenderedIndices(uint64_t count) { num_rendered_indices_in_frame_ += count; }
    void addCompiledShaders(uint64_t count) { num_compiled_shaders_in_frame_ += count; }

    // Frame
    uint64_t getNumRenderedIndicesInFrame() const { return num_rendered_indices_in_frame_; }
    uint64_t getNumCompiledShadersInFrame() const { return num_compiled_shaders_in_frame_; }

    // Total
    uint64_t getNumRenderedIndicesTotal() const { return num_rendered_indices_total_; }
    uint64_t getNumCompiledShadersTotal() const { return num_compiled_shaders_total_; }

    ///////////////////////////////////////////
    // Voxel
    void addRenderedChunks(uint64_t count) { vox.num_rendered_chunks_in_frame += count; }
    void addRenderedChunksVertices(uint64_t count) { vox.num_rendered_vertices_in_frame += count; }

    // Frame
    uint64_t getNumRenderedChunksInFrame() const { return vox.num_rendered_chunks_in_frame; }
    uint64_t getNumRenderChunksVerticesInFrame() const
    {
        return vox.num_rendered_vertices_in_frame;
    }

private:
    // Frame
    uint64_t num_rendered_indices_in_frame_{0};
    uint64_t num_compiled_shaders_in_frame_{0};

    // Total
    uint64_t num_rendered_indices_total_{0};
    uint64_t num_compiled_shaders_total_{0};

    // Voxel
    struct
    {
        // Frame
        uint64_t num_rendered_chunks_in_frame{0};
        uint64_t num_rendered_vertices_in_frame{0};
    } vox;
};
