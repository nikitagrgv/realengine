#pragma once

#include "Base.h"

#include <string>

class REALENGINE_API FileSystem
{
public:
    REMOVE_COPY_MOVE_CLASS(FileSystem);

    FileSystem();
    ~FileSystem();

    [[nodiscard]] std::string getDataPath() const;
    [[nodiscard]] std::string toAbsolutePath(const char *path) const;

    std::string readFile(const char *path);

private:
    std::string data_path_;
};
