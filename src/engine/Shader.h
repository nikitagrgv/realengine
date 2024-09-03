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
    REMOVE_COPY_CLASS(Shader);

    Shader();
    Shader(const char *vertex_src, const char *fragment_src);
    explicit Shader(const char *path);

    Shader(Shader &&other) noexcept;
    Shader &operator=(Shader &&other) noexcept;

    ~Shader();

    void loadSources(const char *vertex_src, const char *fragment_src);
    void loadFile(const char *path);

    void setUniformFloat(int location, float value);
    void setUniformVec2(int location, const glm::vec2 &value);
    void setUniformVec3(int location, const glm::vec3 &value);
    void setUniformVec4(int location, const glm::vec4 &value);
    void setUniformMat4(int location, const glm::mat4 &value);
    void setUniformInt(int location, int value);

    void setUniformFloat(const char *name, float value);
    void setUniformVec2(const char *name, const glm::vec2 &value);
    void setUniformVec3(const char *name, const glm::vec3 &value);
    void setUniformVec4(const char *name, const glm::vec4 &value);
    void setUniformMat4(const char *name, const glm::mat4 &value);
    void setUniformInt(const char *name, int value);

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

    int getUniformLocation(const char *name);
    int hasUniform(const char *name);

private:
    static unsigned int compile_shader(const char *vertex_src, const char *fragment_src);
    static void read_from_file(const char *path, std::string &vertex, std::string &fragment);
    static void prepare_shader(std::string &shader,
        const std::unordered_set<std::string> &defines);
    static bool check_compiler_errors(unsigned int shader, const char *type);
    static bool check_linking_errors(unsigned int program);

    int get_uniform_location_with_warning(const char *name);

private:
    std::string filepath_;
    std::string vertex_src_;
    std::string fragment_src_;

    std::unordered_map<std::string, int> uniform_locations_;
    unsigned int program_id_{0};
    std::unordered_set<std::string> defines_;
    std::unordered_set<std::string> compiled_defines_;
};