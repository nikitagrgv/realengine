#pragma once

#include "Base.h"

#include "glm/fwd.hpp"
#include <string>
#include <unordered_map>


class ShaderSource;
class Shader
{
public:
    REMOVE_COPY_MOVE_CLASS(Shader);

    Shader();
    explicit Shader(ShaderSource *source);

    ~Shader();

    void setSource(ShaderSource *source);
    void unbindSource();
    ShaderSource *getSource() const { return source_; }

    void setUniformFloat(int location, float value);
    void setUniformVec2(int location, const glm::vec2 &value);
    void setUniformVec3(int location, const glm::vec3 &value);
    void setUniformVec4(int location, const glm::vec4 &value);
    void setUniformMat3(int location, const glm::mat3 &value);
    void setUniformMat4(int location, const glm::mat4 &value);
    void setUniformInt(int location, int value);

    void setUniformFloat(const char *name, float value);
    void setUniformVec2(const char *name, const glm::vec2 &value);
    void setUniformVec3(const char *name, const glm::vec3 &value);
    void setUniformVec4(const char *name, const glm::vec4 &value);
    void setUniformMat3(const char *name, const glm::mat3 &value);
    void setUniformMat4(const char *name, const glm::mat4 &value);
    void setUniformInt(const char *name, int value);

    void setDefines(std::vector<std::string> defines);

    void recompile();

    bool isLoaded() const;
    void clearProgram();
    void clearAll();

    void bind() const;

    bool isDirty() const;

    int getUniformLocation(const char *name);
    int hasUniform(const char *name);

private:
    static unsigned int compile_shader(const char *vertex_src, const char *fragment_src);
    static bool check_compiler_errors(unsigned int shader, const char *type);
    static bool check_linking_errors(unsigned int program);

    int get_uniform_location_with_warning(const char *name);

    void check_used_program();

    static unsigned int get_current_program();

private:
    ShaderSource *source_{};

    std::unordered_map<std::string, int> uniform_locations_;
    unsigned int program_id_{0};
    std::vector<std::string> defines_;
    bool dirty_{true};
};