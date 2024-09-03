#pragma once


class Time;
class Engine;
class FileSystem;
class Visualizer;
class MaterialManager;
class ShaderManager;

struct EngineGlobals
{
    Engine *engine_{};
    FileSystem *fs{};
    Time *time{};
    Visualizer *visualizer{};
    MaterialManager *material_manager{};
    ShaderManager *shader_manager{};
};

extern EngineGlobals engine_globals;