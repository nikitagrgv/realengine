#pragma once

#include "Node.h"

#include <memory>
#include <unordered_map>
#include <vector>

struct SimpleIntersection;
class Node;


class World
{
public:
    Node *getNodeByIndex(int index);
    Node *getNodeById(int id);

    int getNodeIndexById(int id);
    int getNodeIndex(Node *node);

    Node *createNode(Node::Type type);

    template<typename T>
    T *createNode();

    void removeNode(Node *node);
    void removeNodeByIndex(int index);
    void removeNodeById(int id);

    int getNumNodes() const;

    bool hasNodeIndex(int index) const;
    bool hasNodeId(int id) const;

    void getDirectionIntersection(const glm::vec3 &origin, const glm::vec3 &direction,
        SimpleIntersection &intersection) const;

private:
    std::vector<std::unique_ptr<Node>> nodes_;
    std::unordered_map<int, int> index_by_id_;
};


template<typename T>
T *World::createNode()
{
    Node *node = createNode(T::getTypeStatic());
    assert(dynamic_cast<T *>(node));
    return static_cast<T *>(node);
}
