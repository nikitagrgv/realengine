#pragma once

#include <glm/vec3.hpp>

class Node;

struct SimpleIntersection
{
public:
    bool isCloserThan(const SimpleIntersection &other) const
    {
        if (!valid)
        {
            return false;
        }
        if (!other.valid)
        {
            return true;
        }
        return distance < other.distance;
    }

    bool isValid() const { return valid; }

    void clear() { valid = false; }

public:
    bool valid{false};
    float distance{false};
};