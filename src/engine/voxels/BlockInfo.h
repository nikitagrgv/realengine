#pragma once

struct BlockInfo
{
public:
    BlockInfo() = default;

    explicit BlockInfo(int id)
        : id(id)
    {}

    friend bool operator==(const BlockInfo &lhs, const BlockInfo &rhs) { return lhs.id == rhs.id; }
    friend bool operator!=(const BlockInfo &lhs, const BlockInfo &rhs) { return !(lhs == rhs); }

public:
    int id{0};
};