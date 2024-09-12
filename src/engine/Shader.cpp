#include "Shader.h"

#include "EngineGlobals.h"
#include "ShaderSource.h"
#include "fs/FileSystem.h"
#include "glad/glad.h"

#include "glm/gtc/type_ptr.inl"
#include "glm/mat4x4.hpp"
#include <cassert>
#include <iostream>
#include <unordered_set>

Shader::Shader() = default;

Shader::Shader(ShaderSource *source)
{
    setSource(source);
}

Shader::~Shader()
{
    clearAll();
}

void Shader::setSource(ShaderSource *source)
{
    if (source_ == source)
    {
        return;
    }
    unbindSource();
    source_ = source;
    if (source_)
    {
        source_->add_shader(this);
    }
    dirty_ = true;
}

void Shader::unbindSource()
{
    if (!source_)
    {
        return;
    }
    source_->remove_shader(this);
    source_ = nullptr;
    dirty_ = true;
}

void Shader::setUniformFloat(int location, float value)
{
    assert(location != -1);
    check_used_program();
    GL_CHECKED(glUniform1f(location, value));
}

void Shader::setUniformVec2(int location, const glm::vec2 &value)
{
    assert(location != -1);
    check_used_program();
    GL_CHECKED(glUniform2f(location, value.x, value.y));
}

void Shader::setUniformVec3(int location, const glm::vec3 &value)
{
    assert(location != -1);
    check_used_program();
    GL_CHECKED(glUniform3f(location, value.x, value.y, value.z));
}

void Shader::setUniformVec4(int location, const glm::vec4 &value)
{
    assert(location != -1);
    check_used_program();
    GL_CHECKED(glUniform4f(location, value.x, value.y, value.z, value.w));
}

void Shader::setUniformMat4(int location, const glm::mat4 &value)
{
    assert(location != -1);
    check_used_program();
    GL_CHECKED(glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value)));
}

void Shader::setUniformInt(int location, int value)
{
    assert(location != -1);
    check_used_program();
    GL_CHECKED(glUniform1i(location, value));
}

void Shader::setUniformFloat(const char *name, float value)
{
    const int location = getUniformLocation(name);
    if (location != -1)
    {
        setUniformFloat(location, value);
    }
}

void Shader::setUniformVec2(const char *name, const glm::vec2 &value)
{
    const int location = getUniformLocation(name);
    if (location != -1)
    {
        setUniformVec2(location, value);
    }
}

void Shader::setUniformVec3(const char *name, const glm::vec3 &value)
{
    const int location = getUniformLocation(name);
    if (location != -1)
    {
        setUniformVec3(location, value);
    }
}

void Shader::setUniformVec4(const char *name, const glm::vec4 &value)
{
    const int location = getUniformLocation(name);
    if (location != -1)
    {
        setUniformVec4(location, value);
    }
}

void Shader::setUniformMat4(const char *name, const glm::mat4 &value)
{
    const int location = getUniformLocation(name);
    if (location != -1)
    {
        setUniformMat4(location, value);
    }
}

void Shader::setUniformInt(const char *name, int value)
{
    const int location = getUniformLocation(name);
    if (location != -1)
    {
        setUniformInt(location, value);
    }
}

void Shader::setDefines(std::vector<std::string> defines)
{
#ifndef NDEBUG
    std::unordered_set<std::string> set;
    for (const std::string &d : defines)
    {
        auto it = set.find(d);
        assert(it == set.end());
        set.insert(d);
    }
#endif

    defines_ = std::move(defines);
    dirty_ = true;
}

void Shader::recompile()
{
    clearProgram();
    assert(program_id_ == 0);

    if (!source_)
    {
        dirty_ = false;
        return;
    }

    std::string vertex_source = source_->makeSourceVertex(defines_);
    std::string fragment_source = source_->makeSourceFragment(defines_);

    program_id_ = compile_shader(vertex_source.c_str(), fragment_source.c_str());
    dirty_ = false;
}

bool Shader::isLoaded() const
{
    return program_id_ != 0;
}

void Shader::clearProgram()
{
    if (program_id_ != 0)
    {
        GL_CHECKED(glDeleteProgram(program_id_));
        program_id_ = 0;
    }
    uniform_locations_.clear();
}

