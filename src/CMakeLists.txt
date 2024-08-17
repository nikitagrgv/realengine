cmake_minimum_required(VERSION 3.28)
project(realengine)

set(CMAKE_CXX_STANDARD 17)

add_executable(realengine main.cpp)

# GLFW
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
add_subdirectory(third_party/glfw-3.4)
target_link_libraries(realengine glfw)

#GLAD
add_subdirectory(third_party/glad)
target_link_libraries(realengine glad)

# OPENGL
target_link_libraries(realengine opengl32)