#pragma once

#include <cassert>
#include <memory>
#include <string>
#include <unordered_map>

class T
{
    std::unique_ptr<int> g;
};

template<class T>
class AbstractManager
{
public:
    explicit AbstractManager(std::string empty_name_base);

    T *create(const char *name = nullptr);
    T *add(T obj, const char *name = nullptr); // TODO: remove

    T *get(int index);
    T *get(const char *name);

    int getIndex(T *obj);
    int getIndex(const char *name);

    const char *getName(int index);
    const char *getName(T *obj);

    void remove(int index);
    void remove(const char *name);
    void remove(T *obj);

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

template<typename T>
inline AbstractManager<T>::AbstractManager(std::string empty_name_base)
    : empty_name_base_(std::move(empty_name_base))
{}

template<typename T>
inline T *AbstractManager<T>::create(const char *name)
{
    T obj;
    return add(std::move(obj), name);
}

template<typename T>
inline T *AbstractManager<T>::add(T obj, const char *name)
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

    by_name_[name_string] = index;
    by_ptr[ptr] = index;
    return ptr;
}

template<typename T>
inline T *AbstractManager<T>::get(int index)
{
    return objects_[index].obj.get();
}

template<typename T>
inline T *AbstractManager<T>::get(const char *name)
{
    const int index = getIndex(name);
    return index == -1 ? nullptr : get(index);
}

template<typename T>
inline int AbstractManager<T>::getIndex(T *obj)
{
    const auto it = by_ptr.find(obj);
    if (it == by_ptr.end())
    {
        return -1;
    }
    return it->second;
}

template<typename T>
inline int AbstractManager<T>::getIndex(const char *name)
{
    const auto it = by_name_.find(name);
    if (it == by_name_.end())
    {
        return -1;
    }
    return it->second;
}

template<typename T>
inline const char *AbstractManager<T>::getName(int index)
{
    return objects_[index].name.c_str();
}

template<typename T>
inline const char *AbstractManager<T>::getName(T *obj)
{
    const int index = getIndex(obj);
    return index == -1 ? nullptr : getName(index);
}

template<typename T>
inline void AbstractManager<T>::remove(const char *name)
{
    const int index = getIndex(name);
    if (index == -1)
    {
        return;
    }
    remove(index);
}

template<typename T>
inline void AbstractManager<T>::remove(T *obj)
{
    const int index = getIndex(obj);
    if (index == -1)
    {
        return;
    }
    remove(index);
}

template<typename T>
inline void AbstractManager<T>::remove(int index)
{
#ifndef NDEBUG
    const int objects_size = objects_.size();
    const int by_ptr_size = by_ptr.size();
    const int by_name_size = by_name_.size();
    assert(objects_size == by_ptr_size && by_ptr_size == by_name_size);
#endif

    Object &obj = objects_[index];
    assert(by_ptr.find(obj.obj.get())->second == index);
    assert(by_name_.find(obj.name)->second == index);
    by_ptr.erase(obj.obj.get());
    by_name_.erase(obj.name);

    const int last_index = objects_.size() - 1;
    Object &last_obj = objects_[last_index];

    std::swap(obj, last_obj);

    // NOTE: obj now is last_obj

    by_ptr[obj.obj.get()] = index;
    by_name_[obj.name] = index;

    objects_.resize(last_index);

#ifndef NDEBUG
    assert(objects_size == objects_.size() + 1);
    assert(by_ptr_size == by_ptr.size() + 1);
    assert(by_name_size == by_name_.size() + 1);
#endif
}
