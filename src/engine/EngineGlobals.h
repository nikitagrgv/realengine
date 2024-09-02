#pragma once


class Time;
class Engine;
class FileSystem;
class Visualizer;
class MaterialManager;

struct EngineGlobals
{
    Engine *engine_{};
    FileSystem *fs{};
    Time *time{};
    Visualizer *visualizer{};
    MaterialManager *material_manager{};
};

extern EngineGlobals engine_globals;