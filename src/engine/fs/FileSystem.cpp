#include "FileSystem.h"

#include <windows.h>

#include <cassert>
#include <filesystem>

std::filesystem::path get_exe_path()
{
#ifdef _WIN32
    wchar_t path[MAX_PATH] = {0};
    GetModuleFileNameW(NULL, path, MAX_PATH);
    return path;
#else
    char result[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
    return std::string(result, (count > 0) ? count : 0);
#endif
}

std::string get_data_path()
{
    const std::filesystem::path &exe_path = get_exe_path();
    std::filesystem::path data_path = exe_path.parent_path().parent_path() / "data";
    assert(exists(data_path));
    return data_path.string().data(); // TODO: not string? pass `path` to FileSystem?
}

std::string FileSystem::getDataPath() const
{
    return data_path_;
}

std::string FileSystem::toAbsolutePath(const char *path) const
{
    return data_path_ + '/' + path;
}

FileSystem::FileSystem()
{
    data_path_ = get_data_path();
}

FileSystem::~FileSystem() = default;
