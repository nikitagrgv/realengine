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
    }

    void addRenderedIndices(uint64_t count) { num_rendered_indices_in_frame_ += count; }
    void addCompiledShaders(uint64_t count) { num_compiled_shaders_in_frame_ += count; }

    // Frame
    uint64_t getNumRenderedIndicesInFrame() const { return num_rendered_indices_in_frame_; }
    uint64_t getNumCompiledShadersInFrame() const { return num_compiled_shaders_in_frame_; }

    // Total
    uint64_t getNumRenderedIndicesTotal() const { return num_rendered_indices_total_; }
    uint64_t getNumCompiledShadersTotal() const { return num_compiled_shaders_total_; }

private:
    // Frame
    uint64_t num_rendered_indices_in_frame_{0};
    uint64_t num_compiled_shaders_in_frame_{0};

    // Total
    uint64_t num_rendered_indices_total_{0};
    uint64_t num_compiled_shaders_total_{0};
};