void Shader::clearAll()
{
    unbindSource();
    clearProgram();
    defines_.clear();
    dirty_ = true;
}

void Shader::bind() const
{
    assert(!dirty_);
    if (program_id_ == 0)
    {
        std::cout << "Shader is not loaded\n" << std::endl;
    }
    GL_CHECKED(glUseProgram(program_id_));
}

bool Shader::isDirty() const
{
    return !isLoaded() || dirty_;
}

int Shader::getUniformLocation(const char *name)
{
    if (!isLoaded())
    {
        return -1;
    }
    auto it = uniform_locations_.find(name);
    if (it != uniform_locations_.end())
    {
        assert(it->second == GL_CHECKED_RET(glGetUniformLocation(program_id_, name)));
        return it->second;
    }
    const int location = GL_CHECKED_RET(glGetUniformLocation(program_id_, name));
    uniform_locations_[name] = location;
    return location;
}

int Shader::hasUniform(const char *name)
{
    return getUniformLocation(name) != -1;
}

unsigned int Shader::compile_shader(const char *vertex_src, const char *fragment_src)
{
    unsigned int vertex_id;
    vertex_id = GL_CHECKED_RET(glCreateShader(GL_VERTEX_SHADER));
    GL_CHECKED(glShaderSource(vertex_id, 1, &vertex_src, NULL));
    GL_CHECKED(glCompileShader(vertex_id));
    bool success = check_compiler_errors(vertex_id, "VERTEX");
    if (!success)
    {
        GL_CHECKED(glDeleteShader(vertex_id));
        return 0;
    }

    unsigned int fragment_id;
    fragment_id = GL_CHECKED_RET(glCreateShader(GL_FRAGMENT_SHADER));
    GL_CHECKED(glShaderSource(fragment_id, 1, &fragment_src, NULL));
    GL_CHECKED(glCompileShader(fragment_id));
    success = check_compiler_errors(fragment_id, "FRAGMENT");
    if (!success)
    {
        GL_CHECKED(glDeleteShader(vertex_id));
        GL_CHECKED(glDeleteShader(fragment_id));
        return 0;
    }

    unsigned int program_id = GL_CHECKED_RET(glCreateProgram());
    GL_CHECKED(glAttachShader(program_id, vertex_id));
    GL_CHECKED(glAttachShader(program_id, fragment_id));
    GL_CHECKED(glLinkProgram(program_id));
    success = check_linking_errors(program_id);
    if (!success)
    {
        GL_CHECKED(glDeleteShader(vertex_id));
        GL_CHECKED(glDeleteShader(fragment_id));
        GL_CHECKED(glDeleteProgram(program_id));
        return 0;
    }

    GL_CHECKED(glUseProgram(program_id));
    GL_CHECKED(glDeleteShader(vertex_id));
    GL_CHECKED(glDeleteShader(fragment_id));
    eng.stat.addCompiledShaders(1);
    return program_id;
}

bool Shader::check_compiler_errors(unsigned int shader, const char *type)
{
    int success;
    char infoLog[512];
    GL_CHECKED(glGetShaderiv(shader, GL_COMPILE_STATUS, &success));
    if (!success)
    {
        GL_CHECKED(glGetShaderInfoLog(shader, 512, NULL, infoLog));
        std::cout << "ERROR::SHADER::COMPILATION_FAILED (" << type << "):\n"
                  << infoLog << std::endl;
    }
    return success;
}

bool Shader::check_linking_errors(unsigned int program)
{
    int success;
    char infoLog[512];
    GL_CHECKED(glGetProgramiv(program, GL_LINK_STATUS, &success));
    if (!success)
    {
        GL_CHECKED(glGetProgramInfoLog(program, 512, NULL, infoLog));
        std::cout << "ERROR::SHADER::LINKING_FAILED:\n" << infoLog << std::endl;
    }
    return success;
}

int Shader::get_uniform_location_with_warning(const char *name)
{
    const int location = getUniformLocation(name);
    if (location == -1)
    {
        std::cout << "Could not find uniform: " << name << std::endl;
    }
    return location;
}

void Shader::check_used_program()
{
    assert(get_current_program() == program_id_);
}

unsigned int Shader::get_current_program()
{
    int id;
    GL_CHECKED(glGetIntegerv(GL_CURRENT_PROGRAM, &id));
    return (unsigned int)id;
}