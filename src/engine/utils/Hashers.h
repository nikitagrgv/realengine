#pragma once

#include <glm/vec2.hpp>

namespace std
{

template<>
struct hash<glm::ivec2>
{
    std::size_t operator()(const glm::ivec2 &v) const noexcept
    {
        return std::hash<int>()(v.x) ^ (std::hash<int>()(v.y) << 1);
    }
};

} // namespace std