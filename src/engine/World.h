#pragma once
#include "Node.h"

#include <memory>
#include <unordered_map>
#include <vector>

class Node;


class World
{
public:
    Node *getNodeByIndex(int index);
    Node *getNodeById(int id);

    int getNodeIndexById(int id);
    int getNodeIndex(Node *node);

    Node *createNode(Node::Type type);

    void removeNode(Node *node);
    void removeNodeByIndex(int index);
    void removeNodeById(int id);

    int getNumNodes() const;

    bool hasNodeId(int id) const;

private:
    std::vector<std::unique_ptr<Node>> nodes_;
    std::unordered_map<int, int> index_by_id_;
};
