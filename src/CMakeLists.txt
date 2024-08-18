cmake_minimum_required(VERSION 3.28)
project(realengine)

set(CMAKE_CXX_STANDARD 17)

add_executable(realengine)

target_include_directories(realengine PRIVATE engine)

add_subdirectory(engine)

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

# GLM
add_subdirectory(third_party/glm-1.0.1)
target_link_libraries(realengine glm)

# STB
target_include_directories(realengine PRIVATE third_party/stb)

# OBJ LOADER
target_include_directories(realengine PRIVATE third_party/obj_loader)

set(REALENGINE_BIN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../bin)
set_target_properties(realengine
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_RELEASE ${REALENGINE_BIN_DIR}
        RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${REALENGINE_BIN_DIR}
        RUNTIME_OUTPUT_DIRECTORY_DEBUG ${REALENGINE_BIN_DIR}
        RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${REALENGINE_BIN_DIR}
        LIBRARY_OUTPUT_DIRECTORY_RELEASE ${REALENGINE_BIN_DIR}
        LIBRARY_OUTPUT_DIRECTORY_MINSIZEREL ${REALENGINE_BIN_DIR}
        LIBRARY_OUTPUT_DIRECTORY_DEBUG ${REALENGINE_BIN_DIR}
        LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO ${REALENGINE_BIN_DIR}
)
