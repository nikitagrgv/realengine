#pragma once

#include "Base.h"

#include <string>

class Shader
{
public:
    REMOVE_COPY_MOVE_CLASS(Shader);

    Shader(const char *path);
    ~Shader();

    void clear();

    void bind();

private:
    void read_shader(const char *path, std::string &vertex, std::string &fragment);

    bool check_compiler_errors(unsigned int shader);

    bool check_linking_errors(unsigned int program);

private:
    bool valid_{false};
    unsigned int program_id_{0};
};