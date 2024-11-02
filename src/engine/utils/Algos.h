#pragma once

#include <algorithm>

namespace Alg
{

template<typename T, typename F>
void removeIf(T &container, const F &f)
{
    const auto end = container.end();
    const auto new_end = std::remove_if(container.begin(), end, f);
    container.erase(new_end, end);
}

template<typename T, typename V>
void removeOne(T &container, const V &value)
{
    const auto it = std::find(container.begin(), container.end(), value);
    if (it != container.end())
    {
        container.erase(it);
    }
}

template<typename T, typename F>
void removeOneIf(T &container, const F &f)
{
    const auto it = std::find_if(container.begin(), container.end(), f);
    if (it != container.end())
    {
        container.erase(it);
    }
}

template<typename T, typename F>
int countIf(const T &container, const F &f)
{
    return std::count_if(container.begin(), container.end(), f);
}

template<typename T, typename F>
int anyOf(const T &container, const F &f)
{
    return std::any_of(container.begin(), container.end(), f);
}

template<typename T, typename F>
int allOf(const T &container, const F &f)
{
    return std::all_of(container.begin(), container.end(), f);
}

template<typename T, typename F>
int noneOf(const T &container, const F &f)
{
    return std::none_of(container.begin(), container.end(), f);
}

template<typename T, typename V>
bool contains(const T &container, const V &value)
{
    const auto it = std::find(container.begin(), container.end(), value);
    return it != container.end();
}

} // namespace Alg
