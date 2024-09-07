#pragma once

#include <cstdint>

struct Statistics
{
    void reset()
    {
        numRenderedIndices = 0;
        numCompiledShadersInFrame = 0;
    }

    uint64_t numRenderedIndices{0};
    uint64_t numCompiledShadersInFrame{0};
};
