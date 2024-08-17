#include "FileSystem.h"

#include <windows.h>

#include <cassert>
#include <filesystem>
#include <fstream>

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

std::string FileSystem::readFile(const char *path)
{
    std::string abs_path = toAbsolutePath(path);
    std::ifstream file(abs_path, std::ios::binary);
    std::string str;
    file.seekg(0, std::ios::end);
    str.reserve(file.tellg());
    file.seekg(0, std::ios::beg);
    str.assign((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    return str;
}

FileSystem::FileSystem()
{
    data_path_ = get_data_path();
}

FileSystem::~FileSystem() = default;
