#include "Shader.h"

#include "EngineGlobals.h"
#include "fs/FileSystem.h"
#include "glad/glad.h"

#include "glm/mat4x4.hpp"
#include <cassert>
#include <iostream>

Shader::Shader() = default;

Shader::Shader(const char *vertex_src, const char *fragment_src)
{
    loadSources(vertex_src, fragment_src);
}

Shader::Shader(const char *path)
{
    loadFile(path);
}

Shader::~Shader()
{
    clear();
}

void Shader::loadSources(const char *vertex_src, const char *fragment_src)
{
    clear();

    unsigned int vertex_id;
    vertex_id = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_id, 1, &vertex_src, NULL);
    glCompileShader(vertex_id);
    bool success = check_compiler_errors(vertex_id);
    if (!success)
    {
        glDeleteShader(vertex_id);
        return;
    }

    unsigned int fragment_id;
    fragment_id = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_id, 1, &fragment_src, NULL);
    glCompileShader(fragment_id);
    success = check_compiler_errors(fragment_id);
    if (!success)
    {
        glDeleteShader(vertex_id);
        glDeleteShader(fragment_id);
        return;
    }

    program_id_ = glCreateProgram();
    glAttachShader(program_id_, vertex_id);
    glAttachShader(program_id_, fragment_id);
    glLinkProgram(program_id_);
    success = check_linking_errors(program_id_);
    if (!success)
    {
        program_id_ = 0;
        glDeleteShader(vertex_id);
        glDeleteShader(fragment_id);
        glDeleteProgram(program_id_);
        return;
    }

    glUseProgram(program_id_);
    glDeleteShader(vertex_id);
    glDeleteShader(fragment_id);
}

void Shader::loadFile(const char *path)
{
    std::string vertex_source;
    std::string fragment_source;
    read_shader(path, vertex_source, fragment_source);
    loadSources(vertex_source.c_str(), fragment_source.c_str());
    filepath_ = path;
}

void Shader::setUniformFloat(const char *name, float value)
{
    const int location = get_uniform_location(name);
    if (location != -1)
    {
        glUniform1f(location, value);
    }
}

void Shader::setUniformVec3(const char *name, const glm::vec3 &value)
{
    const int location = get_uniform_location(name);
    if (location != -1)
    {
        glUniform3f(location, value.x, value.y, value.z);
    }
}

void Shader::setUniformVec4(const char *name, const glm::vec4 &value)
{
    const int location = get_uniform_location(name);
    if (location != -1)
    {
        glUniform4f(location, value.x, value.y, value.z, value.w);
    }
}

void Shader::setUniformMat4(const char *name, const glm::mat4 &value)
{
    const int location = get_uniform_location(name);
    if (location != -1)
    {
        glUniformMatrix4fv(location, 1, GL_FALSE, &value[0][0]);
    }
}

void Shader::recompile()
{
    if (filepath_.empty())
    {
        return;
    }
    // TODO: shit
    std::string filepath = std::move(filepath_);
    loadFile(filepath.c_str());
}

bool Shader::isLoaded() const
{
    return program_id_ != 0;
}

void Shader::clear()
{
    filepath_.clear();
    uniform_locations_.clear();
    if (program_id_ != 0)
    {
        glDeleteProgram(program_id_);
        program_id_ = 0;
    }
}

void Shader::bind()
{
    glUseProgram(program_id_);
}

void Shader::read_shader(const char *path, std::string &vertex, std::string &fragment)
{
    vertex.clear();
    fragment.clear();

    std::string shaders = engine_globals.fs->readFile(path);
    const int vertex_idx = shaders.find("#vertex");
    const int fragment_idx = shaders.find("#fragment");
    const int vertex_idx_end = vertex_idx + strlen("#vertex");
    const int fragment_idx_end = fragment_idx + strlen("#fragment");

    assert(vertex_idx < fragment_idx);

    std::string common = shaders.substr(0, vertex_idx);

    const auto replace_with = [](std::string &string, const char *from, const char *to) {
        std::string::size_type pos = 0;
        while ((pos = string.find(from, pos)) != std::string::npos)
        {
            string.replace(pos, strlen(from), to);
            pos += strlen(to);
        }
    };

    vertex = common;
    replace_with(vertex, "#inout", "out");
    vertex += shaders.substr(vertex_idx_end, fragment_idx - vertex_idx_end);

    fragment = std::move(common);
    replace_with(fragment, "#inout", "in");
    fragment += shaders.substr(fragment_idx_end);
}

bool Shader::check_compiler_errors(unsigned int shader)
{
    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    return success;
}

bool Shader::check_linking_errors(unsigned int program)
{
    int success;
    char infoLog[512];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::LINKING_FAILED\n" << infoLog << std::endl;
    }
    return success;
}

int Shader::get_uniform_location(const char *name)
{
    if (!isLoaded())
    {
        return -1;
    }
    auto it = uniform_locations_.find(name);
    if (it != uniform_locations_.end())
    {
        return it->second;
    }
    const int location = glGetUniformLocation(program_id_, name);
    if (location == -1)
    {
        std::cout << "Could not find uniform: " << name << std::endl;
    }
    else
    {
        uniform_locations_[name] = location;
    }
    return location;
}