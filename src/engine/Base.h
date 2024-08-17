#pragma once

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