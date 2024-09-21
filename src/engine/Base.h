#pragma once

#include <cassert>
#include <memory>

#if defined(_WIN32) && defined(REALENGINE_SHARED)
    #define REALENGINE_IMPORT __declspec(dllimport)
    #define REALENGINE_EXPORT __declspec(dllexport)
#else
    #define REALENGINE_IMPORT
    #define REALENGINE_EXPORT
#endif

#ifdef REALENGINE_LIBRARY
    #define REALENGINE_API REALENGINE_EXPORT
#else
    #define REALENGINE_API REALENGINE_IMPORT
#endif

#define REMOVE_COPY_CLASS(className)                                                               \
    className(const className &) = delete;                                                         \
    className &operator=(const className &) = delete

#define REMOVE_MOVE_CLASS(className)                                                               \
    className(className &&) = delete;                                                              \
    className &operator=(className &&) = delete

#define REMOVE_COPY_MOVE_CLASS(className)                                                          \
    REMOVE_COPY_CLASS(className);                                                                  \
    REMOVE_MOVE_CLASS(className)

#define _REALENGINE_CONCATENATE_HELPER(x, y) x##y
#define REALENGINE_CONCATENATE(x, y)         _REALENGINE_CONCATENATE_HELPER(x, y)

template<typename F>
class ScopedExit
{
public:
    explicit ScopedExit(F f)
        : f_(std::move(f))
    {}
    ~ScopedExit() { f_(); }

private:
    F f_;
};

#define REALENGINE_SCOPE_EXIT(f) ScopedExit REALENGINE_CONCATENATE(scoped_exit_, __LINE__)(f)

template<typename T>
using UPtr = std::unique_ptr<T>;

template<typename T, class... Args>
UPtr<T> makeU(Args &&...args)
{
    return std::make_unique<T>(std::forward<Args>(args)...);
}

void clearGLErrors();
bool checkGLErrors(const char *function, const char *file, int line);

#ifndef NDEBUG
    #define GL_CHECKED(x)                                                                          \
        assert(checkGLErrors(#x, __FILE__, __LINE__) && "Previous errors");                        \
        x;                                                                                         \
        assert(checkGLErrors(#x, __FILE__, __LINE__))
    #define GL_CHECKED_RET(x)                                                                      \
        [&]() {                                                                                    \
            clearGLErrors();                                                                       \
            auto ret = x;                                                                          \
            assert(checkGLErrors(#x, __FILE__, __LINE__));                                         \
            return ret;                                                                            \
        }()
#else
    #define GL_CHECKED(x)     x
    #define GL_CHECKED_RET(x) x
#endif