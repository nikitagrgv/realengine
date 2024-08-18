#pragma once

#include "Base.h"

#include "glm/vec4.hpp"
#include <string>
#include <unordered_map>

class Shader
{
public:
    REMOVE_COPY_MOVE_CLASS(Shader);

    Shader(const char *path);
    ~Shader();

    void setUniformFloat(const char *name, float value);
    void setUniformVec4(const char *name, const glm::vec4 &value);

    void clear();
    void bind();

private:
    void read_shader(const char *path, std::string &vertex, std::string &fragment);
    bool check_compiler_errors(unsigned int shader);
    bool check_linking_errors(unsigned int program);

    int get_uniform_location(const char *name);

private:
    std::unordered_map<std::string, int> uniform_locations_;
    bool valid_{false};
    unsigned int program_id_{0};
};