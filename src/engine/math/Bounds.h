#pragma once

#include <glm/mat4x4.hpp>

namespace math
{

struct BoundBox
{
public:
    BoundBox() = default;

    BoundBox(const glm::vec3 &min, const glm::vec3 &max)
        : min(min)
        , max(max)
    {
        assert(min.x <= max.x && min.y <= max.y && min.z <= max.z);
    }

    glm::vec3 getCenter() const { return (min + max) * 0.5f; }
    glm::vec3 getSize() const { return max - min; }

    void expand(glm::vec3 point)
    {
        // clang-format off
        if (point.x < min.x)
            min.x = point.x;
        if (point.y < min.y)
            min.y = point.y;
        if (point.z < min.z)
            min.z = point.z;
        if (point.x > max.x)
            max.x = point.x;
        if (point.y > max.y)
            max.y = point.y;
        if (point.z > max.z)
            max.z = point.z;
        // clang-format on
    }

    bool contains(const glm::vec3 &point) const
    {
        return point.x >= min.x && point.y >= min.y && point.z >= min.z && point.x <= max.x
            && point.y <= max.y && point.z <= max.z;
    }

    bool contains(const BoundBox &box) const
    {
        return box.min.x >= min.x && box.min.y >= min.y && box.min.z >= min.z && box.max.x <= max.x
            && box.max.y <= max.y && box.max.z <= max.z;
    }

    bool intersects(const BoundBox &box) const
    {
        assert(0 && "TODO");
        return false;
    }

    BoundBox transformed(const glm::mat4 &mat) const
    {
        glm::vec3 new_center = mat * glm::vec4(getCenter(), 1.0f);
        glm::vec3 old_edge = getSize() * 0.5f;
        // clang-format off
        glm::vec3 new_edge = glm::vec3(
            glm::abs(mat[0][0]) * old_edge.x + glm::abs(mat[0][1]) * old_edge.y + glm::abs(mat[0][2]) * old_edge.z,
            glm::abs(mat[1][0]) * old_edge.x + glm::abs(mat[1][1]) * old_edge.y + glm::abs(mat[1][2]) * old_edge.z,
            glm::abs(mat[2][0]) * old_edge.x + glm::abs(mat[2][1]) * old_edge.y + glm::abs(mat[2][2]) * old_edge.z
        );
        // clang-format on

        return BoundBox(new_center - new_edge, new_center + new_edge);
    }

public:
    glm::vec3 min{0.0f};
    float alignment_padding_1{0.0f};
    glm::vec3 max{0.0f};
    float alignment_padding_2{0.0f};
};

} // namespace math
