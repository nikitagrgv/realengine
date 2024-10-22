#pragma once

struct BlockInfo
{
    BlockInfo() = default;

    explicit BlockInfo(int id)
        : id(id)
    {}

    int id{0};
};