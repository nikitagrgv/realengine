#include "World.h"

#include "Intersection.h"
#include "Random.h"
#include "math/IntersectionMath.h"

#include <NodeMesh.h>

#include <cassert>

Node *World::getNodeByIndex(int index)
{
    return nodes_[index].get();
}

Node *World::getNodeById(int id)
{
    const int index = getNodeIndexById(id);
    return index == -1 ? nullptr : getNodeByIndex(index);
}

int World::getNodeIndexById(int id)
{
    const auto it = index_by_id_.find(id);
    if (it == index_by_id_.end())
    {
        return -1;
    }
    return it->second;
}

int World::getNodeIndex(Node *node)
{
    return getNodeIndexById(node->getId());
}

Node *World::createNode(Node::Type type)
{
    constexpr int MIN_ID = 1;
    constexpr int MAX_ID = INT_MAX;
    int id = Random::randInt(MIN_ID, MAX_ID);
    while (hasNodeId(id))
    {
        id = Random::randInt(MIN_ID, MAX_ID);
    }

    Node *node = nullptr;
    switch (type)
    {
    case Node::Type::Mesh:
    {
        node = new NodeMesh(id);
        break;
    }
    default: assert(0); break;
    }

    const int index = nodes_.size();
    nodes_.push_back(std::unique_ptr<Node>(node));
    index_by_id_[id] = index;

    assert(nodes_.size() == index_by_id_.size());
    return node;
}

void World::removeNode(Node *node)
{
    removeNodeById(node->getId());
}

void World::removeNodeByIndex(int index)
{
    const Node *n = getNodeByIndex(index);

    {
        const auto it = index_by_id_.find(n->getId());
        assert(it != index_by_id_.end());
        index_by_id_.erase(it);
    }

    auto &last = nodes_[nodes_.size() - 1];
    auto &rem = nodes_[index];

    const int id_of_last = last->getId();

    std::swap(last, rem);

    index_by_id_[id_of_last] = index;

    nodes_.resize(nodes_.size() - 1);

    assert(nodes_.size() == index_by_id_.size());
}

void World::removeNodeById(int id)
{
    const int index = getNodeIndexById(id);
    if (index == -1)
    {
        return;
    }
    removeNodeByIndex(index);
}

int World::getNumNodes() const
{
    assert(nodes_.size() == index_by_id_.size());
    return nodes_.size();
}

bool World::hasNodeIndex(int index) const
{
    return index >= 0 && index < nodes_.size();
}

bool World::hasNodeId(int id) const
{
    return index_by_id_.find(id) != index_by_id_.end();
}

void World::getDirectionIntersection(const glm::vec3 &origin, const glm::vec3 &direction,
    SimpleIntersection &intersection) const
{
    if (nodes_.empty())
    {
        intersection.clear();
        return;
    }

    SimpleIntersection nearest_intersection;
    for (auto &nptr : nodes_)
    {
        Node *node = nptr.get();

        SimpleIntersection ni;
        math::getDirectionIntersection(node->getGlobalBoundBox(), origin, direction, ni);
        if (ni.isCloserThan(nearest_intersection))
        {
            nearest_intersection = ni;
        }
    }
    intersection = nearest_intersection;
}
