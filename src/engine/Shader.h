#pragma once

#include "Base.h"

#include "glm/fwd.hpp"
#include "glm/vec4.hpp"
#include <string>
#include <unordered_map>
#include <unordered_set>

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

    void setDefine(const char *name, bool value);
    void addDefine(const char *name);
    void removeDefine(const char *name);
    void clearDefines();

    void recompile();

    bool isLoaded() const;
    void clearProgram();
    void clearAll();

    void bind();

    bool isDirty() const;

private:
    static unsigned int compile_shader(const char *vertex_src, const char *fragment_src);
    static void read_shader(const char *path, const std::unordered_set<std::string> &defines,
        std::string &vertex, std::string &fragment);
    static void apply_defines(std::string &shader, const std::unordered_set<std::string> &orig_defines);
    static bool check_compiler_errors(unsigned int shader, const char *type);
    static bool check_linking_errors(unsigned int program);

    int get_uniform_location(const char *name);

private:
    std::string filepath_;
    std::string vertex_src_;
    std::string fragment_src_;

    std::unordered_map<std::string, int> uniform_locations_;
    unsigned int program_id_{0};
    std::unordered_set<std::string> defines_;
    std::unordered_set<std::string> compiled_defines_;
};