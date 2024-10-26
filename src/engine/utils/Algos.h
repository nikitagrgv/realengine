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

} // namespace Alg
