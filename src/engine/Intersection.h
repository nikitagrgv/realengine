#pragma once

#include <glm/vec3.hpp>

class Node;
struct SimpleNodeIntersection;

struct SimpleIntersection
{
public:
    SimpleIntersection() = default;

    explicit SimpleIntersection(float distance, glm::vec3 point)
        : valid(true)
        , distance(distance)
        , point(point)
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

    float getDistance() const { return distance; }
    glm::vec3 getPoint() const { return point; }

    void set(float distance, const glm::vec3 &point)
    {
        valid = true;
        this->distance = distance;
        this->point = point;
    }

private:
    bool valid{false};
    float distance{false};
    glm::vec3 point{};
};

struct SimpleNodeIntersection : SimpleIntersection
{
public:
    SimpleNodeIntersection() = default;

    explicit SimpleNodeIntersection(Node *node, float distance, glm::vec3 point)
        : node(node)
        , distance(distance)
        , point(point)
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
        if (!other.isValid())
        {
            return true;
        }
        return distance < other.getDistance();
    }

    bool isValid() const { return node; }

    void clear() { node = nullptr; }

    Node *getNode() const { return node; }
    float getDistance() const { return distance; }
    glm::vec3 getPoint() const { return point; }

    void set(Node *node, float distance, const glm::vec3 &point)
    {
        this->node = node;
        this->distance = distance;
        this->point = point;
    }

    SimpleIntersection toSimpleIntersection() const { return SimpleIntersection{distance, point}; }

private:
    Node *node{};
    float distance{false};
    glm::vec3 point{0.0f};
};

////////////////////////////////////////////////////////////////////////////////////////////////////

inline bool SimpleIntersection::isCloserThan(const SimpleNodeIntersection &other) const
{
    if (!valid)
    {
        return false;
    }
    if (!other.isValid())
    {
        return true;
    }
    return distance < other.getDistance();
}