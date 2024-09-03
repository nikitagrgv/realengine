#pragma once

#include <cassert>
#include <memory>
#include <string>
#include <unordered_map>

class T
{
    std::unique_ptr<int> g;
};

// template<class T>
class AbstractManager
{
public:
    AbstractManager(std::string empty_name_base);

    T *create(const char *name = nullptr);

    T *add(T obj, const char *name = nullptr);

    T *get(const char *name);

    void remove(const char *name);
    void remove(T *obj);
    void remove(int index);

protected:
    std::string empty_name_base_;

    struct Object
    {
        std::string name;
        std::unique_ptr<T> obj;
    };
    std::vector<Object> objects_;

    std::unordered_map<std::string, int> by_name_;
    std::unordered_map<T *, int> by_ptr;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

// template<typename T>
inline AbstractManager::AbstractManager(std::string empty_name_base)
    : empty_name_base_(std::move(empty_name_base))
{}

// template<typename T>
inline T *AbstractManager::create(const char *name)
{
    T obj;
    return add(std::move(obj), name);
}

// template<typename T>
inline T *AbstractManager::add(T obj, const char *name)
{
    std::string name_string = name ? name : "";

    // TODO: shitty
    if (name_string.empty())
    {
        int i = 0;
        while (true)
        {
            name_string = empty_name_base_;
            name_string += std::to_string(i);
            if (by_name_.find(name_string) == by_name_.end())
            {
                break;
            }
            i++;
        }
    }
    else
    {
        auto it = by_name_.find(name_string);
        assert(it == by_name_.end());
        if (it != by_name_.end())
        {
            return nullptr;
        }
    }

    auto unique_ptr = std::make_unique<T>(std::move(obj));
    auto ptr = unique_ptr.get();

    const int index = int(objects_.size());

    Object o;
    o.name = name_string;
    o.obj = std::move(unique_ptr);
    objects_.push_back(std::move(o));

    by_name_[name_string] = std::move(unique_ptr);
    by_ptr[ptr] = std::move(name_string);
    return ptr;
}

// template<typename T>
inline T *AbstractManager::get(const char *name)
{
    auto it = objects_.find(name);
    if (it == objects_.end())
    {
        return nullptr;
    }
    return it->second.get();
}

// template<typename T>
inline void AbstractManager::remove(const char *name)
{
    auto it = objects_.find(name);
    if (it == objects_.end())
    {
        return;
    }
    auto name_it = by_ptr.find(it->second.get());
    assert(name_it != by_ptr.end());
    by_ptr.erase(name_it);
    objects_.erase(it);
}

// template<typename T>
inline void AbstractManager::remove(T *obj)
{
    auto name_it = by_ptr.find(obj);
    if (name_it == by_ptr.end())
    {
        return;
    }
    auto it = objects_.find(name_it->second);
    assert(it != objects_.end());
    by_ptr.erase(name_it);
    objects_.erase(it);
}

// template<typename T>
inline void AbstractManager::remove(int index) {}