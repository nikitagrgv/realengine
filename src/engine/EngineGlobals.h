#pragma once


class Time;
class Engine;
class FileSystem;
class Visualizer;

struct EngineGlobals
{
    Engine *engine_{};
    FileSystem *fs{};
    Time *time{};
    Visualizer *visualizer{};
};

extern EngineGlobals engine_globals;