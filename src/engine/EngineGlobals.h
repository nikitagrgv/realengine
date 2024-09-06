#pragma once


class Renderer;
class Time;
class Engine;
class FileSystem;
class Visualizer;
class MaterialManager;
class ShaderManager;
class TextureManager;
class MeshManager;
class World;
class Input;
class SystemProxy;
struct GLFWwindow;

struct EngineGlobals
{
    Engine *engine_{};
    FileSystem *fs{};
    Time *time{};
    Visualizer *visualizer{};
    MaterialManager *material_manager{};
    ShaderManager *shader_manager{};
    TextureManager *texture_manager{};
    MeshManager *mesh_manager{};
    World *world{};
    Renderer *renderer{};
    GLFWwindow *window{};
    Input *input{};
    SystemProxy *proxy{};
};

extern EngineGlobals eng;