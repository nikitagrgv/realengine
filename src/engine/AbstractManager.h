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

    int getCount() const;

    T *get(int index) const;
    T *get(const char *name) const;

    int getIndex(T *obj) const;
    int getIndex(const char *name) const;

    const char *getName(int index) const;
    const char *getName(T *obj) const;

    bool contains(int index) const;
    bool contains(const char *name) const;
    bool contains(T *obj) const;

    void remove(int index);
    void remove(const char *name);
    void remove(T *obj);

protected:
    std::string generate_or_check_name(const char *name);

    T *add(T obj, std::string name);

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
    std::string name_string = generate_or_check_name(name);
    return add(std::move(obj), std::move(name_string));
}

template<class T>
inline int AbstractManager<T>::getCount() const
{
    assert(objects_.size() == by_name_.size() && objects_.size() == by_ptr.size());
    return objects_.size();
}

template<typename T>
inline T *AbstractManager<T>::get(int index) const
{
    return objects_[index].obj.get();
}

template<typename T>
inline T *AbstractManager<T>::get(const char *name) const
{
    const int index = getIndex(name);
    return index == -1 ? nullptr : get(index);
}

template<typename T>
inline int AbstractManager<T>::getIndex(T *obj) const
{
    const auto it = by_ptr.find(obj);
    if (it == by_ptr.end())
    {
        return -1;
    }
    return it->second;
}

template<typename T>
inline int AbstractManager<T>::getIndex(const char *name) const
{
    const auto it = by_name_.find(name);
    if (it == by_name_.end())
    {
        return -1;
    }
    return it->second;
}

template<typename T>
inline const char *AbstractManager<T>::getName(int index) const
{
    return objects_[index].name.c_str();
}

template<typename T>
inline const char *AbstractManager<T>::getName(T *obj) const
{
    const int index = getIndex(obj);
    return index == -1 ? nullptr : getName(index);
}

template<class T>
bool AbstractManager<T>::contains(int index) const
{
    return index >= 0 && index < objects_.size();
}

template<class T>
bool AbstractManager<T>::contains(const char *name) const
{
    return by_name_.find(name) != by_name_.end();
}

template<class T>
bool AbstractManager<T>::contains(T *obj) const
{
    return by_ptr.find(obj) != by_ptr.end();
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

template<class T>
std::string AbstractManager<T>::generate_or_check_name(const char *name)
{
    // TODO: shitty
    std::string name_string = name ? name : "";

    auto it = by_name_.find(name_string);
    assert(it == by_name_.end());

    if (name_string.empty() || it != by_name_.end())
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

    return name_string;
}

template<class T>
T *AbstractManager<T>::add(T obj, std::string name)
{
    assert(by_name_.find(name) == by_name_.end());

    auto unique_ptr = std::make_unique<T>(std::move(obj));
    auto ptr = unique_ptr.get();

    const int index = int(objects_.size());

    Object o;
    o.name = name;
    o.obj = std::move(unique_ptr);
    objects_.push_back(std::move(o));

    by_name_[std::move(name)] = index;
    by_ptr[ptr] = index;
    return ptr;
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
