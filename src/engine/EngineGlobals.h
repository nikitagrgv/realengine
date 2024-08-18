#pragma once


class Time;
class Engine;
class FileSystem;

struct EngineGlobals
{
    Engine *engine_{};
    FileSystem *fs{};
    Time *time{};
};

extern EngineGlobals engine_globals;