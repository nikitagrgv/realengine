#pragma once

#include <memory>
#include <string>
#include <unordered_map>

template<class T>
class AbstractManager
{
public:
    AbstractManager(std::string empty_name_base);

    T *create(const char *name = nullptr);

    T *add(T obj, const char *name = nullptr);

    T *get(const char *name);

    void remove(const char *name);
    void remove(T *obj);

protected:
    std::string empty_name_base_;
    std::unordered_map<std::string, std::unique_ptr<T>> objects_;
    std::unordered_map<T *, std::string> names_;
};

template<typename T>
AbstractManager<T>::AbstractManager(std::string empty_name_base)
    : empty_name_base_(std::move(empty_name_base))
{}

template<typename T>
T *AbstractManager<T>::create(const char *name)
{
    T obj;
    return add(std::move(obj), name);
}

template<typename T>
T *AbstractManager<T>::add(T obj, const char *name)
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
            if (objects_.find(name_string) == objects_.end())
            {
                break;
            }
            i++;
        }
    }
    else
    {
        auto it = objects_.find(name_string);
        assert(it == objects_.end());
        if (it != objects_.end())
        {
            return nullptr;
        }
    }

    auto unique_ptr = std::make_unique<T>(std::move(obj));
    auto ptr = unique_ptr.get();
    objects_[name_string] = std::move(unique_ptr);
    names_[ptr] = std::move(name_string);
    return ptr;
}

template<typename T>
T *AbstractManager<T>::get(const char *name)
{
    auto it = objects_.find(name);
    if (it == objects_.end())
    {
        return nullptr;
    }
    return it->second.get();
}

template<typename T>
void AbstractManager<T>::remove(const char *name)
{
    auto it = objects_.find(name);
    if (it == objects_.end())
    {
        return;
    }
    auto name_it = names_.find(it->second.get());
    assert(name_it != names_.end());
    names_.erase(name_it);
    objects_.erase(it);
}

template<typename T>
void AbstractManager<T>::remove(T *obj)
{
    auto name_it = names_.find(obj);
    if (name_it == names_.end())
    {
        return;
    }
    auto it = objects_.find(name_it->second);
    assert(it != objects_.end());
    names_.erase(name_it);
    objects_.erase(it);
}