#include "Shader.h"

#include "EngineGlobals.h"
#include "fs/FileSystem.h"
#include "glad/glad.h"

#include <cassert>
#include <iostream>

Shader::Shader(const char *path)
{
    std::string vertex_source;
    std::string fragment_source;
    read_shader(path, vertex_source, fragment_source);
    const char *vertex_ptr = vertex_source.c_str();
    const char *fragment_ptr = fragment_source.c_str();

    unsigned int vertex_id;
    vertex_id = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_id, 1, &vertex_ptr, NULL);
    glCompileShader(vertex_id);
    valid_ = check_compiler_errors(vertex_id);
    if (!valid_)
    {
        glDeleteShader(vertex_id);
        return;
    }

    unsigned int fragment_id;
    fragment_id = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_id, 1, &fragment_ptr, NULL);
    glCompileShader(fragment_id);
    valid_ = check_compiler_errors(fragment_id);
    if (!valid_)
    {
        glDeleteShader(vertex_id);
        glDeleteShader(fragment_id);
        return;
    }

    program_id_ = glCreateProgram();
    glAttachShader(program_id_, vertex_id);
    glAttachShader(program_id_, fragment_id);
    glLinkProgram(program_id_);
    valid_ = check_linking_errors(program_id_);
    if (!valid_)
    {
        glDeleteShader(vertex_id);
        glDeleteShader(fragment_id);
        glDeleteProgram(program_id_);
        return;
    }

    glUseProgram(program_id_);
    glDeleteShader(vertex_id);
    glDeleteShader(fragment_id);
}

Shader::~Shader()
{
    clear();
}

void Shader::clear()
{
    if (valid_)
    {
        glDeleteProgram(program_id_);
        program_id_ = 0;
        valid_ = false;
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