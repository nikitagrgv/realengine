add_subdirectory(fs)
add_subdirectory(time)

target_sources(realengine
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/EngineGlobals.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/EngineGlobals.h
        ${CMAKE_CURRENT_SOURCE_DIR}/Base.h
        ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Shader.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Shader.h
)
