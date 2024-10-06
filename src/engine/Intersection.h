#pragma once

#include <glm/vec3.hpp>

class Node;

struct Intersection
{
    bool valid{false};
    Node *node{};
    glm::vec3 point{0.0f};
};