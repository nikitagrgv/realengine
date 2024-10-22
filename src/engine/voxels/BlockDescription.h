#pragma once

#include "glm/vec2.hpp"
#include <string>

enum class BlockType
{
    AIR,
    SOLID,
    LIQUID
};

struct BlockDescription
{
    std::string name{"None"};
    int id{-1};
    BlockType type{BlockType::SOLID};
    bool transparent{false};

    union
    {
        int texture_indexes[6];
        struct
        {
            int texture_index_px{0};
            int texture_index_nx{0};
            int texture_index_py{0};
            int texture_index_ny{0};
            int texture_index_pz{0};
            int texture_index_nz{0};
        };
    };

    struct Cached
    {
        bool valid{false};
        union
        {
            glm::vec2 texture_coords[6];
            struct
            {
                glm::vec2 texture_coord_px;
                glm::vec2 texture_coord_nx;
                glm::vec2 texture_coord_py;
                glm::vec2 texture_coord_ny;
                glm::vec2 texture_coord_pz;
                glm::vec2 texture_coord_nz;
            };
        };
    } cached;
};