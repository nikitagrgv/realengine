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
public:
    REMOVE_COPY_CLASS(BlockDescription);

    BlockDescription(BlockDescription &&other) noexcept = default;
    BlockDescription &operator=(BlockDescription &&other) noexcept = default;

    BlockDescription() = default;

    bool isValid() const { return id != -1; }

public:
    std::string name{"None"};
    int id{-1};
    BlockType type{BlockType::SOLID};
    bool transparent{false};

    union
    {
        int texture_indexes[6]{0, 0, 0, 0, 0, 0};
        struct
        {
            int texture_index_px;
            int texture_index_nx;
            int texture_index_py;
            int texture_index_ny;
            int texture_index_pz;
            int texture_index_nz;
        };
    };

    /////////////////////////////////////////////////////////

    struct TexCoords
    {
        glm::vec2 bottom_left;
        glm::vec2 bottom_right;
        glm::vec2 top_left;
        glm::vec2 top_right;
    };

    struct Cached
    {
        bool valid{false};
        union
        {
            TexCoords texture_coords[6];
            struct
            {
                TexCoords texture_coord_px;
                TexCoords texture_coord_nx;
                TexCoords texture_coord_py;
                TexCoords texture_coord_ny;
                TexCoords texture_coord_pz;
                TexCoords texture_coord_nz;
            };
        };
    } cached;
};