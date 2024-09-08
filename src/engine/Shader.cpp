#include "Shader.h"

#include "EngineGlobals.h"
#include "ShaderSource.h"
#include "fs/FileSystem.h"
#include "glad/glad.h"

#include "glm/gtc/type_ptr.inl"
#include "glm/mat4x4.hpp"
#include <cassert>
#include <iostream>

Shader::Shader() = default;

Shader::Shader(ShaderSource *source)
{
    setSource(source);
}

Shader::Shader(Shader &&other) noexcept
{
    *this = std::move(other);
}

Shader &Shader::operator=(Shader &&other) noexcept
{
    if (this != &other)
    {
        clearAll();

        setSource(other.source_);
        uniform_locations_ = std::move(other.uniform_locations_);
        program_id_ = other.program_id_;
        defines_ = std::move(other.defines_);
        dirty_ = other.dirty_;

        other.clearAll();
    }
    return *this;
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
    glUniform1f(location, value);
}

void Shader::setUniformVec2(int location, const glm::vec2 &value)
{
    assert(location != -1);
    check_used_program();
    glUniform2f(location, value.x, value.y);
}

void Shader::setUniformVec3(int location, const glm::vec3 &value)
{
    assert(location != -1);
    check_used_program();
    glUniform3f(location, value.x, value.y, value.z);
}

void Shader::setUniformVec4(int location, const glm::vec4 &value)
{
    assert(location != -1);
    check_used_program();
    glUniform4f(location, value.x, value.y, value.z, value.w);
}

void Shader::setUniformMat4(int location, const glm::mat4 &value)
{
    assert(location != -1);
    check_used_program();
    glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
}

void Shader::setUniformInt(int location, int value)
{
    assert(location != -1);
    check_used_program();
    glUniform1i(location, value);
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

void Shader::setDefine(const char *name, bool value)
{
    if (value)
    {
        addDefine(name);
    }
    else
    {
        removeDefine(name);
    }
}

void Shader::addDefine(const char *name)
{
    if (hasDefine(name))
    {
        return;
    }
    dirty_ = true;
    defines_.emplace_back(name);
}

void Shader::removeDefine(const char *name)
{
    // TODO: removeFast
    auto it = std::find_if(defines_.begin(), defines_.end(),
        [&](const std::string &d) { return d == name; });
    if (it == defines_.end())
    {
        return;
    }
    dirty_ = true;
    defines_.erase(it);
}

void Shader::clearDefines()
{
    defines_.clear();
    dirty_ = true;
}

bool Shader::hasDefine(const char *name)
{
    auto it = std::find_if(defines_.begin(), defines_.end(),
        [&](const std::string &d) { return d == name; });
    return it != defines_.end();
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
        glDeleteProgram(program_id_);
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
    glUseProgram(program_id_);
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
        assert(it->second == glGetUniformLocation(program_id_, name));
        return it->second;
    }
    const int location = glGetUniformLocation(program_id_, name);
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
    vertex_id = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_id, 1, &vertex_src, NULL);
    glCompileShader(vertex_id);
    bool success = check_compiler_errors(vertex_id, "VERTEX");
    if (!success)
    {
        glDeleteShader(vertex_id);
        return 0;
    }

    unsigned int fragment_id;
    fragment_id = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_id, 1, &fragment_src, NULL);
    glCompileShader(fragment_id);
    success = check_compiler_errors(fragment_id, "FRAGMENT");
    if (!success)
    {
        glDeleteShader(vertex_id);
        glDeleteShader(fragment_id);
        return 0;
    }

    unsigned int program_id = glCreateProgram();
    glAttachShader(program_id, vertex_id);
    glAttachShader(program_id, fragment_id);
    glLinkProgram(program_id);
    success = check_linking_errors(program_id);
    if (!success)
    {
        glDeleteShader(vertex_id);
        glDeleteShader(fragment_id);
        glDeleteProgram(program_id);
        return 0;
    }

    glUseProgram(program_id);
    glDeleteShader(vertex_id);
    glDeleteShader(fragment_id);
    ++eng.stat.numCompiledShadersInFrame;
    return program_id;
}

bool Shader::check_compiler_errors(unsigned int shader, const char *type)
{
    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::COMPILATION_FAILED (" << type << "):\n"
                  << infoLog << std::endl;
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
    glGetIntegerv(GL_CURRENT_PROGRAM, &id);
    return (unsigned int)id;
}