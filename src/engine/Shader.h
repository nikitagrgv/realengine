#pragma once

#include "Base.h"

#include "glm/fwd.hpp"
#include "glm/vec4.hpp"
#include <string>
#include <unordered_map>

class Shader
{
public:
    REMOVE_COPY_MOVE_CLASS(Shader);

    Shader();
    Shader(const char *vertex_src, const char *fragment_src);
    explicit Shader(const char *path);

    ~Shader();

    void loadSources(const char *vertex_src, const char *fragment_src);
    void loadFile(const char *path);

    void setUniformFloat(const char *name, float value);
    void setUniformVec3(const char *name, const glm::vec3 &value);
    void setUniformVec4(const char *name, const glm::vec4 &value);
    void setUniformMat4(const char *name, const glm::mat4 &value);

    void recompile();

    bool isLoaded() const;
    void clear();

    void bind();

private:
    static void read_shader(const char *path, std::string &vertex, std::string &fragment);
    static bool check_compiler_errors(unsigned int shader);
    static bool check_linking_errors(unsigned int program);

    int get_uniform_location(const char *name);

private:
    std::string filepath_;
    std::unordered_map<std::string, int> uniform_locations_;
    unsigned int program_id_{0};
};