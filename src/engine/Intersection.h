#pragma once

#include <glm/vec3.hpp>

class Node;
struct SimpleNodeIntersection;

struct SimpleIntersection
{
public:
    SimpleIntersection() = default;

    explicit SimpleIntersection(bool valid, float distance = 0.0f)
        : valid(valid)
        , distance(distance)
    {}

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

    void takeCloser(const SimpleIntersection &other)
    {
        if (other.isCloserThan(*this))
        {
            *this = other;
        }
    }

    bool isCloserThan(const SimpleNodeIntersection &other) const;

    bool isValid() const { return valid; }

    void clear() { valid = false; }

public:
    bool valid{false};
    float distance{false};
};

struct SimpleNodeIntersection
{
public:
    SimpleNodeIntersection() = default;

    explicit SimpleNodeIntersection(Node *node, float distance = 0.0f)
        : node(node)
        , distance(distance)
    {}

    bool isCloserThan(const SimpleNodeIntersection &other) const
    {
        if (!node)
        {
            return false;
        }
        if (!other.node)
        {
            return true;
        }
        return distance < other.distance;
    }

    bool isCloserThan(const SimpleIntersection &other) const
    {
        if (!node)
        {
            return false;
        }
        if (!other.valid)
        {
            return true;
        }
        return distance < other.distance;
    }

    bool isValid() const { return node; }

    void clear() { node = nullptr; }

    SimpleIntersection toSimpleIntersection() const
    {
        return SimpleIntersection{node != nullptr, distance};
    }

public:
    Node *node{};
    float distance{false};
};

////////////////////////////////////////////////////////////////////////////////////////////////////

inline bool SimpleIntersection::isCloserThan(const SimpleNodeIntersection &other) const
{
    if (!valid)
    {
        return false;
    }
    if (!other.node)
    {
        return true;
    }
    return distance < other.distance;
}